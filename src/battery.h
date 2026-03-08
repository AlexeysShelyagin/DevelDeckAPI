#ifndef GAMEPAD_BATTERY_H
#define GAMEPAD_BATTERY_H

#include <Arduino.h>
#include <vector>

#include "config.h"

class Gamepad_battery{
    float critical_v;
    float full_v;
    float charging_v;
    float only_charging_v;

    float (*v_adj_func)(float);
    float *voltage_levels = nullptr;

    bool calibrating = false;
    uint64_t calibration_start_time;
    TaskHandle_t calibration_handler = NULL;
public:
    enum Charge_mode_t{
        POWER_ON,
        ONLY_CHARHING,
        POWER_ON_CHARGING
    };
    uint16_t lifetime = 0;

    Gamepad_battery();

    void init(float critical_v_, float full_v_, float charging_v_, float only_charging_v_);
    void set_voltage_adjustment(float (*v_adj_func_ptr)(float));

    float get_battery_voltage();
    uint8_t get_battery_charge();

    Charge_mode_t get_device_mode();

    void start_calibration();
    float* finish_calibration();
    bool is_calibrating();
    bool calibration_failed();

    bool calibrated();
    float* get_calibration_data();
    void set_calibration_data(float data[BATTERY_LEVELS]);
};

#endif