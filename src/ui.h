#ifndef GAMEPAD_UI_H
#define GAMEPAD_UI_H

#include <Arduino.h>
#include <vector>
#include <stack>

#include "config.h"

struct Game_config_t;
struct System_data_t;

struct file_mngr_t{
    String dir;
    String file;
    Game_config_t *game_config;
};



class Gamepad_UI{
    TaskHandle_t notification_handler = NULL;
public:
    Gamepad_UI() = default;

    uint8_t main_menu(bool game_active = true, bool game_select_active = true, uint8_t init_cursor = 0);

    file_mngr_t file_manager(bool selecting_game = false, String root = "/");
    file_mngr_t file_manager(String root);

    uint8_t settings(System_data_t &data);

    uint8_t message_box(String msg, std::vector < String > actions = std::vector < String > (), uint16_t w = 0, uint16_t h = 0, int16_t dx = 0, int16_t dy = 0);
    bool notification(String msg);

    void init_game_downloading_screen(Game_config_t game_data, String dir = "");
    void game_downloading_screen(uint8_t percentage);

    void on_charge_screen(bool invert = false);
};



class Gamepad_UI_button{
    void (*render_func)(Gamepad_UI_button *, uint8_t, void *);
public:
    uint16_t x, y, w, h;
    uint16_t id;
    int16_t up_id = -1, down_id = -1, left_id = -1, right_id = -1;

    Gamepad_UI_button(uint16_t id_ = 0, uint16_t x_ = 0, uint16_t y_ = 0, uint16_t w_ = 0, uint16_t h_ = 0);

    void set_neighbours(int16_t up_id_ = -1, int16_t down_id_ = -1, int16_t left_id_ = -1, int16_t right_id_ = -1);

    void assign_render_function(void (*render_func_)(Gamepad_UI_button *, uint8_t, void *));
    void render(void* params = nullptr, uint8_t skin = 0, bool clear = false);
    void render(uint8_t skin, bool clear = false);
};

#endif