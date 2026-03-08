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

    vec3 basis_x;
    vec3 basis_y;

public:
    Gamepad_accel() = default;

    void init(int sda_pin = ACCEL_SDA_PIN, int scl_pin = ACCEL_SCL_PIN);

    /**
     * @brief Accelerometer chip initial calibration
     * 
     */
    void auto_calibrate();



    /**
     * @brief Set accelerometer zero for normal orientaion (dispay Y is vertical to the ground)
     * 
     */
    void set_vertical_mode();

    /**
     * @brief Set accelerometer zero for ground-parallel orientation
     * 
     */
    void set_horizontal_mode();

    /**
     * @brief Set current hend-held orientaion as zero
     * 
     * @param hold_x_axis does not reset roll (gamepad x axis) if true
     */
    void set_current_as_zero(bool hold_x_axis = false);

    /**
     * @brief Set zero from acceleration vector
     * 
     * @param accel 
     * @param hold_x_axis 
     */
    void set_as_zero(vec3 accel, bool hold_x_axis = false);



    /**
     * @brief Get raw acceleration in m/s^2
     * 
     * @return vec3 
     */
    vec3 get_accel();

    /**
     * @brief Get gamepad tilt angles for x and y axes (roll and pitch)
     * 
     * @param accel raw acceleration data
     * @return vec2: (X_tilt, Y_tilt) in degrees
     */
    vec2 get_angles(vec3 &accel);

    /**
     * @brief Get gamepad tilt angles for x and y axes (roll and pitch)
     * 
     * @return vec2: (X_tilt, Y_tilt) in degrees
     */
    vec2 get_angles();
};

#endif