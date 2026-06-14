#include "accel.h"

#define REG_CMD_BMI160 0x7E
#define REG_DATA_BMI160 0x12
#define CMD_NORMAL_MODE_BMI160 0x11
#define CMD_OFFSET_CALIBR_BMI160 0x37


void DD_accel::init(int sda_pin, int scl_pin){
    _i2c.begin(sda_pin, scl_pin);

    _i2c.beginTransmission(ACCEL_I2C_ADDRESS);
	_i2c.write(REG_CMD_BMI160);
	_i2c.write(0x11);
	_i2c.endTransmission();

    invert_mask = vec3(
        (ACCEL_INVERT_X_AXIS) ? -1.0f : 1.0f, 
        (ACCEL_INVERT_Y_AXIS) ? -1.0f : 1.0f, 
        (ACCEL_INVERT_Z_AXIS) ? -1.0f : 1.0f
    );

    set_vertical_mode();

    auto_calibrate();
}

void DD_accel::auto_calibrate(){
    _i2c.beginTransmission(ACCEL_I2C_ADDRESS);
    _i2c.write(REG_CMD_BMI160);
    _i2c.write(CMD_OFFSET_CALIBR_BMI160);
    _i2c.endTransmission();

    _i2c.beginTransmission(ACCEL_I2C_ADDRESS);
    _i2c.write(0x00);                  // TODO: get the chip_id   0xD1 for BMI160, 0x43 for BMI323
                                            // TODO: add chip recognition, using different register map
    _i2c.endTransmission();

    _i2c.requestFrom(ACCEL_I2C_ADDRESS, 1);
    if(_i2c.available())
        chip = _i2c.read();
}

void DD_accel::set_vertical_mode(){
    basis_x = vec3(0.0f, 1.0f, 0.0f);
    basis_y = vec3(0.0f, 0.0f, -1.0f);
}

void DD_accel::set_horizontal_mode(){
    basis_x = vec3(0.0f, 1.0f, 0.0f);
    basis_y = vec3(1.0f, 0.0f, 0.0f);
}

void DD_accel::set_current_as_zero(bool hold_x_axis){
    vec3 basis_z;
    for(uint8_t i = 0; i < ACCEL_CALIBRATION_MEASURE_N; i++){
        basis_z += get_accel();
        delay(1);
    }
    
    set_as_zero(basis_z, hold_x_axis);
}

void DD_accel::set_as_zero(vec3 basis_z, bool hold_x_axis){
    if(hold_x_axis)
        basis_z.y = 0.0f;
    basis_z = basis_z.norm();

    basis_x = vec3(basis_z.y, -basis_z.x, 0.0f).norm();
    basis_y = basis_z.cross(basis_x).norm();
}

vec3 DD_accel::get_accel(){
    int16_t ax = 0, ay = 0, az = 0;
    if(chip == BMI160_ID){
        _i2c.beginTransmission(ACCEL_I2C_ADDRESS);
        _i2c.write(REG_DATA_BMI160);
        _i2c.endTransmission(false);
        _i2c.requestFrom(ACCEL_I2C_ADDRESS, 6);
        
        
        if (_i2c.available() == 6) {
            ax = _i2c.read() | (_i2c.read() << 8);
            ay = _i2c.read() | (_i2c.read() << 8);
            az = _i2c.read() | (_i2c.read() << 8);
        }
    }
    
    if(chip == BMI323_ID){
        // TODO:
    }

    vec3 data(ax, ay, az);
    data *= (GRAVITY_MS2 / ACCEL_SENSITIVITY);
    data *= invert_mask;

    return data;
}

vec2 DD_accel::get_angles(vec3 accel){
    vec2 ang = vec2(
        acos(accel.fast_norm().dot(basis_x)) - HALF_PI,
        acos(accel.fast_norm().dot(basis_y)) - HALF_PI
    );

    return ang * RAD_TO_DEG;
}

vec2 DD_accel::get_angles(){
    return get_angles(get_accel());
}