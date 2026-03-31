#ifndef DEVEL_DECK_API_H
#define DEVEL_DECK_API_H

#include <Arduino.h>
#include <vector>


#include "SPI_v3x_compat.h"
#include "config.h"
#include "buttons.h"
#include "display.h"
#include "accel.h"
#include "tacktile_response.h"
#include "battery.h"
#include "sd_card.h"
#include "ui.h"
#include "OTA.h"


#ifndef GAMEPAD_CANVAS_T_DEFINED
typedef DEFAULT_CANVAS_T Gamepad_canvas_t;
#endif


/**
 * @brief Game info for loading screen container
 * 
 */
struct Game_config_t{
    String name;                /** Game name */
    String description;         /** Game description text */
    String game_path;           /** Path to `game.ini` */
    String icon_path;           /** Path to game icon */
    uint8_t minimum_flash;      /** Minimum required flash for installation */
};

struct System_data_t{
    uint8_t game_path_size;
    char game_path[255];
    uint8_t buzzer_volume;
    uint8_t brightness;
    uint8_t vibro_strength;

    float battery_critical_v;
    float battery_charging_v;
    float battery_only_charging_v;
    float battery_full_v;
    
    uint8_t battery_levels_n;
    float battery_levels[BATTERY_LEVELS];
    uint16_t battery_lifetime;
};

struct Layer_t{
    Gamepad_canvas_t *canvas;
    uint16_t x, y;
};

typedef Layer_t* Layer_id_t;



class Gamepad{
    enum Sys_param_t{
        INITIALIZED,
        DISPLAY_ENABLED,
        BUTTONS_ENABLED,
        BUZZER_ENABLED,
        VIBRO_ENABLED,
        SD_ENABLED,
        SPIFFS_ENABLED,
        ACCEL_ENABLED,
        GAME_FILES_LOCATED,
        SYSTEM_SETTINGS_TO_DEFAULT,
        READY_TO_PLAY
    };
    uint16_t system_params = 0;

    uint8_t brightness = DEFAULT_BRIGHTNESS;
    System_data_t *system_data;
    String game_path;

    TaskHandle_t battery_listener_handler = NULL;
    TaskHandle_t system_data_updater_handler = NULL;
    TaskHandle_t forced_main_menu_handler = NULL;
    TaskHandle_t display_updater_handler = NULL;

    Gamepad_display *disp;
    Gamepad_battery batt;
    Gamepad_SD_card sd_card;

    std::vector < Layer_t* > layers;

    void (*game_func)();

    bool init_buttons();
    void init_display();
    bool init_SD();
    bool init_accel();
    bool init_buzzer();
    bool init_vibro();
    void init_battery();
    bool init_SPIFFS();

    bool sys_param(Sys_param_t id);
    void sys_param(Sys_param_t id, bool val);
    void system_data_dump();

    void locate_game();
    void init_system_data();
    void apply_system_settings();
    void user_locate_game_folder();

    void on_charge_screen();

public:

    SemaphoreHandle_t semaphore = NULL;

    Gamepad_canvas_t *canvas = nullptr;
    Gamepad_buttons buttons;
    Gamepad_buzzer buzzer;
    Gamepad_vibro vibro;
    Gamepad_accel accel;
    Gamepad_SD_card game_files;

    Gamepad() = default;

    /**
     * @brief Start main game loop
     * 
     * @param game_func_ override game loop function instead `void loop()` if needed
     */
    void main_loop(void (*game_func_)() = loop);

    /**
     * @brief Some system level subprocesses are blocked during `game_func()` handling. Call this function to avoid long-term system delay.
     * 
     */
    void give_access_to_subprocess();

    void init__();

    

    /**
     * @brief Battery charge check
     * 
     * @return battery level in range from 0 to `BATTERY_LEVELS`
     */
    uint8_t get_battery_charge();



    /**
     * @brief Clears image buffer to black
     * 
     */
    void clear_canvas();

    /**
     * @brief Transfers image buffer to display
     * 
     * @param ignore_layers do not render layers above canvas if true
     * 
     * @note Function takes a while (~37ms at max ESP32 SPI frequency)
     * 
     */
    void update_display(bool ignore_layers = false);

    /**
     * @brief Transfers image buffer to display on different core
     * 
     * @note May be unstable if core2 is busy
     *
     * @param ignore_layers do not render layers above canvas if true
     * @param fps_max update will try to maintain stable fps (if render speed is enough)
     */
    void update_display_threaded(bool ignore_layers = false, float fps_max = 0);

    /**
     * @brief Checks if it is possible to perform `Gamepad::update_display_threaded()`
     * 
     * @return true: means previous update has finished
     * @return false: if previous update is in progress
     */
    bool update_display_threaded_available();


    /**
     * @brief Changes the display backlight brightness
     * 
     * @param brightness_  value in range from 0 to `BRIGHTNESS_LEVELS`
     */
    void set_display_brightness(uint8_t brightness_);

    /**
     * @brief returns current display brigtness
     * 
     * @return uint8_t
     */
    uint8_t get_display_brightness();



    /**
     * @brief Creates a layer which will be rendered above the main canvas
     * 
     * @note Creation of the layer creates new image buffer with a size of `W * H * color_depth / 8` bytes. 
     * If there is not enough memory layer won't be created
     * 
     * @note Layers arranged by their creation order
     * 
     * @note Layers are rendered directly from memory separately, which can cause flickering for frequent updates
     * 
     * @param width 
     * @param height 
     * @param x 
     * @param y 
     * @param color_depth  1 | 4 | 8 | 16 bits
     * 
     * @return Layer_id_t: pointer to the layer
     * @return nullptr: if layer creation failed
     */
    Layer_id_t create_layer(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, uint8_t color_depth = 8);

    /**
     * @brief Checks if layer exists
     * 
     * @param id layer pointer
     * 
     * @return true 
     * @return false 
     */
    bool layer_exists(Layer_id_t &id);

    /**
     * @brief Access layer canvas
     * 
     * @param id layer pointer
     * 
     * @return Gamepad_canvas_t*: pointer to the canvas
     */
    Gamepad_canvas_t* layer(Layer_id_t &id);
    
    /**
     * @brief Fills layer black
     * 
     * @param id layer pointer
     */
    void clear_layer(Layer_id_t &id);
    
    /**
     * @brief Deletes layer with its canvas
     * 
     * @param id layer pointer
     */
    void delete_layer(Layer_id_t &id);

    /**
     * @brief Changes layer position on display
     * 
     * @param id layer pointer
     * @param new_x 
     * @param new_y 
     */
    void move_layer(Layer_id_t &id, uint16_t new_x, uint16_t new_y);

    /**
     * @brief Update specific layer on display
     * 
     * @param id layer pointer
     */
    void update_layer(Layer_id_t &id);

    /**
     * @brief Transfers layer contents **on top** of display image
     * 
     * @note May be unstable if core2 is busy
     *
     * @param id layer id to update
     * @param fps_max update will try to maintain stable fps (if render speed is enough)
     */
    void update_layer_threaded(Layer_id_t &id, float fps_max = 0);

    

    /**
     * @brief Enter gamepad main menu function
     * 
     */
    void main_menu();

    /**
     * @brief Enter gamepad game selection menu
     * 
     */
    void select_game_menu();

    /**
     * @brief Enter gamepad settings menu
     * 
     */
    void settings_menu();

    /**
     * @brief Opens gamepad file manager at game source folder as root
     * 
     * @return String: absolute path to file selected by user
     */
    String file_manager();

    

    // ----------- API-only functions ------------

    void game_downloading_screen(uint8_t percentage);

    void save_system_settings();
    void apply_system_settings(System_data_t *settings);

    Game_config_t read_game_config(String &config);
};



// ---------- GLOBAL GAMEPAD VARIABLES -----------

extern Gamepad gamepad;

extern bool GAME_FILES_REQUIRED;

// -----------------------------------------------


#endif