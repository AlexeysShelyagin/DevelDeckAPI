#include "ui.h"

#include "gamepadAPI.h"

String file_mngr_trim(String filename, uint16_t max_len){
    if(filename.length() <= max_len)
        return filename;
    
    String res = "";
    for(uint16_t i = 0; i < max_len - 3; i++)
        res += filename[i];
    res += "...";

    return res;
}

void render_menu_button(Gamepad_UI_button *button, uint8_t skin, void* params){
    String *text = (String *) params;

    uint16_t colors[] = {TFT_WHITE, TFT_GREEN, 0x7BEF, 0x7DEF};
    uint16_t color = colors[skin];

    gamepad.canvas -> setTextSize(2);
    gamepad.canvas -> setTextColor(color);
    gamepad.canvas -> drawRect(button -> x, button -> y, button -> w, button -> h, color);
    gamepad.canvas -> drawCentreString(*text, button -> x + button -> w / 2, button -> y + 12, 1);
}

uint8_t Gamepad_UI::main_menu(bool game_active, bool game_select_active, uint8_t init_cursor){
    bool update_disp = true;
    bool quit = false;

    uint8_t cursor = init_cursor % 3;

    uint8_t text_size = gamepad.canvas -> textsize;
    uint32_t text_color = gamepad.canvas -> textcolor;

    uint16_t dims[][4] = {
        {120, 100, 80, 40},
        {220, 100, 80, 40},
        {20, 100, 80, 40}
    };
    String names[] = {"Play", "Optns", "Load"}; 

    Gamepad_UI_button buttons[3];
    for(uint8_t i = 0; i < 3; i++){
        buttons[i] = Gamepad_UI_button(i, dims[i][0], dims[i][1], dims[i][2], dims[i][3]);
        buttons[i].set_neighbours(-1, -1, (i + 2) % 3, (i + 1) % 3);
        buttons[i].assign_render_function(render_menu_button);
    }

    while(!quit){
        while(gamepad.buttons.event_available()){
            uint8_t* event = gamepad.buttons.get_button_event();

            if(event[L_LEFT_BUT_ID] == BUT_PRESSED)
                cursor = buttons[cursor].left_id;
            if(event[L_RIGHT_BUT_ID] == BUT_PRESSED)
                cursor = buttons[cursor].right_id;

            if(event[R_UP_BUT_ID] == BUT_PRESSED)
                quit = true;
            if(event[MENU_BUT_ID] == BUT_PRESSED){
                cursor = 0;
                quit = true;
            }
            
            update_disp = true;
        }

        if(quit)
            break;

        if(update_disp){
            gamepad.clear_canvas();

            for(uint8_t i = 0; i < 3; i++){
                uint8_t skin = 0;
                if(i == 0 && !game_active)
                    skin = 2;
                if(i == 2 && !game_select_active)
                    skin = 2;
                
                buttons[i].render(&names[i], skin + (i == cursor));
            }
            
            gamepad.update_display();
            update_disp = false;
        }

        gamepad.give_access_to_subprocess();
    }

    gamepad.canvas -> setTextSize(text_size);
    gamepad.canvas -> setTextColor(text_color);

    return cursor;
}

file_mngr_t Gamepad_UI::file_manager(bool selecting_game, String root){
    bool update_disp = true;
    bool update_dir = true;
    bool is_game_folder = false;
    bool quit = false;

    uint8_t initial_text_size = gamepad.canvas -> textsize;
    uint32_t initial_color = gamepad.canvas -> textcolor;
    uint8_t initial_font = gamepad.canvas -> getFontID();
    gamepad.canvas -> setFont(0);
    gamepad.canvas -> setTextWrap(false);
    gamepad.canvas -> setTextSize(1);
    gamepad.canvas -> setTextColor(TFT_WHITE);
    uint16_t orig_x = (gamepad.canvas -> width() - FILE_MANAGER_W) / 2;
    uint16_t orig_y = (gamepad.canvas -> height() - FILE_MANAGER_H) / 2;
    gamepad.canvas -> setOrigin(orig_x, orig_y);

    std::vector < file_name_t > dir;
    std::stack < uint16_t > cursor;
    std::stack < uint16_t > scroll;
    cursor.push(0);
    scroll.push(0);
    uint16_t items_n = 0;

    uint16_t max_len = (FILE_MANAGER_W - 4) / gamepad.canvas -> textWidth("a") - 10;
    uint16_t max_items = (FILE_MANAGER_H - 4) / gamepad.canvas -> fontHeight();
    
    Gamepad_SD_card file_manager;
    file_manager.init(root);

    game_config_t *game_config = new game_config_t();

    file_mngr_t res = {"", "", nullptr};
    
    while (!quit){
        while(gamepad.buttons.event_available()){
            uint8_t* buttons = gamepad.buttons.get_button_event();
            
            if(items_n != 0){
                if(buttons[L_DOWN_BUT_ID] == BUT_PRESSED){
                    cursor.top() = (cursor.top() + items_n + 1) % items_n;

                    if(cursor.top() >= scroll.top() + max_items)
                        scroll.top()++;
                    if(cursor.top() == 0)
                        scroll.top() = 0;
                }

                if(buttons[L_UP_BUT_ID] == BUT_PRESSED){
                    cursor.top() = (cursor.top() + items_n - 1) % items_n;

                    if(cursor.top() < scroll.top())
                        scroll.top()--;
                    if(cursor.top() == items_n - 1)
                        scroll.top() = max((int) items_n - max_items, 0);
                }

                if(buttons[L_RIGHT_BUT_ID] == BUT_PRESSED || buttons[R_UP_BUT_ID] == BUT_PRESSED){
                    if(is_game_folder){
                        res = {file_manager.current_dir(), GAME_CONFIG_FILE_NAME, game_config};
                        quit = true;
                    }
                    else{
                        if(dir[cursor.top()].type == IS_FOLDER){
                            file_manager.open_dir(dir[cursor.top()].name);
                            update_dir = true;
                            cursor.push(0);
                            scroll.push(0);
                        }
                        else if(!selecting_game){
                            res = {file_manager.current_dir(), dir[cursor.top()].name, nullptr};
                            quit = true;
                        }
                    }
                }
            }

            if(buttons[L_LEFT_BUT_ID] == BUT_PRESSED || buttons[R_DOWN_BUT_ID] == BUT_PRESSED){
                if(file_manager.open_root_dir()){
                    update_dir = true;
                    cursor.pop();
                    scroll.pop();
                }
                else
                    quit = true;
            }

            if(buttons[MENU_BUT_ID] == BUT_PRESSED){
                if(!selecting_game)
                    res = {file_manager.current_dir(), dir[cursor.top()].name, nullptr};
                quit = true;
            }
            if(buttons[R_UP_BUT_ID] == BUT_PRESSED && !selecting_game && dir[cursor.top()].type == IS_FOLDER){
                res = {file_manager.current_dir(), dir[cursor.top()].name, nullptr};
                quit = true;
            }
            
            update_disp = true;
        }

        if(quit)
            break;


        if(update_dir){
            dir =  file_manager.list_dir();
            items_n = dir.size();

            update_dir = false;
            is_game_folder = false;

            if(selecting_game){
                for(uint16_t i = 0; i < dir.size(); i++){
                    if(dir[i].name == GAME_CONFIG_FILE_NAME){
                        file_manager.open_file(GAME_CONFIG_FILE_NAME);
                        String file = file_manager.file_read_string();
                        *game_config = gamepad.read_game_config(file);
                        file_manager.close_file();
                        
                        is_game_folder = true;
                        break;
                    }
                }
            }
        }
        
        if(update_disp){
            gamepad.canvas -> setOrigin(orig_x, orig_y);
            gamepad.canvas -> fillRect(0, 0, FILE_MANAGER_W, FILE_MANAGER_H, TFT_BLACK);
            gamepad.canvas -> drawRect(0, 0, FILE_MANAGER_W, FILE_MANAGER_H, TFT_WHITE);
            gamepad.canvas -> setOrigin(orig_x + 2, orig_y + 2);
            gamepad.canvas -> setCursor(0, 0);

            if(is_game_folder){
                if(game_config -> icon_path != ""){
                    file_manager.open_file(game_config -> icon_path);
                    gamepad.canvas -> drawPNGFromFile(file_manager.get_file_reference(), 0, 0, true);
                    file_manager.close_file();

                    gamepad.canvas -> setCursor(GAME_ICON_SIZE + 2, 0);
                }

                gamepad.canvas -> setTextSize(2);

                gamepad.canvas -> print(game_config -> name);

                gamepad.canvas -> setCursor(0, GAME_ICON_SIZE + 4);
                gamepad.canvas -> println(game_config -> description);

                if(game_config -> minimum_flash * 1024 * 1024 > ESP.getFlashChipSize()){
                    gamepad.canvas -> setTextSize(1);
                    gamepad.canvas -> setCursor(4, FILE_MANAGER_H - gamepad.canvas -> fontHeight() - 8);
                    gamepad.canvas -> setTextColor(TFT_RED);
                    gamepad.canvas -> print("Unsupported on your device");
                    gamepad.canvas -> setTextColor(TFT_WHITE);
                }

                gamepad.canvas -> setTextSize(1);
            }
            else{
                for(uint16_t i = scroll.top(); i < min(items_n, (uint16_t) (scroll.top() + max_items)); i++){
                    if(i == cursor.top())
                        gamepad.canvas -> print("> ");
                    gamepad.canvas -> print((dir[i].type) ? "FILE:   " : "DIR:    ");
                    gamepad.canvas -> println(file_mngr_trim(dir[i].name, (i == cursor.top()) ? max_len - 2 : max_len));
                }

                if(items_n == 0)
                    gamepad.canvas -> print("< empty >");

                if(items_n > max_items){
                    gamepad.canvas -> setOrigin(orig_x, orig_y);

                    gamepad.canvas -> drawRect(FILE_MANAGER_W - 6, 0, 6, FILE_MANAGER_H, TFT_WHITE);
                    gamepad.canvas -> fillRect(
                        FILE_MANAGER_W - 4, 
                        round(((float) scroll.top() / items_n) * (FILE_MANAGER_H - 6)) + 2,
                        2, 
                        round(((float) max_items / items_n) * (FILE_MANAGER_H - 6) - 0.01) + 2,
                        TFT_WHITE
                    );
                }
            }
            
            update_disp = false; 

            gamepad.update_display();
        }

        gamepad.give_access_to_subprocess();
    }

    if(!is_game_folder)
        delete game_config;
    
    gamepad.canvas -> setFont(initial_font);
    gamepad.canvas -> setTextSize(initial_text_size);
    gamepad.canvas -> setTextColor(initial_color);
    gamepad.canvas -> setOrigin(0, 0);

    return res;
}

file_mngr_t Gamepad_UI::file_manager(String root){
    return file_manager(false, root);
}

void render_setting_param(String value, uint16_t x, uint16_t y, bool active){
    gamepad.canvas -> setCursor(x - gamepad.canvas -> textWidth("< " + value + "> "), y);
    if(active){
        gamepad.canvas -> setTextColor((active) ? TFT_GREEN : TFT_WHITE);
        gamepad.canvas -> print("< " + value + " >");
    }
    else
        gamepad.canvas -> print("  " + value + "  ");
    gamepad.canvas -> setTextColor(TFT_WHITE);
}

uint8_t Gamepad_UI::settings(system_data_t &data){
    bool update_disp = true;
    bool quit = false;
    bool changes = false;

    uint8_t cursor = 0;
    uint8_t scroll = 0;
    uint8_t selected;

    uint8_t initial_text_size = gamepad.canvas -> textsize;
    uint32_t initial_text_color = gamepad.canvas -> textcolor;
    gamepad.canvas -> setTextSize(2);
    gamepad.canvas -> setTextColor(TFT_WHITE);

    String setting_names[] = {"Buzz. vol: ", "Brightness: ", "Vibro: ", "Battery calibration", "Battery lifetime: ", "Factory reset"};
    uint8_t settings_n = sizeof(setting_names) / sizeof(String);
    selected = settings_n;
    uint8_t line_h = round(gamepad.canvas -> fontHeight() * 1.5);
    uint8_t max_items = SETTINGS_H / line_h;

    while(!quit){
        int8_t change = 0;
        while(gamepad.buttons.event_available()){
            uint8_t* buttons = gamepad.buttons.get_button_event();

            if(buttons[L_DOWN_BUT_ID] == BUT_PRESSED){
                if(selected == settings_n){
                    cursor = (cursor + settings_n + 1) % settings_n;

                    if(cursor >= scroll + max_items)
                        scroll++;
                    if(cursor == 0)
                        scroll = 0;
                }
            }

            if(buttons[L_UP_BUT_ID] == BUT_PRESSED){
                if(selected == settings_n){
                    cursor = (cursor + settings_n - 1) % settings_n;

                    if(cursor < scroll)
                        scroll--;
                    if(cursor == settings_n - 1)
                        scroll = max((int) settings_n - max_items, 0);
                }
            }

            if(buttons[L_LEFT_BUT_ID] == BUT_PRESSED){
                if(selected != settings_n)
                    change -= 1;
            }

            if(buttons[L_RIGHT_BUT_ID] == BUT_PRESSED){
                if(selected != settings_n)
                    change += 1;
            }

            if(buttons[R_DOWN_BUT_ID] == BUT_PRESSED || buttons[MENU_BUT_ID] == BUT_PRESSED){
                if(selected == settings_n){
                    if(changes){
                        std::vector < String > optns = {"Save", "Discard"};
                        changes = !message_box("\nSave changes?", optns);
                    }
                    quit = true;
                }
                else
                    selected = settings_n;
            }

            if(buttons[R_UP_BUT_ID] == BUT_PRESSED){
                if(selected == settings_n)
                    selected = cursor;
            }

            update_disp = true;
        }

        if(quit)
            break;
        else;

        if(selected == 3){
            gamepad.clear_canvas();
            gamepad.canvas -> setCursor(0, 0);
            gamepad.canvas -> setTextSize(2);
            gamepad.canvas -> setTextWrap(1);
            gamepad.canvas -> print(BATTERY_CALIBRATION_ALERT);
            gamepad.update_display();

            std::vector < String > optns = {"Cancel", "Calibrate"};
            if(message_box("", optns, 140, 30, 0, 100))
                return 3;
            selected = settings_n;
        }
        else if(selected == 5){
            std::vector < String > optns = {"No", "Yes"};
            if(message_box(FACTORY_RESET_MSG, optns))
                return 2;
            selected = settings_n;
        }
        
        else if(change != 0){
            changes = true;
            switch (selected){
            case 0:
                data.buzzer_volume = max(min(data.buzzer_volume + change, BUZZER_VOLUME_LEVELS), 0);
                break;
            case 1:
                data.brightness = max(min(data.brightness + change, BRIGHTNESS_LEVELS), 1);
                break;
            case 2:
                data.vibro_strength = max(min(data.vibro_strength + change, VIBRO_STRENGTH_LEVELS), 0);
                break;
            }

            gamepad.apply_system_settings(&data);
        }
        
        if(update_disp){
            gamepad.clear_canvas();
            
            for(uint8_t i = scroll; i < min((int) settings_n, scroll + max_items); i++){
                gamepad.canvas -> setCursor(0, (i - scroll) * line_h);
                if(cursor == i)
                    gamepad.canvas -> print("> ");
                
                gamepad.canvas -> print(setting_names[i]);

                uint16_t param_x = SETTINGS_W - 10;
                uint16_t param_y = (i - scroll) * line_h;
                switch (i){
                case 0:
                    render_setting_param((data.buzzer_volume == 0) ? "OFF" : String(data.buzzer_volume), param_x, param_y, (i == selected));
                    break;
                case 1:
                    render_setting_param(String(data.brightness), param_x, param_y, (i == selected));
                    break;
                case 2:
                    render_setting_param((data.vibro_strength == 0) ? "OFF" : String(data.vibro_strength), param_x, param_y, (i == selected));
                    break;
                case 4:
                    String tmp;
                    if(data.battery_lifetime == 0)
                        tmp = "undef";
                    else{
                        uint8_t h = data.battery_lifetime / 60;
                        uint8_t m = data.battery_lifetime % 60;
                        tmp = String(h) + "h" + String(m) + "m";
                    }
                    gamepad.canvas -> setCursor(param_x - gamepad.canvas -> textWidth(tmp), param_y);
                    gamepad.canvas -> print(tmp);
                }

                gamepad.canvas -> println();
            }

            if(settings_n > max_items){
                gamepad.canvas -> drawRect(SETTINGS_W - 6, 0, 6, SETTINGS_H, TFT_WHITE);
                gamepad.canvas -> fillRect(
                    SETTINGS_W - 4, 
                    round(((float) scroll / settings_n) * (SETTINGS_H - 6)) + 2,
                    2, 
                    round(((float) max_items / settings_n) * (SETTINGS_H - 6)) + 2,
                    TFT_WHITE
                );
            }

            gamepad.update_display();
            update_disp = true;
        }

        gamepad.give_access_to_subprocess();
    }

    return changes;
}

struct msgbox_buttons_params_t{
    Gamepad_canvas_t* canvas;
    String text;
};

void render_msgbox_button(Gamepad_UI_button *button, uint8_t skin, void* parameters){
    msgbox_buttons_params_t *params = (msgbox_buttons_params_t *) parameters;

    params -> canvas -> setTextSize(1);
    params -> canvas -> setTextColor((skin) ? TFT_BLACK : TFT_WHITE);
    if(skin)
        params -> canvas -> fillRoundRect(button -> x, button -> y, button -> w, button -> h, 2, TFT_WHITE);
    else
        params -> canvas -> drawRoundRect(button -> x, button -> y, button -> w, button -> h, 2, TFT_WHITE);
    params -> canvas -> drawCentreString(params -> text, button -> x + button -> w / 2, button -> y + 3, 1);
}

uint8_t Gamepad_UI::message_box(String msg, std::vector < String > actions, uint16_t w, uint16_t h, int16_t dx, int16_t dy){
    bool quit = false;
    bool update_disp = true;
    uint8_t cursor = 0;
    
    if(w == 0) w = 200;
    if(h == 0) h = 100;
    Gamepad::layer_id_t layer_id = gamepad.create_layer(w, h, dx + (DISP_WIDTH - w) / 2, dy + (DISP_HEIGHT - h) / 2, 1);
    if(!gamepad.layer_exists(layer_id)){
        Serial.println("ERROR: unable to create message box");
        return 0;
    }

    std::vector < String > msg_lines(1, "");
    for(uint16_t i = 0; i < msg.length(); i++){
        if(msg[i] == '\n'){
            msg_lines.push_back("");
            i++;
        }
        msg_lines[msg_lines.size() - 1] += msg[i];
    }
    
    if(actions.size() == 0)
        actions.push_back(MSG_BOX_DEFAULT_ACTION);
    
    uint8_t buttons_n = actions.size();
    uint16_t indent = round((float) w / buttons_n);
    uint16_t buttons_h = gamepad.layer(layer_id) -> fontHeight() + 4;

    Gamepad_UI_button buttons[buttons_n];
    for(uint8_t i = 0; i < buttons_n; i++){
        uint16_t text_w = gamepad.layer(layer_id) -> textWidth(actions[i]);
        buttons[i] = Gamepad_UI_button(i, indent * (0.5 + i) - (text_w / 2 + 3), h - buttons_h - 8, text_w + 6, buttons_h);
        buttons[i].set_neighbours(-1, -1, (i + buttons_n - 1) % buttons_n, (i + buttons_n + 1) % buttons_n);
        buttons[i].assign_render_function(render_msgbox_button);
    }

    gamepad.layer(layer_id) -> setTextColor(TFT_WHITE);
    gamepad.layer(layer_id) -> setTextSize(2);
    uint16_t font_h = gamepad.layer(layer_id) -> fontHeight();
    
    while(!quit){
        while(gamepad.buttons.event_available()){
            uint8_t* event = gamepad.buttons.get_button_event();
            
            if(event[L_LEFT_BUT_ID] == BUT_PRESSED){
                cursor = buttons[cursor].left_id;
                update_disp = true;
            }
            if(event[L_RIGHT_BUT_ID] == BUT_PRESSED){
                cursor = buttons[cursor].right_id;
                update_disp = true;
            }

            if(event[R_UP_BUT_ID] == BUT_PRESSED)
                quit = true;
            
        }

        if(quit)
            break;
        
        if(update_disp){
            gamepad.clear_layer(layer_id);

            gamepad.layer(layer_id) -> drawRect(0, 0, w, h, TFT_WHITE);

            gamepad.layer(layer_id) -> setTextSize(2);
            gamepad.layer(layer_id) -> setTextColor(TFT_WHITE);
            for(uint8_t i = 0; i < msg_lines.size(); i++)
                gamepad.layer(layer_id) -> drawCentreString(msg_lines[i], w / 2, 5 + i * font_h, 1);
            
            for(uint8_t i = 0; i < buttons_n; i++){
                msgbox_buttons_params_t but_params = {gamepad.layer(layer_id), actions[i]};
                buttons[i].render(&but_params, (i == cursor), true);
            }

            gamepad.update_display();
            update_disp = false;
        }

        gamepad.give_access_to_subprocess();
    }

    gamepad.delete_layer(layer_id);
    gamepad.update_display();

    return cursor;
}

void notification_destructor(void* params){
    Gamepad::layer_id_t *layer_id = (Gamepad::layer_id_t *) params;

    vTaskDelay(pdTICKS_TO_MS(NOTIFICATION_HOLD_TIME));

    while(!xSemaphoreTake(gamepad.semaphore, portMAX_DELAY))
        vTaskDelay(pdMS_TO_TICKS(1));
    
    gamepad.delete_layer(*layer_id);
    gamepad.update_display();

    xSemaphoreGive(gamepad.semaphore);
        
    delete layer_id;
    vTaskDelete(NULL);
}

bool Gamepad_UI::notification(String msg){
    if(notification_handler != NULL && eTaskGetState(notification_handler) == eBlocked){
        Serial.println("ERROR: unable to create notification");
        return 0;
    }

    Gamepad::layer_id_t layer_id = gamepad.create_layer(200, 100, 60, 70, 1);
    if(!gamepad.layer_exists(layer_id)){
        Serial.println("ERROR: unable to create message box");
        return 0;
    }

    std::vector < String > msg_lines(1, "");
    for(uint16_t i = 0; i < msg.length(); i++){
        if(msg[i] == '\n'){
            msg_lines.push_back("");
            i++;
        }
        msg_lines[msg_lines.size() - 1] += msg[i];
    }

    gamepad.layer(layer_id) -> setTextColor(TFT_WHITE);
    gamepad.layer(layer_id) -> setTextSize(2);
    uint16_t font_h = gamepad.layer(layer_id) -> fontHeight();

    gamepad.layer(layer_id) -> drawRect(0, 0, 200, 100, TFT_WHITE);
    gamepad.layer(layer_id) -> setTextSize(2);
    gamepad.layer(layer_id) -> setTextColor(TFT_WHITE);
    for(uint8_t i = 0; i < msg_lines.size(); i++)
        gamepad.layer(layer_id) -> drawCentreString(msg_lines[i], 100, 5 + i * font_h, 1);
    
    gamepad.update_display();

    Gamepad::layer_id_t *layer_id_param = new Gamepad::layer_id_t;
    *layer_id_param = layer_id;
    xTaskCreatePinnedToCore(
        notification_destructor,
        "notif",
        NOTIFICATION_DESTRUCTOR_STACK_SIZE,
        layer_id_param,
        1,
        &notification_handler,
        xPortGetCoreID()
    );

    return 1;
}

void Gamepad_UI::init_game_downloading_screen(game_config_t game_data, String dir){
    gamepad.clear_canvas();

    gamepad.canvas -> setTextSize(2);
    gamepad.canvas -> setTextColor(TFT_WHITE);

    uint16_t cursor_x = (gamepad.canvas -> width() - gamepad.canvas -> textWidth(game_data.name)) / 2;
    uint16_t y0 = 20;
    gamepad.canvas -> setCursor(cursor_x, y0);
    gamepad.canvas -> print(game_data.name);

    gamepad.canvas -> drawRect(10, gamepad.canvas -> fontHeight() + GAME_ICON_SIZE + y0 + 8, gamepad.canvas -> width() - 20, 8, TFT_WHITE);

    if(game_data.icon_path != ""){
        Gamepad_SD_card temp_fs;
        temp_fs.init(dir);

        temp_fs.open_file(game_data.icon_path);
        gamepad.canvas -> drawPNGFromFile(
            temp_fs.get_file_reference(), 
            (gamepad.canvas -> width() - GAME_ICON_SIZE) / 2, 
            gamepad.canvas -> fontHeight() + y0 + 4, 
            true
        );
        temp_fs.close_file();

        gamepad.canvas -> setCursor(GAME_ICON_SIZE + 2, 0);
    }

    gamepad.update_display();
}

void Gamepad_UI::game_downloading_screen(uint8_t percentage){
    gamepad.canvas -> setTextSize(2);

    uint16_t width = round((float) percentage / 100.0 * (gamepad.canvas -> width() - 24));
    uint16_t start_h = gamepad.canvas -> fontHeight() + GAME_ICON_SIZE + 30;

    gamepad.canvas -> fillRect(12, start_h, gamepad.canvas -> width() - 24, 4, TFT_BLACK);
    gamepad.canvas -> fillRect(12, start_h, width, 4, TFT_WHITE);

    gamepad.canvas -> fillRect(50, start_h + gamepad.canvas -> fontHeight(), gamepad.canvas -> width() - 100, gamepad.canvas -> fontHeight(), TFT_BLACK);
    String percentage_str = String(percentage) + "%";
    uint16_t cursor_x = (gamepad.canvas -> width() - gamepad.canvas -> textWidth(percentage_str)) / 2;
    gamepad.canvas -> setCursor(cursor_x, start_h + gamepad.canvas -> fontHeight());
    gamepad.canvas -> print(percentage_str);

    gamepad.update_display();
}

void Gamepad_UI::on_charge_screen(bool invert){
    uint8_t initial_font_size = gamepad.canvas -> textsize;
    uint32_t initial_text_color = gamepad.canvas -> textcolor;

    gamepad.canvas -> fillSprite((invert) ? TFT_WHITE : TFT_BLACK);

    gamepad.canvas -> drawRect(100, 85, 120, 70, (invert) ? TFT_BLACK : TFT_WHITE);
    gamepad.canvas -> setTextSize(2);
    gamepad.canvas -> setTextColor((invert) ? TFT_BLACK : TFT_WHITE);
    gamepad.canvas -> drawCentreString("CHARGING", 160, 120 - gamepad.canvas -> fontHeight() / 2, 1);
    
    gamepad.update_display();

    gamepad.canvas -> setTextSize(initial_font_size);
    gamepad.canvas -> setTextColor(initial_text_color);
}



void empty_button_render(Gamepad_UI_button *button, uint8_t skin, void *params){}

Gamepad_UI_button::Gamepad_UI_button(uint16_t id_, uint16_t x_, uint16_t y_, uint16_t w_, uint16_t h_){
    id = id_;
    x = x_;
    y = y_;
    w = w_;
    h = h_;

    render_func = empty_button_render;
}

void Gamepad_UI_button::set_neighbours(int16_t up_id_, int16_t down_id_, int16_t left_id_, int16_t right_id_){
    up_id = up_id_;
    down_id = down_id_;
    left_id = left_id_;
    right_id = right_id_;
}

void Gamepad_UI_button::assign_render_function(void (*render_func_)(Gamepad_UI_button *, uint8_t, void *)){
    render_func = render_func_;
}

void Gamepad_UI_button::render(void* params, uint8_t skin, bool clear){
    if(clear)
        gamepad.canvas -> fillRect(x, y, w, h, TFT_BLACK);
    
    render_func(this, skin, params);
}

void Gamepad_UI_button::render(uint8_t skin, bool clear){
    render(nullptr, skin, clear);
}
