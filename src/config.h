#ifndef GAMEPAD_CONFIG_H
#define GAMEPAD_CONFIG_H

#include <Arduino.h>




// ##################################################################################
//                                  PIN SETUP
// ##################################################################################



// ================ BUTTONS =======================

//  |===================================|
//  |                              MENU |
//  |                                   |
//  |       UP                     A    |
//  |  LEFT   RIGHT                     |
//  |      DOWN                    B    |
//  |===================================|


#define LEFT_BUT_PIN 17
#define UP_BUT_PIN 5
#define RIGHT_BUT_PIN 4
#define DOWN_BUT_PIN 16
#define A_BUT_PIN 39
#define B_BUT_PIN 34
#define MENU_BUT_PIN 36

#define INVERT_BUTTONS_STATE 0

#define BUTTONS_N 7
static int16_t buttons_map[] = {
                                LEFT_BUT_PIN, 
                                UP_BUT_PIN, 
                                RIGHT_BUT_PIN, 
                                DOWN_BUT_PIN, 
                                A_BUT_PIN, 
                                B_BUT_PIN, 
                                MENU_BUT_PIN
                            };

enum buttons_id{
    LEFT_BUT_ID,
    UP_BUT_ID,
    RIGHT_BUT_ID,
    DOWN_BUT_ID,
    A_BUT_ID,
    B_BUT_ID,
    MENU_BUT_ID
};



// ================ DISPLAY =======================

#define DISP_CS_PIN 13
#define DISP_DC_PIN 22
#define DISP_RST_PIN -1
#define DISP_BACKLIGHT_PIN 25
#define DISP_BACKLIGHT_LEDC_CHANNEL 0



// ================ SD_CARD =======================

#define SD_CS_PIN 21



// ================ BUZZER =======================

#define BUZZ_PIN 32
#define BUZZ_LEDC_CHANNEL 1



// ============== ACCELEROMETER =================

#define ACCEL_SDA_PIN 26
#define ACCEL_SCL_PIN 27
#define ACCEL_I2C_ADDRESS 0x68

#define ACCEL_INVERT_X_AXIS true
#define ACCEL_INVERT_Y_AXIS true
#define ACCEL_INVERT_Z_AXIS false



// ================ VIBRO =======================

#define VIBRO_PIN 33
#define VIBRO_LEDC_CHANNEL 2



// ================ BATTERY =======================

#define BATTERY_V_PIN 35



// ##################################################################################







// ##################################################################################
//                              DISPLAY PARAMETERS
// ##################################################################################



#define DISP_WIDTH 320
#define DISP_HEIGHT 240

#define DISP_ROTATION 1
#define DEFAULT_COLOR_DEPTH 8
#define DEFAULT_CANVAS_T TFT_eSprite
#define BRIGHTNESS_LEVELS 10
#define DEFAULT_BRIGHTNESS 10


#define FONTS_MAX_N 8       // How many font slots are in memory



// TFT display has its own refresh rate, this leads to flickering, and diogonal lines during frequent updates
// Especially if the contrast between frames is high

// There are, when the flickering is minimal. Diagonal lines just freeze on the same place, or move slowly
// Those FPSs should be maintained to avoid display flickering

#define NO_FLICKERING_FPS_1 30.31f
#define NO_FLICKERING_FPS_2 20.35f
#define NO_FLICKERING_FPS_3 37.242f
#define NO_FLICKERING_FPS_4 15.15f
#define NO_FLICKERING_FPS_5 13.34f



// ##################################################################################







// ##################################################################################
//                           BATTERY HARDWARE PARAMETERS
// ##################################################################################


//      BATT_VCC-------[===R2===]------*-------[===R1===]--------GND
//                                     |
//                                 V_READ_PIN



#define BATTERY_DIVIDER_R1 10.0
#define BATTERY_DIVIDER_R2 10.0
#define BATTERY_V_REF 3.3                               // Logical 1 voltage

#define BATTERY_CRITICAL_V 3.55                         // At this voltage device would turn off
#define BATTERY_FULL_V 4.15                             // Voltage considered as fully charged battery, for proportional charge level calculations
#define BATTERY_CHARGING_V 4.3                          // value above that would mean that device is connected to the charger
#define BATTERY_ONLY_CHARGING_V 4.5                     // voltage above that would mean that devices is connected to charger and powered off (VCC pulled to 5v)



// Battery voltage is converted to the charge level
// Levels are presented as:
//      [1234...N]    e.g.  [##  ] 2 of 4 charge
//      [        ]  - battery zero level.
//                    Zero level is level above BATTERY_ZERO_PERCENTAGE

#define BATTERY_LEVELS 4
#define BATTERY_ZERO_PERCENTAGE 0.05                    // from 0 to 1



// To avoid analog noise, the value of battery voltage is read multiple times
#define BATTERY_N_OF_MEASURES 5



// Battery voltage is calculated from analog value and converted to RAW voltage
// However, adjustment function is needed, due to internal resistanse, nonlinearity of ADC etc.
#define BATTERY_VADJ_FUNC(v) -0.1961 + 1.5359*v - 0.0767*v*v    // best experimental fit
//#define BATTERY_VADJ_FUNC(v) 0.890 * v + 1.158                // theor approx  



// ##################################################################################







// ##################################################################################
//                            PERIPHERY PARAMETERS
// ##################################################################################



// ============== ACCELEROMETER =================

// Sensitivity for ±2g in LSB/g
#define ACCEL_SENSITIVITY 16384.0       // (float) 2^14



// ================ BUTTONS =======================

// double-triggering of button state would be filtered out if the timeot is less this value
#define BUTTON_FILTERING_TIME 15        // (ms)



// ================ BUZZER =======================

#define BUZZER_VOLUME_LEVELS 10
#define DEFAULT_BUZZER_VOLUME 10



// ================ VIBRO =======================

#define VIBRO_STRENGTH_LEVELS 3
#define DEFAULT_VIBRO_STRENGTH 2



// ##################################################################################







// ##################################################################################
//                            GAMEPAD SYSTEM CONFIG
// ##################################################################################



#define GAMEPAD_DATA_FILE_NAME "/gamepad.dat"
#define GAME_CONFIG_FILE_NAME "game.ini"



#define API_VERSION "0.0.1"



// ================= Dialogue boxes =======================

#define GAME_FILES_NOT_FOUND_MSG "Game files\nnot found.\nLocate them?"
#define NOT_GAME_FOLDER_MSG "Not a game\n folder selected"
#define NO_SD_CARD_MSG "SD card is not\ninserted"
#define FACTORY_RESET_MSG "Perform factory\nreset?\n(Console will\n restart)"
#define BATTERY_CALIBRATION_MSG "Battery is\ncalibrating"
#define BATTERY_CALIBRATION_ALERT "To calibrate battery:\n 1. Make sure battery is fully charged\n 2. Do not power off gamepad until battery fully discharges\n 3. Do not connect the charger\n\n (You can use the gamepad during the calibration)"



// ##################################################################################







// ##################################################################################
//                            SUBPROCESSES CONFIG
// ##################################################################################



// Subprocesses timeouts
// All values are in [ms]

#define BATTERY_CHECK_TIMEOUT 2000 // TODO: change to 10000
#define BATTERY_LIGHT_SLEEP_CHECK_TIMEOUT 10000
#define BATTERY_LOW_CHARGE_ALARM_TIMEOUT 30000
#define BATTERY_CALIBRATION_TIMEOUT 60000
#define DEVICE_MODE_CHECK_TIMEOUT 500
#define SYSTEM_DATA_AUTOSAVE_TIMEOUT 10000
#define SD_PRESENCE_CHECK_TIMEOUT 3000
#define FORCED_MENU_CHECK_TIMEOUT 500
#define FORCED_MENU_HOLD_TIME 4000
#define NOTIFICATION_HOLD_TIME 2000



// RTOS stack sizes per process

#define FORCED_MENU_STACK_SIZE 4096
#define BATTERY_LISTENER_STACK_SIZE 4096 // TODO: Change
#define NOTIFICATION_DESTRUCTOR_STACK_SIZE 2048
#define DISPLAY_UPDATE_THREAD_STACK_SIZE 2048



// ##################################################################################







// ##################################################################################
//                                    UI
// ##################################################################################



// ================= File manager =======================

#define FILE_MANAGER_W 280
#define FILE_MANAGER_H 200
#define GAME_ICON_SIZE 64
#define FILE_MANAGER_MAX_FILE_NAME_LENGTH 30



// ================= Settings window =======================

#define SETTINGS_W 320
#define SETTINGS_H 240



// ================= Dialogue boxes =======================

#define MSG_BOX_DEFAULT_ACTION "Ok"



// ##################################################################################


#endif