#ifndef GAMEPAD_API_H
#define GAMEPAD_API_H

#include <Arduino.h>
#include <vector>

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

struct game_config_t{
    String name;
    String description;
    String game_path;
    String icon_path;
    uint8_t minimum_flash;
};

struct system_data_t{
    uint8_t game_path_size;
    char game_path[255];
    uint8_t buzzer_volume;
    uint8_t brightness;
    uint8_t vibro_strength;
    uint8_t battery_levels_n;
    float battery_levels[BATTERY_LEVELS];
    uint16_t battery_lifetime;
};


class Gamepad{
    enum sys_param_t{
        DISPLAY_ENABLED,
        BUTTONS_ENABLED,
        BUZZER_ENABLED,
        VIBRO_ENABLED,
        SD_ENABLED,
        ACCEL_ENABLED,
        GAME_FILES_LOCATED,
        SYSTEM_SETTINGS_TO_DEFAULT,
        GAME_FILES_REQ,
        READY_TO_PLAY
    };
    uint16_t system_params = 0;

    uint8_t brightness = DEFAULT_BRIGHTNESS;
    system_data_t *system_data;
    String game_path;

    TaskHandle_t battery_listener_handler = NULL;
    TaskHandle_t system_data_updater_handler = NULL;
    TaskHandle_t forced_main_menu_handler = NULL;
    TaskHandle_t display_updater_handler = NULL;

    Gamepad_display *disp;
    Gamepad_battery batt;
    Gamepad_SD_card sd_card;

    struct layer_t{
        Gamepad_canvas_t *canvas;
        uint16_t x, y;
    };
    std::vector < layer_t* > layers;

    void (*game_func)();

    bool sys_param(sys_param_t id);
    void sys_param(sys_param_t id, bool val);

    bool init_buttons();
    void init_display();
    bool init_SD();
    bool init_accel();
    bool init_buzzer();
    bool init_vibro();
    void init_battery();

    void init_system_settings();
    void apply_system_settings();
    void locate_game_folder();
    

    void on_charge();
public:
    typedef layer_t* layer_id_t;

    SemaphoreHandle_t semaphore = NULL;

    Gamepad_buttons buttons;
    Gamepad_buzzer buzzer;
    Gamepad_vibro vibro;
    Gamepad_accel accel;
    Gamepad_SD_card game_files;
    
    Gamepad_canvas_t *canvas = nullptr;

    Gamepad() = default;

    void main_loop();

    void init(void (*game_func_)());
    void game_files_required();

    void clear_canvas();
    void update_display();
    void update_display_threaded(float maintain_fps = 0);
    bool update_display_threaded_available();

    void set_display_brightness(uint8_t brightness_);
    uint8_t get_display_brightness();

    Gamepad::layer_id_t create_layer(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, uint8_t color_depth = 8);
    bool layer_exists(layer_id_t id);
    Gamepad_canvas_t* layer(layer_id_t id);
    void move_layer(layer_id_t id, uint16_t new_x, uint16_t new_y);
    void clear_layer(layer_id_t id);
    void delete_layer(layer_id_t id);

    void main_menu();
    void select_game_menu();
    String file_manager();
    void game_downloading_screen(uint8_t percentage);

    uint8_t get_battery_charge();

    game_config_t read_game_config(String &config);


    void save_system_settings();
    void apply_system_settings(system_data_t *settings);

    void give_access_to_subprocess();

};



// ---------- GLOBAL GAMEPAD VARIABLE ------------

extern Gamepad gamepad;

// -----------------------------------------------


#endif