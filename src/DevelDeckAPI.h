#ifndef DEVELDECK_API_H
#define DEVELDECK_API_H

#include <Arduino.h>
#include <vector>

#include "SPI_v3x_compat.h"
#include "config.h"
#include "buttons.h"
#include "display.h"
#include "accel.h"
#include "buzzer.h"
#include "vibro.h"
#include "battery.h"
#include "sd_card.h"
#include "ui.h"
#include "OTA.h"


#ifndef DD_CANVAS_T_DEFINED
typedef DEFAULT_CANVAS_T DD_canvas_t;
#endif

namespace DD_GLOBAL{
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
        DD_canvas_t *canvas;
        uint16_t x, y;
    };

    extern bool forced_display_update;
}

typedef DD_GLOBAL::Layer_t* Layer_id_t;



class DevelDeck{
    enum Sys_flags_t : uint8_t{
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
    uint16_t sys_flags = 0x0000;
    bool sys_flag(Sys_flags_t id);
    void sys_flag(Sys_flags_t id, bool val);

    DD_GLOBAL::System_data_t *system_data;
    String game_path;
    
    DD_display *disp;
    uint8_t brightness = DISP_DEFAULT_BRIGHTNESS;
    
    DD_SD_card sd_card;

    std::vector < DD_GLOBAL::Layer_t* > layers;
    DD_GLOBAL::Layer_t sys_overlay_layer = {nullptr, 0, 0};

    bool init_buttons();
    void init_display();
    bool init_SD();
    bool init_accel();
    bool init_buzzer();
    bool init_vibro();
    void init_battery();
    bool init_SPIFFS();

    void system_data_dump();

    void locate_game();
    void init_system_data();
    void apply_system_settings();
    void user_locate_game_folder();

    void on_charge_mode();

    void __main_menu();
    void __select_game_menu();
    void __settings_menu();
    String __file_manager();


    // ---------- system event listener --------------

    uint32_t last_charge_check = 0;
    uint32_t last_low_charge_alarm = 0;
    float deadband_v = 0;
    bool is_discharged = false;
    bool resume_system = false;
    uint8_t brightness_before_suspension;
    inline void battery_listener_implementation();

    bool menu_pressed = false;
    uint64_t menu_pressed_st;
    inline void forced_main_menu_listener_implementation();

    static void sys_event_listener_task(void *params);

    // --------------------------------

public:
    DD_canvas_t *canvas = nullptr;
    DD_buttons buttons;
    DD_buzzer buzzer;
    DD_vibro vibro;
    DD_accel accel;
    DD_SD_card game_files;

    DevelDeck() = default;

    /**
     * @brief Start main game loop
     * 
     * @param game_func_ override game loop function instead `void loop()` if needed
     */
    void main_loop(void (*game_func_)() = loop);

    void init__();

    

    /**
     * @brief Battery charge check
     * 
     * @return battery level in range from 0 to `BATTERY_LEVELS`
     */
    uint8_t get_charge();



    /**
     * @brief Clears image buffer to black
     * 
     */
    void clear_canvas();

    /**
     * @brief Transfers image buffer to display
     * 
     * @param ignore_layers do not render layers above canvas if true
     * @param x0 update region starting x
     * @param y0 update region starting y
     * @param w update region width (fullsereen if 0)
     * @param h update region height (fullscreen if 0)
     * 
     * @note Function takes a while (~24-29ms at max ESP32 SPI frequency)
     * 
     */
    void update_display(bool ignore_layers = true, int16_t x0 = 0, int16_t y0 = 0, uint16_t w = 0, uint16_t h = 0);

    /**
     * @brief Transfers image buffer to display on different core
     * 
     * @note May be unstable if core2 is busy
     *
     * @param ignore_layers do not render layers above canvas if true
     * @param fps_max update will try to maintain stable fps (if render speed is enough). Ignored if equal 0.
     * @param x0 update region starting x
     * @param y0 update region starting y
     * @param w update region width (fullsereen if 0)
     * @param h update region height (fullscreen if 0)
     */
    void update_display_threaded(bool ignore_layers = true, float fps_max = 0, 
        int16_t x0 = 0, int16_t y0 = 0, uint16_t w = 0, uint16_t h = 0);

    /**
     * @brief Checks if it is possible to perform `DevelDeck::update_display_threaded()`
     * 
     * @return true: means previous update has finished
     * @return false: if previous update is in progress
     */
    bool update_display_threaded_available();


    /**
     * @brief Changes the display backlight brightness
     * 
     * @param brightness_  value in range from 0 to `DISP_BRIGHTNESS_LEVELS`
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
     * @return DD_canvas_t*: pointer to the canvas
     */
    DD_canvas_t* layer(Layer_id_t &id);
    
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
     * @param x0 update region starting x (layer's coordinate basis)
     * @param y0 update region starting y (layer's coordinate basis)
     * @param w update region width
     * @param h update region height
     */
    void update_layer(Layer_id_t &id, int16_t x0 = 0, int16_t y0 = 0, uint16_t w = 0, uint16_t h = 0);

    /**
     * @brief Transfers layer contents **on top** of display image
     * 
     * @note May be unstable if core2 is busy
     *
     * @param id layer id to update
     * @param fps_max update will try to maintain stable fps (if render speed is enough). Ignored if equal 0.
     * @param x0 update region starting x (layer's coordinate basis)
     * @param y0 update region starting y (layer's coordinate basis)
     * @param w update region width (full sprite if 0)
     * @param h update region height (full sprite if 0)
     */
    void update_layer_threaded(Layer_id_t &id, float fps_max = 0,
        int16_t x0 = 0, int16_t y0 = 0, uint16_t w = 0, uint16_t h = 0);

    

    /**
     * @brief Enter DevelDeck main menu function
     * 
     */
    void main_menu();

    /**
     * @brief Enter DevelDeck game selection menu
     * 
     */
    void select_game_menu();

    /**
     * @brief Enter DevelDeck settings menu
     * 
     */
    void settings_menu();

    /**
     * @brief Opens DevelDeck file manager at game source folder as root
     * 
     * @return String: absolute path to file selected by user
     */
    String file_manager();

    

    // ----------- API-only functions ------------

    Layer_id_t create_sys_overlay(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, uint8_t color_depth = 1);
    void delete_sys_overlay();

    void game_downloading_screen(uint8_t percentage);

    void save_system_settings();
    void apply_system_settings(DD_GLOBAL::System_data_t *settings);

    DD_GLOBAL::Game_config_t read_game_config(String &config);
};



// ---------- GLOBAL DEVELDECK VARIABLES -----------

extern DevelDeck ddeck;

extern bool GAME_FILES_REQUIRED;

#define current_file() ddeck.game_files.file_ref()

#define force_sys_disp_update() DD_GLOBAL::forced_display_update = true

// -----------------------------------------------


#endif