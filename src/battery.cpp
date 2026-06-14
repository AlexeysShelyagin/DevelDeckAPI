#include <battery.h>

namespace DD_GLOBAL{
    DD_battery battery;
}



const float INV_DIVIDER_VAL = (float) (BATTERY_DIVIDER_R1 + BATTERY_DIVIDER_R2) / BATTERY_DIVIDER_R1;

std::vector < float > calibr_v;
bool calibr_failed = false;
float critical_v_global;



float clamp(float val, float min_ = 0.0f, float max_ = 1.0f){
    return max(min(val, max_), min_);
}

float identity_v_adj(float v){
    return v;
}

void callibration_task(void *params){
    float v = DD_GLOBAL::battery.get_voltage();
    while(v > critical_v_global){
        if(DD_GLOBAL::battery.get_device_mode() != DD_battery::POWER_ON){
            calibr_failed = true;
            break;
        }

        calibr_v.push_back(DD_GLOBAL::battery.get_voltage());
        
        vTaskDelay(BATTERY_CALIBRATION_TIMEOUT);
    }

    vTaskDelete(NULL);
}



DD_battery::DD_battery(){
    v_adj_func = identity_v_adj;
}

void DD_battery::init(float critical_v_, float full_v_, float charging_v_, float only_charging_v_){
    critical_v = critical_v_global = critical_v_;
    full_v = full_v_;
    charging_v = charging_v_;
    power_off_v = only_charging_v_;

    batt_pin_mutex = xSemaphoreCreateMutex();

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(BATTERY_ADC_CHANNEL, ADC_ATTEN_DB_12);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 0, &adc1_chars);
}

void DD_battery::set_voltage_adjustment(float (*v_adj_func_ptr)(float)){
    v_adj_func = v_adj_func_ptr;
}


float DD_battery::get_voltage(){
    while(!xSemaphoreTake(batt_pin_mutex, portMAX_DELAY));      // wait until analog read is available

    int raw_read = 0;
    for(uint8_t i = 0; i < BATTERY_N_OF_MEASURES; i++)
        raw_read += adc1_get_raw(BATTERY_ADC_CHANNEL);
    raw_read = (float) raw_read / BATTERY_N_OF_MEASURES;

    float v_raw = esp_adc_cal_raw_to_voltage(raw_read, &adc1_chars);
    v_raw = v_raw * INV_DIVIDER_VAL / 1000.0f;

    xSemaphoreGive(batt_pin_mutex);

    return v_adj_func(v_raw); 
}

uint8_t DD_battery::get_charge(float v){
    if (v == 0.0f)
        v = get_voltage();

    if(voltage_levels == nullptr){
        return roundf(clamp((v - critical_v) / (full_v - critical_v)) * BATTERY_LEVELS);
    }

    uint8_t i = 0.0f;
    while(i < BATTERY_LEVELS && voltage_levels[i] > v)
        i++;

    return BATTERY_LEVELS - i;
}

DD_battery::Mode_t DD_battery::get_device_mode(float v){
    if(v == 0.0f)
        v = get_voltage();
    
    if(v > power_off_v)
        return POWER_OFF;
    if(v > charging_v)
        return CHARGING;
    return POWER_ON;
}

void DD_battery::start_calibration(){
    calibr_failed = false;

    xTaskCreatePinnedToCore(
        callibration_task,
        "batt_calibr",
        BATTERY_CALIBRATION_STACK_SIZE,
        NULL,
        BATTERY_CALIBRATION_TASK_PRIORITY,
        &calibration_handler,
        DIFFERENT_CORE
    );

    calibrating = true;
    calibration_start_time = millis();
}

float* DD_battery::finish_calibration(){
    if(!calibrating || calibr_v.size() == 0 || calibr_failed){
        calibrating = false;
        return nullptr;
    }
    calibrating = false;
        
    if(calibration_handler != NULL && eTaskGetState(calibration_handler) != eDeleted)
        vTaskDelete(calibration_handler);

    if(voltage_levels != nullptr)
        delete [] voltage_levels;
    voltage_levels = new float[BATTERY_LEVELS];


    float level_width = (float) calibr_v.size() * (1.0f-BATTERY_ZERO_PERCENTAGE) / BATTERY_LEVELS;
    for(uint8_t i = 0; i < BATTERY_LEVELS; i++){
        uint8_t idx = level_width * i;
        voltage_levels[i] = calibr_v[idx]; 
    }

    lifetime = (millis() - calibration_start_time) / 60000;

    return voltage_levels;
}

bool DD_battery::is_calibrating(){
    return calibrating;
}

bool DD_battery::is_calibration_failed(){
    return calibr_failed;
}

bool DD_battery::is_calibrated(){
    return (voltage_levels != nullptr);
}

float* DD_battery::get_calibration_data(){
    return voltage_levels;
}

void DD_battery::set_calibration_data(float *data){
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