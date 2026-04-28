#include <battery.h>

const float batt_inv_divider_val = (float) (BATTERY_DIVIDER_R1 + BATTERY_DIVIDER_R2) / BATTERY_DIVIDER_R1;

std::vector < uint16_t > calibr_time;
std::vector < float > calibr_v;
bool calibr_failed = false;
float CRITICAL_V;

float clamp(float val, float min_ = 0, float max_ = 1){
    return max(min(val, max_), min_);
}

float default_v_adj(float v){
    return v;
}



Gamepad_battery::Gamepad_battery(){
    v_adj_func = default_v_adj;
}

void Gamepad_battery::init(float critical_v_, float full_v_, float charging_v_, float only_charging_v_){
    critical_v = critical_v_;
    full_v = full_v_;
    charging_v = charging_v_;
    power_off_v = only_charging_v_;
    CRITICAL_V = critical_v;

    batt_pin_mutex = xSemaphoreCreateMutex();

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(BATTERY_ADC_CHANNEL, ADC_ATTEN_DB_12);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 0, &adc1_chars);
}

void Gamepad_battery::set_voltage_adjustment(float (*v_adj_func_ptr)(float)){
    v_adj_func = v_adj_func_ptr;
}


float Gamepad_battery::get_battery_voltage(){
    while(!xSemaphoreTake(batt_pin_mutex, portMAX_DELAY));      // wait until analog read is available

    int raw_read = 0;
    for(uint8_t i = 0; i < BATTERY_N_OF_MEASURES; i++)
        raw_read += adc1_get_raw(BATTERY_ADC_CHANNEL);
    raw_read = (float) raw_read / BATTERY_N_OF_MEASURES;

    float v_raw = esp_adc_cal_raw_to_voltage(raw_read, &adc1_chars);
    v_raw = v_raw * batt_inv_divider_val / 1000.0;

    xSemaphoreGive(batt_pin_mutex);

    return v_adj_func(v_raw); 
}

uint8_t Gamepad_battery::get_battery_charge(){
    float v = get_battery_voltage();

    if(voltage_levels == nullptr){
        return round(clamp((v - critical_v) / (full_v - critical_v)) * BATTERY_LEVELS);
    }

    uint8_t i = 0;
    while(i < BATTERY_LEVELS && voltage_levels[i] > v)
        i++;

    return BATTERY_LEVELS - i;
}

Gamepad_battery::Charge_mode_t Gamepad_battery::get_device_mode(){
    float v = get_battery_voltage();
    if(v > power_off_v)
        return POWER_OFF;
    if(v > charging_v)
        return CHARGING;
    return POWER_ON;
}

void battery_callibration(void *params){
    float v = GAMEPAD_GLOBAL::battery.get_battery_voltage();
    while(v > CRITICAL_V){
        if(GAMEPAD_GLOBAL::battery.get_device_mode() != Gamepad_battery::POWER_ON){
            calibr_failed = true;
            break;
        }

        v = GAMEPAD_GLOBAL::battery.get_battery_voltage();

        calibr_time.push_back(millis() / 1000);
        calibr_v.push_back(v);

        vTaskDelay(BATTERY_CALIBRATION_TIMEOUT);
    }

    vTaskDelete(NULL);
}

void Gamepad_battery::start_calibration(){
    calibr_failed = false;

    xTaskCreatePinnedToCore(
        battery_callibration,
        "batt_calibr",
        BATTERY_CALIBRATION_STACK_SIZE,
        NULL,
        BATTERY_CALIBRATION_TASK_PRIORITY,
        &calibration_handler,
        THIS_CORE
    );

    calibrating = true;
    calibration_start_time = millis();
}

float* Gamepad_battery::finish_calibration(){
    if(!calibrating || calibr_v.size() == 0 || calibr_failed){
        calibrating = false;
        return nullptr;
    }
    calibrating = false;
        
    if(calibration_handler != NULL && eTaskGetState(calibration_handler) != eDeleted)
        vTaskSuspend(calibration_handler);

    if(voltage_levels != nullptr)
        delete [] voltage_levels;
    voltage_levels = new float[BATTERY_LEVELS];

    uint16_t n = calibr_v.size();
    float period = (float) (calibr_time[n - 1] - calibr_time[0]) * (1.0 - BATTERY_ZERO_PERCENTAGE) / (float) BATTERY_LEVELS;
    uint8_t iteration = 1;

    for(int i = 0; i < n; i++){
        if(calibr_time[i] >= period * iteration){
            if(calibr_time[i] != calibr_time[i - 1] && calibr_v[i] != calibr_v[i - 1]){
                float scale = (period * iteration - (float) calibr_time[i - 1]) / (calibr_time[i] - calibr_time[i - 1]);
                voltage_levels[iteration - 1] = calibr_v[i - 1] + (float) (calibr_v[i] - calibr_v[i - 1]) * scale;
            }
            else
                voltage_levels[iteration - 1] = calibr_v[i];

            if(iteration == BATTERY_LEVELS)
                break;
            iteration++;
        }
    }

    calibr_time.clear();
    calibr_v.clear();
    lifetime = (millis() - calibration_start_time) / 60000;

    return voltage_levels;
}

bool Gamepad_battery::is_calibrating(){
    return calibrating;
}

bool Gamepad_battery::calibration_failed(){
    return calibr_failed;
}

bool Gamepad_battery::calibrated(){
    return (voltage_levels != nullptr);
}

float* Gamepad_battery::get_calibration_data(){
    return voltage_levels;
}

void Gamepad_battery::set_calibration_data(float data[BATTERY_LEVELS]){
    if(data == nullptr){
        voltage_levels = nullptr;
        return;
    }

    if(voltage_levels != nullptr)
        delete [] voltage_levels;

    voltage_levels = new float[BATTERY_LEVELS];
    for(uint8_t i = 0; i < BATTERY_LEVELS; i++)
        voltage_levels[i] = data[i];
}




namespace GAMEPAD_GLOBAL{
    Gamepad_battery battery;
}