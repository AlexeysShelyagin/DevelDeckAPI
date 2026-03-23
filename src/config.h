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

enum Buttons_id_t{
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

#define SD_SPI_FREQUENCY 80000000


// ================ BUZZER =======================

#define BUZZ_PIN 32
#define BUZZ_LEDC_CHANNEL 1



// ============== ACCELEROMETER =================

#define ACCEL_SDA_PIN 26
#define ACCEL_SCL_PIN 27
#define ACCEL_I2C_ADDRESS 0x68

#define ACCEL_INVERT_X_AXIS false
#define ACCEL_INVERT_Y_AXIS false
#define ACCEL_INVERT_Z_AXIS true



// ================ VIBRO =======================

#define VIBRO_PIN 33
#define VIBRO_LEDC_CHANNEL 2



// ================ BATTERY =======================

//#define BATTERY_V_PIN 35
#define BATTERY_ADC_CHANNEL ADC1_CHANNEL_7



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


// Battery voltage is converted to the charge level
// Levels are presented as:
//      [1234...N]    e.g.  [##  ] 2 of 4 charge
//      [        ]  - battery zero level.
//                    Zero level is level above BATTERY_ZERO_PERCENTAGE

#define BATTERY_LEVELS 4
#define BATTERY_ZERO_PERCENTAGE 0.05                    // from 0 to 1



// To avoid analog noise, the value of battery voltage is read multiple times
#define BATTERY_N_OF_MEASURES 10

// Battery voltage is calculated from analog value and converted to RAW voltage
// However, adjustment function is needed, due to internal resistanse, nonlinearity of ADC etc.
#define BATTERY_VADJ_FUNC [](float v) -> float{ return v + 0.3095; }

#define BATTERY_CRITICAL_V 3.40                         // At this voltage device would turn off
#define BATTERY_FULL_V 4.15                             // Voltage considered as fully charged battery, for proportional charge level calculations
#define BATTERY_CHARGING_V 4.20                         // value above that would mean that device is connected to the charger
#define BATTERY_ONLY_CHARGING_V 4.80                    // value above that would bean that device is charging and powered off



// ##################################################################################







// ##################################################################################
//                            PERIPHERY PARAMETERS
// ##################################################################################



// ============== ACCELEROMETER =================

// Sensitivity for ±2g in LSB/g
#define ACCEL_SENSITIVITY 16384.0       // (float) 2^14
#define ACCEL_CALIBRATION_MEASURE_N 10



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



const char GAMEPAD_DATA_FILE_NAME[] PROGMEM = "/gamepad.dat";
const char GAME_CONFIG_FILE_NAME[] PROGMEM = "game.ini";


// Major version number (X.x.x)
#define DEVELDECK_API_VERSION_MAJOR 0
// Minor version number (x.X.x)
#define DEVELDECK_API_VERSION_MINOR 2
// Patch version number (x.x.X)
#define DEVELDECK_API_VERSION_PATCH 0

const char DEVELDECK_API_VERSION[] PROGMEM = "0.1.1";


#define DUMP_SYS_DATA_ON_INIT



// ================= Dialogue boxes =======================

const char GAME_FILES_NOT_FOUND_MSG[] PROGMEM = "Game files\nnot found.\nLocate them?";
const char NOT_GAME_FOLDER_MSG[] PROGMEM = "Not a game\n folder selected";
const char NO_SD_CARD_MSG[] PROGMEM = "SD card is not\ninserted";
const char FACTORY_RESET_MSG[] PROGMEM = "Perform factory\nreset?\n(Console will\n restart)";
const char BATTERY_CALIBRATION_MSG[] PROGMEM = "Battery is\ncalibrating";
const char BATTERY_CALIBRATION_ALERT[] PROGMEM = "To calibrate battery:\n 1. Make sure battery is fully charged\n 2. Do not power off gamepad until battery fully discharges\n 3. Do not connect the charger\n\n (You can use the gamepad during the calibration)";


// ================= UI text =======================

const char TXT_DISPLAY_ALLOC_FAILED[] PROGMEM = "ERROR: not enough heap for display buffer allocation";
const char TXT_DEFAULT_BITDEPTH_FAILED[] PROGMEM = "WARNING: bitdepth reduced to %d from %d\n";
const char TXT_FAILED_CALIBR[] PROGMEM = "Failed to\ncalibrate battery";
const char TXT_DISPAY_FAILED[] PROGMEM = "ERROR: unable to initialize display";
const char TXT_SD_FAILED[] PROGMEM = "ERROR: failed to init SD card";
const char TXT_SD_DISCONNECT[] PROGMEM = "ERROR: SD is disconnected";
const char TXT_SETTINGS_SAVE_WARINING[] PROGMEM = "Settings won't\nbe saved\nafter reset";
const char TXT_UNSUPPORTED_DEVICE[] PROGMEM = "Unsupported on\nyour device\n";
const char TXT_SPIFFS_FAILED[] PROGMEM = "ERROR: failed to init SPIFFS";

const char TXT_USUPPORTED_ON_DEVICE[] PROGMEM = "Unsupported on your device";
const char TXT_BUZZ_VOL[] PROGMEM = "Buzz. vol:";
const char TXT_BRIGHTNESS[] PROGMEM = "Brightness: ";
const char TXT_VIBRO[] PROGMEM = "Vibro: ";
const char TXT_BATT_CALIBR[] PROGMEM = "Battery calibration";
const char TXT_BATT_LIFETIME[] PROGMEM = "Battery lifetime: ";
const char TXT_FACTORY_RESET[] PROGMEM = "Factory reset";
const char TXT_SAVE_MSG[] PROGMEM = "\nSave changes?";
const char TEXT_UNABLE_CREATE_MSGBOX[] PROGMEM = "ERROR: unable to create message box";
const char TEXT_UNABLE_CREATE_NOTIFF[] PROGMEM = "ERROR: unable to create notification";


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