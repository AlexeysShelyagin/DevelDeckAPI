#ifndef GAMEPAD_BATTERY_H
#define GAMEPAD_BATTERY_H

#include <Arduino.h>
#include <vector>
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "config.h"

class Gamepad_battery{
    float critical_v;
    float full_v;
    float charging_v;
    float power_off_v;

    SemaphoreHandle_t batt_pin_mutex;
    
    float (*v_adj_func)(float);
    float *voltage_levels = nullptr;

    bool calibrating = false;
    uint32_t calibration_start_time;
    TaskHandle_t calibration_handler = NULL;
    esp_adc_cal_characteristics_t adc1_chars;
public:
    enum Charge_mode_t{
        POWER_ON,
        POWER_OFF,
        CHARGING
    };
    uint16_t lifetime = 0;

    Gamepad_battery();

    void init(float critical_v_, float full_v_, float charging_v_, float only_charging_v_);
    void set_voltage_adjustment(float (*v_adj_func_ptr)(float));

    float get_battery_voltage();
    uint8_t get_battery_charge(float v = 0);

    Charge_mode_t get_device_mode(float v = 0);

    void start_calibration();
    float* finish_calibration();
    bool is_calibrating();
    bool calibration_failed();

    bool calibrated();
    float* get_calibration_data();
    void set_calibration_data(float *data);
};


namespace GAMEPAD_GLOBAL{
    extern Gamepad_battery battery;
}

#endif