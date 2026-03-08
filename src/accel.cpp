#include "accel.h"

void Gamepad_accel::init(int sda_pin, int scl_pin){
    accelWire.begin(sda_pin, scl_pin);

    accelWire.beginTransmission(ACCEL_I2C_ADDRESS);
	accelWire.write(0x7E); // Command register
	accelWire.write(0x11); // Set accelerometer to normal mode
	accelWire.endTransmission();

    invert_mask = vec3(
        (ACCEL_INVERT_X_AXIS) ? -1 : 1, 
        (ACCEL_INVERT_Y_AXIS) ? -1 : 1, 
        (ACCEL_INVERT_Z_AXIS) ? -1 : 1
    );

    set_vertical_mode();

    auto_calibrate();
}

void Gamepad_accel::auto_calibrate(){
    accelWire.beginTransmission(ACCEL_I2C_ADDRESS);
    accelWire.write(0x7E); // Command register
    accelWire.write(0x37); // Start accelerometer offset calibration
    accelWire.endTransmission();

    accelWire.beginTransmission(ACCEL_I2C_ADDRESS);
    accelWire.write(0x00);                  // TODO: get the chip_id   0xD1 for BMI160, 0x43 for BMI323
                                            // TODO: add chip recognition, using different register map
    accelWire.endTransmission();

    accelWire.requestFrom(ACCEL_I2C_ADDRESS, 1);
    if(accelWire.available())
        chip = accelWire.read();
}

void Gamepad_accel::set_vertical_mode(){
    basis_x = vec3(0, 1, 0);
    basis_y = vec3(0, 0, -1);
}

void Gamepad_accel::set_horizontal_mode(){
    basis_x = vec3(0, 1, 0);
    basis_y = vec3(1, 0, 0);
}

void Gamepad_accel::set_current_as_zero(bool hold_x_axis){
    vec3 basis_z;

    for(uint8_t i = 0; i < ACCEL_CALIBRATION_MEASURE_N; i++){
        basis_z += get_accel();
        delay(1);
    }
    
    set_as_zero(basis_z);
}

void Gamepad_accel::set_as_zero(vec3 basis_z, bool hold_x_axis){
    if(hold_x_axis)
        basis_z.y = 0;
    basis_z = basis_z.norm();

    basis_x = vec3(basis_z.y, -basis_z.x, 0).norm();
    basis_y = basis_z.cross(basis_x).norm();
}

vec3 Gamepad_accel::get_accel(){
    int16_t ax = 0, ay = 0, az = 0;
    if(chip == BMI160_ID){
        accelWire.beginTransmission(ACCEL_I2C_ADDRESS);
        accelWire.write(0x12); // Start register for accelerometer data
        accelWire.endTransmission(false);
        accelWire.requestFrom(ACCEL_I2C_ADDRESS, 6);
        
        
        if (accelWire.available() == 6) {
            ax = accelWire.read() | (accelWire.read() << 8);
            ay = accelWire.read() | (accelWire.read() << 8);
            az = accelWire.read() | (accelWire.read() << 8);
        }
    }
    
    if(chip == BMI323_ID){
        // TODO:
    }

    vec3 data(ax, ay, az);
    data *= (g_const / ACCEL_SENSITIVITY);
    data *= invert_mask;

    return data;
}

vec2 Gamepad_accel::get_angles(vec3 &accel){
    vec2 ang = vec2(
        acos(accel.fast_norm().dot(basis_x)) - HALF_PI,
        acos(accel.fast_norm().dot(basis_y)) - HALF_PI
    );

    return ang * RAD_TO_DEG;
}

vec2 Gamepad_accel::get_angles(){
    vec3 accel = get_accel();
    return get_angles(accel);
}