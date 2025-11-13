#include <battery.h>

const float batt_inv_divider_val = (float) (BATTERY_DIVIDER_R1 + BATTERY_DIVIDER_R2) / BATTERY_DIVIDER_R1;

#ifndef BATTERY_VADJ_FUNC
#define BATTERY_VADJ_FUNC(v) v
#endif

std::vector < uint16_t > calibr_time;
std::vector < float > calibr_v;
bool calibr_failed = false;

float clamp(float val, float min_ = 0, float max_ = 1){
    return max(min(val, max_), min_);
}

void Gamepad_battery::init(){
    adcAttachPin(BATTERY_V_PIN);
    analogReadResolution(12);
}


float Gamepad_battery::get_battery_voltage(){
    float pin_val = 0;
    for(uint8_t i = 0; i < BATTERY_N_OF_MEASURES; i++)
        pin_val += analogRead(BATTERY_V_PIN);
    pin_val /= (float) BATTERY_N_OF_MEASURES;

    float v_raw = pin_val * BATTERY_V_REF * batt_inv_divider_val / 4096.0;

    return BATTERY_VADJ_FUNC(v_raw); 
}

uint8_t Gamepad_battery::get_battery_charge(){
    float v = get_battery_voltage();

    if(voltage_levels == nullptr){
        return round(clamp((v - BATTERY_CRITICAL_V) / (BATTERY_FULL_V - BATTERY_CRITICAL_V)) * BATTERY_LEVELS);
    }

    uint8_t i = 0;
    while(i < BATTERY_LEVELS && voltage_levels[i] > v)
        i++;

    return BATTERY_LEVELS - i;
}

Gamepad_battery::charge_mode Gamepad_battery::get_device_mode(){
    float v = get_battery_voltage();
    if(v > BATTERY_ONLY_CHARGING_V)
        return ONLY_CHARHING;
    if(v > BATTERY_CHARGING_V)
        return POWER_ON_CHARGING;
    return POWER_ON;
}

void battery_callibration(void *params){
    Gamepad_battery *batt = (Gamepad_battery *) params;
    float v = batt -> get_battery_voltage();
    while(v > BATTERY_CRITICAL_V){
        if(batt -> get_device_mode() != Gamepad_battery::POWER_ON){
            calibr_failed = true;
            break;
        }

        v = batt -> get_battery_voltage();

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
        1024,
        this,
        1,
        &calibration_handler,
        xPortGetCoreID()
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