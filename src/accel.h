#ifndef ACCEL_H
#define ACCEL_H

#include <Wire.h>
#include <Arduino.h>

#include "config.h"
#include "vectors.h"

const float g_const = 9.81;

class Gamepad_accel{
    enum chip_type_t{
        BMI160_ID = 0xD1,
        BMI323_ID = 0x43
    };
    uint8_t chip = 0;

    TwoWire accelWire = TwoWire(0);
    vec3 invert_mask;
    uint8_t mode = 0;
public:
    Gamepad_accel() = default;

    void init(int sda_pin = ACCEL_SDA_PIN, int scl_pin = ACCEL_SCL_PIN);

    void auto_calibrate();

    void set_vertical_mode();
    void set_horizontal_mode();

    vec3 get_accel();
    vec2 get_angles(vec3 &accel);
    vec2 get_angles();
};

#endif