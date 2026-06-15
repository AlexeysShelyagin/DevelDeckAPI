#ifndef DD_CONFIG_H
#define DD_CONFIG_H

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


#define DD_LEFT_BUT_PIN 17
#define DD_UP_BUT_PIN 5
#define DD_RIGHT_BUT_PIN 4
#define DD_DOWN_BUT_PIN 16
#define DD_A_BUT_PIN 39
#define DD_B_BUT_PIN 34
#define DD_MENU_BUT_PIN 36

#define DD_BUTTONS_INV 0

#define DD_BUTTONS_N 7
static int16_t buttons_map[] = {
                                DD_LEFT_BUT_PIN, 
                                DD_UP_BUT_PIN, 
                                DD_RIGHT_BUT_PIN, 
                                DD_DOWN_BUT_PIN, 
                                DD_A_BUT_PIN, 
                                DD_B_BUT_PIN, 
                                DD_MENU_BUT_PIN
                            };

enum Buttons_id_t : uint8_t{
    LEFT_BUT_ID = 0,
    UP_BUT_ID,
    RIGHT_BUT_ID,
    DOWN_BUT_ID,
    A_BUT_ID,
    B_BUT_ID,
    MENU_BUT_ID
};



// ================ DISPLAY =======================

#define DD_DISP_CS_PIN 13
#define DD_DISP_DC_PIN 22
#define DD_DISP_RST_PIN -1
#define DD_DISP_BACKLIGHT_PIN 25
#define DD_DISP_BACKLIGHT_LEDC_CHANNEL 0



// ================ SD_CARD =======================

#define DD_SD_CS_PIN 21

#define DD_SD_SPI_FREQUENCY 80000000


// ================ BUZZER =======================

#define DD_BUZZ_PIN 32
#define DD_BUZZ_LEDC_CHANNEL 1



// ============== ACCELEROMETER =================

#define DD_ACCEL_SDA_PIN 26
#define DD_ACCEL_SCL_PIN 27
#define DD_ACCEL_I2C_ADDRESS 0x68

#define DD_ACCEL_INV_X false
#define DD_ACCEL_INV_Y false
#define DD_ACCEL_INV_Z true



// ================ VIBRO =======================

#define DD_VIBRO_PIN 33
#define DD_VIBRO_LEDC_CHANNEL 2



// ================ BATTERY =======================

//#define DD_BATTERY_V_PIN 35
#define DD_BATTERY_ADC_CHANNEL ADC1_CHANNEL_7



// ##################################################################################







// ##################################################################################
//                              DISPLAY PARAMETERS
// ##################################################################################



#define DISP_WIDTH 320
#define DISP_HEIGHT 240

#define DISP_ROTATION 1
#define DEFAULT_COLOR_DEPTH 8
#define DEFAULT_CANVAS_T TFT_eSprite
#define DISP_BRIGHTNESS_LEVELS 10
#define DISP_DEFAULT_BRIGHTNESS 10


#define FONTS_MAX_N 8       // How many font slots are in memory



// TFT display has its own refresh rate, this leads to flickering, and diogonal lines during frequent updates
// Especially if the contrast between frames is high

// There are, when the flickering is minimal. Diagonal lines just freeze on the same place, or move slowly
// Those FPSs should be maintained to avoid display flickering

#define NO_FLICKERING_FPS_1 30.304f
#define NO_FLICKERING_FPS_2 20.0004f
#define NO_FLICKERING_FPS_3 17.2417f
#define NO_FLICKERING_FPS_4 15.1517f
#define NO_FLICKERING_FPS_5 13.3336f
#define NO_FLICKERING_FPS_6 12.049f



// ##################################################################################







// ##################################################################################
//                           BATTERY HARDWARE PARAMETERS
// ##################################################################################


//      BATT_VCC-------[===R2===]------*-------[===R1===]--------GND
//                                     |
//                                 V_READ_PIN



#define DD_BATTERY_DIV_R1 10.0
#define DD_BATTERY_DIV_R2 10.0
#define DD_BATTERY_V_REF 3.3                               // Logical 1 voltage


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
#define BATTERY_VADJ_FUNC [](float v)->float{ return v + 0.3095; }

#define BATTERY_CRITICAL_V 3.40                         // At this voltage device would turn off
#define BATTERY_FULL_V 4.15                             // Voltage considered as fully charged battery, for proportional charge level calculations
#define BATTERY_CHARGING_V 4.20                         // value above that would mean that device is connected to the charger
#define BATTERY_ONLY_CHARGING_V 4.80                    // value above that would bean that device is charging and powered off
// TODO: only charging ~4.59 when fully discharged

#define BATTERY_DISCHARGED_DEADBAND 0.1                 // to this value battery voltage need to increase to turn on the system


// ##################################################################################







// ##################################################################################
//                            PERIPHERY PARAMETERS
// ##################################################################################



// ============== ACCELEROMETER =================

// Sensitivity for ±2g in LSB/g
#define DD_ACCEL_SENS 16384.0       // (float) 2^14
#define DD_ACCEL_CALIBR_MEASURE_N 10



// ================ BUTTONS =======================

// double-triggering of button state would be filtered out if the timeot is less this value
#define DD_BUTTON_FILTERING_TIME_MS 15        // (ms)



// ================ BUZZER =======================

#define BUZZER_VOLUME_LEVELS 10
#define DEFAULT_BUZZER_VOLUME 10



// ================ VIBRO =======================

#define VIBRO_STRENGTH_LEVELS 3
#define DEFAULT_VIBRO_STRENGTH 2



// ##################################################################################







// ##################################################################################
//                            DEVELDECK SYSTEM CONFIG
// ##################################################################################



const char DEVELDECK_DATA_FILE_NAME[] PROGMEM = "/develdeck.dat";
const char GAME_CONFIG_FILE_NAME[] PROGMEM = "game.ini";


// Major version number (X.x.x)
#define DD_API_VERSION_MAJOR 0
// Minor version number (x.X.x)
#define DD_API_VERSION_MINOR 3
// Patch version number (x.x.X)
#define DEVELDECK_API_VERSION_PATCH 3

const char DEVELDECK_API_VERSION[] PROGMEM = "0.3.3";


#define DD_DUMP_SYS_DATA_ON_INIT



// ================= Dialogue boxes =======================

const char DDMSG_GAME_FILES_NOT_FOUND[] PROGMEM = "Game files\nnot found.\nLocate them?";
const char DDMSG_NOT_GAME_FOLDER[] PROGMEM = "Not a game\n folder selected";
const char DDMSG_NO_SD_CARD[] PROGMEM = "SD card is not\ninserted";
const char DDMSG_FACTORY_RESET[] PROGMEM = "Perform factory\nreset?\n(Console will\n restart)";
const char DDMSG_BATTERY_CALIBRATION[] PROGMEM = "Battery is\ncalibrating";
const char DDMSG_BATTERY_CALIBRATION_FAILED[] PROGMEM = "Failed to\ncalibrate battery";
const char DDMSG_UNPLUG_FOR_CALIBRATION[] PROGMEM = "Unplug the\ncharger\nto calibrate";
const char DDMSG_BATTERY_CALIBRATION_ALERT[] PROGMEM = "To calibrate battery:\n 1. Make sure battery is fully charged\n 2. Do not power off gamepad until battery fully discharges\n 3. Do not connect the charger\n\n (You can use the gamepad during the calibration)";


// ================= UI text =======================

const char DDTXT_DISPLAY_ALLOC_FAILED[] PROGMEM = "ERROR: not enough heap for display buffer allocation";
const char DDTXT_DEFAULT_BITDEPTH_FAILED[] PROGMEM = "WARNING: bitdepth reduced to %d from %d\n";
const char DDTXT_DISCHARGED[] PROGMEM = "Discharged";
const char DDTXT_LOW_CHARGE_ALARM[] PROGMEM = "Low charge";
const char DDTXT_DISPAY_FAILED[] PROGMEM = "ERROR: unable to initialize display";
const char DDTXT_SD_FAILED[] PROGMEM = "ERROR: failed to init SD card";
const char DDTXT_SD_DISCONNECT[] PROGMEM = "ERROR: SD is disconnected";
const char DDTXT_UNSUPPORTED_DEVICE[] PROGMEM = "Unsupported on\nyour device\n";
const char DDTXT_SPIFFS_FAILED[] PROGMEM = "ERROR: failed to init SPIFFS";

const char DDTXT_USUPPORTED_ON_DEVICE[] PROGMEM = "Unsupported on your device";
const char DDTXT_BUZZ_VOL[] PROGMEM = "Buzz. vol:";
const char DDTXT_BRIGHTNESS[] PROGMEM = "Brightness: ";
const char DDTXT_VIBRO[] PROGMEM = "Vibro: ";
const char DDTXT_BATT_CALIBR[] PROGMEM = "Battery calibration";
const char DDTXT_BATT_LIFETIME[] PROGMEM = "Battery lifetime: ";
const char DDTXT_FACTORY_RESET[] PROGMEM = "Factory reset";
const char DDTXT_SAVE_Q[] PROGMEM = "\nSave changes?";
const char DDTXT_UNABLE_CREATE_MSGBOX[] PROGMEM = "ERROR: unable to create message box";
const char DDTXT_UNABLE_CREATE_NOTIFF[] PROGMEM = "ERROR: unable to create notification";


// ##################################################################################







// ##################################################################################
//                            SUBPROCESSES CONFIG
// ##################################################################################



// Subprocesses timeouts
// All values are in [ms]

#define DD_TIMEOUT_SYS_EVENT_CHECK 500

#define DD_TIMEOUT_BATTERY_LEVEL_CHECK 10000

#define DD_TIMEOUT_BATTERY_LIGHT_SLEEP_CHECK 5000
#define DD_TIMEOUT_BATTERY_LOW_CHARGE_ALARM 30000
#define DD_TIMEOUT_BATTERY_CALIBRATION 60000

#define DD_FORCED_MENU_HOLD_TIME 4000

#define DD_NOTIFICATION_PRESENSE_TIME 2000

// #define DD_SD_PRESENCE_CHECK_TIMEOUT 3000



// RTOS stack sizes per task

#define DD_STACK_SIZE_GAME 8192
#define DD_STACK_SIZE_SYS_EVENT_LISTENER 1024
#define DD_STACK_SIZE_DISPLAY_UPDATE_THREAD 2048
#define DD_STACK_SIZE_BATTERY_CALIBRATION 1024
#define DD_STACK_SIZE_BUZZER 640
#define DD_STACK_SIZE_VIBRO 640



// RTOS priorities

#define DD_TASK_PRIORITY_SYS 1

#define DD_TASK_PRIORITY_SYS_EVENTS 1
#define DD_TASK_PRIORITY_GAME_LOOP 1
#define DD_TASK_PRIORITY_DISP_THREADED 1
#define DD_TASK_PRIORITY_BUZZER 2
#define DD_TASK_PRIORITY_VIBRO 2
#define DD_TASK_PRIORITY_BATTERY_CALIBRATION 1



#define THIS_CORE xPortGetCoreID()
#define DIFFERENT_CORE !xPortGetCoreID()



// ##################################################################################







// ##################################################################################
//                                    UI
// ##################################################################################



// ================= File manager =======================

#define DD_FILE_MANAGER_W 280
#define DD_FILE_MANAGER_H 200
#define DD_GAME_ICON_SIZE 64
#define DD_FILE_MANAGER_MAX_FILE_NAME_LENGTH 30



// ================= Settings window =======================

#define DD_SETTINGS_W 320
#define DD_SETTINGS_H 240



// ================= Dialogue boxes =======================

#define DD_MSGBOX_DEFAULT_ACTION "Ok"



// ##################################################################################


#endif