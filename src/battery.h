#ifndef GAMEPAD_BATTERY_H
#define GAMEPAD_BATTERY_H

#include <Arduino.h>
#include <vector>

#include "config.h"

class Gamepad_battery{
    float slope_adj_k;
    float v_offset;
    float *voltage_levels = nullptr;

    bool calibrating = false;
    uint64_t calibration_start_time;
    TaskHandle_t calibration_handler = NULL;
public:
    enum charge_mode{
        POWER_ON,
        ONLY_CHARHING,
        POWER_ON_CHARGING
    };
    uint16_t lifetime = 0;

    Gamepad_battery() = default;

    void init();

    float get_battery_voltage();
    uint8_t get_battery_charge();

    charge_mode get_device_mode();

    void start_calibration();
    float* finish_calibration();
    bool is_calibrating();
    bool calibration_failed();

    bool calibrated();
    float* get_calibration_data();
    void set_calibration_data(float data[BATTERY_LEVELS]);
};

#endif