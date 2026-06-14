#include "ui.h"

#include "DevelDeckAPI.h"

extern TaskHandle_t sys_task_handler;



String File_mngr_trim(String filename, uint16_t max_len){
    if(filename.length() <= max_len)
        return filename;
    
    String res = "";
    for(uint16_t i = 0; i < max_len - 3; i++)
        res += filename[i];
    res += "...";

    return res;
}



void render_menu_button(DD_UI_button *button, uint8_t skin, void* params){
    String *text = (String *) params;

    uint16_t colors[] = {TFT_WHITE, TFT_GREEN, 0x7BEF, 0x7DEF};
    uint16_t color = colors[skin];

    ddeck.canvas->setTextSize(2);
    ddeck.canvas->setTextColor(color);
    ddeck.canvas->drawRect(button->x, button->y, button->w, button->h, color);
    ddeck.canvas->drawCentreString(*text, button->x + button->w / 2, button->y + 12, 1);
}

uint8_t DD_UI::main_menu(bool game_active, bool game_select_active, uint8_t init_cursor){
    bool update_disp = true;
    bool quit = false;

    Graphics_params_t init_graphics = ddeck.canvas->graphicsParams();
    ddeck.canvas->setDefaultGraphicsParams();

    uint8_t cursor = init_cursor % 3;

    uint16_t dims[][4] = {
        {120, 100, 80, 40},
        {220, 100, 80, 40},
        {20, 100, 80, 40}
    };
    String names[] = {"Play", "Optns", "Load"}; 

    DD_UI_button buttons[3];
    for(uint8_t i = 0; i < 3; i++){
        buttons[i] = DD_UI_button(i, dims[i][0], dims[i][1], dims[i][2], dims[i][3]);
        buttons[i].set_neighbours(-1, -1, (i + 2) % 3, (i + 1) % 3);
        buttons[i].assign_render_function(render_menu_button);
    }

    while(!quit){
        while(ddeck.buttons.event_available()){
            uint8_t* event = ddeck.buttons.get_event();

            if(event[LEFT_BUT_ID] == BUT_PRESSED)
                cursor = buttons[cursor].left_id;
            if(event[RIGHT_BUT_ID] == BUT_PRESSED)
                cursor = buttons[cursor].right_id;

            if(event[A_BUT_ID] == BUT_PRESSED)
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
            ddeck.clear_canvas();

            for(uint8_t i = 0; i < 3; i++){
                uint8_t skin = 0;
                if(i == 0 && !game_active)
                    skin = 2;
                if(i == 2 && !game_select_active)
                    skin = 2;
                
                buttons[i].render(&names[i], skin + (i == cursor));
            }
            
            ddeck.update_display();
            update_disp = false;
        }

        vTaskDelay(1);
    }

    ddeck.canvas->setGraphicsParams(init_graphics);

    return cursor;
}

File_mngr_t DD_UI::file_manager(bool selecting_game, String root){
    bool update_disp = true;
    bool update_dir = true;
    bool is_game_folder = false;
    bool quit = false;

    Graphics_params_t init_graphics = ddeck.canvas->graphicsParams();
    ddeck.canvas->setDefaultGraphicsParams();
    ddeck.canvas->setFont(0);
    ddeck.canvas->setTextWrap(false);
    ddeck.canvas->setTextSize(1);
    ddeck.canvas->setTextColor(TFT_WHITE);

    uint16_t orig_x = (ddeck.canvas->width() - FILE_MANAGER_W) / 2;
    uint16_t orig_y = (ddeck.canvas->height() - FILE_MANAGER_H) / 2;
    ddeck.canvas->setOrigin(orig_x, orig_y);

    std::vector < File_name_t > dir;
    std::stack < uint16_t > cursor;
    std::stack < uint16_t > scroll;
    cursor.push(0);
    scroll.push(0);
    uint16_t items_n = 0;

    uint16_t max_len = (FILE_MANAGER_W - 4) / ddeck.canvas->textWidth("a") - 10;
    uint16_t max_items = (FILE_MANAGER_H - 4) / ddeck.canvas->fontHeight();
    
    DD_SD_card file_manager;
    file_manager.init(root);

    Game_config_t *game_config = new Game_config_t();

    File_mngr_t res = {"", "", nullptr};
    
    while (!quit){
        while(ddeck.buttons.event_available()){
            uint8_t* buttons = ddeck.buttons.get_event();
            
            if(items_n != 0){
                if(buttons[DOWN_BUT_ID] == BUT_PRESSED){
                    cursor.top() = (cursor.top() + items_n + 1) % items_n;

                    if(cursor.top() >= scroll.top() + max_items)
                        scroll.top()++;
                    if(cursor.top() == 0)
                        scroll.top() = 0;
                }

                if(buttons[UP_BUT_ID] == BUT_PRESSED){
                    cursor.top() = (cursor.top() + items_n - 1) % items_n;

                    if(cursor.top() < scroll.top())
                        scroll.top()--;
                    if(cursor.top() == items_n - 1)
                        scroll.top() = max((int) items_n - max_items, 0);
                }

                if(buttons[RIGHT_BUT_ID] == BUT_PRESSED || buttons[A_BUT_ID] == BUT_PRESSED){
                    if(is_game_folder){
                        res = {file_manager.current_dir(), GAME_CONFIG_FILE_NAME, game_config};
                        quit = true;
                    }
                    else{
                        if(dir[cursor.top()].type == IS_DIR){
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

            if(buttons[LEFT_BUT_ID] == BUT_PRESSED || buttons[B_BUT_ID] == BUT_PRESSED){
                if(file_manager.open_parent_dir()){
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
            if(buttons[A_BUT_ID] == BUT_PRESSED && !selecting_game && dir[cursor.top()].type == IS_DIR){
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
                        *game_config = ddeck.read_game_config(file);
                        file_manager.close_file();
                        
                        is_game_folder = true;
                        break;
                    }
                }
            }
        }
        
        if(update_disp){
            ddeck.canvas->setOrigin(orig_x, orig_y);
            ddeck.canvas->fillRect(0, 0, FILE_MANAGER_W, FILE_MANAGER_H, TFT_BLACK);
            ddeck.canvas->drawRect(0, 0, FILE_MANAGER_W, FILE_MANAGER_H, TFT_WHITE);
            ddeck.canvas->setOrigin(orig_x + 2, orig_y + 2);
            ddeck.canvas->setCursor(0, 0);

            if(is_game_folder){
                if(game_config->icon_path != ""){
                    file_manager.open_file(game_config->icon_path);
                    ddeck.canvas->drawPNGFromFile(file_manager.get_file_reference(), 0, 0, true);
                    file_manager.close_file();

                    ddeck.canvas->setCursor(GAME_ICON_SIZE + 2, 0);
                }

                ddeck.canvas->setTextSize(2);

                ddeck.canvas->print(game_config->name);

                ddeck.canvas->setCursor(0, GAME_ICON_SIZE + 4);
                ddeck.canvas->println(game_config->description);

                if(game_config->minimum_flash * 1024 * 1024 > ESP.getFlashChipSize()){
                    ddeck.canvas->setTextSize(1);
                    ddeck.canvas->setCursor(4, FILE_MANAGER_H - ddeck.canvas->fontHeight() - 8);
                    ddeck.canvas->setTextColor(TFT_RED);
                    ddeck.canvas->print(TXT_USUPPORTED_ON_DEVICE);
                    ddeck.canvas->setTextColor(TFT_WHITE);
                }

                ddeck.canvas->setTextSize(1);
            }
            else{
                for(uint16_t i = scroll.top(); i < min(items_n, (uint16_t) (scroll.top() + max_items)); i++){
                    if(i == cursor.top())
                        ddeck.canvas->print("> ");
                    ddeck.canvas->print((dir[i].type) ? "FILE:   " : "DIR:    ");
                    ddeck.canvas->println(File_mngr_trim(dir[i].name, (i == cursor.top()) ? max_len - 2 : max_len));
                }

                if(items_n == 0)
                    ddeck.canvas->print("< empty >");

                if(items_n > max_items){
                    ddeck.canvas->setOrigin(orig_x, orig_y);

                    ddeck.canvas->drawRect(FILE_MANAGER_W - 6, 0, 6, FILE_MANAGER_H, TFT_WHITE);
                    ddeck.canvas->fillRect(
                        FILE_MANAGER_W - 4, 
                        round(((float) scroll.top() / items_n) * (FILE_MANAGER_H - 6)) + 2,
                        2, 
                        round(((float) max_items / items_n) * (FILE_MANAGER_H - 6) - 0.01) + 2,
                        TFT_WHITE
                    );
                }
            }
            
            update_disp = false; 

            ddeck.update_display();
        }

        vTaskDelay(1);
    }

    if(!is_game_folder)
        delete game_config;
    
    ddeck.canvas->setGraphicsParams(init_graphics);

    return res;
}

File_mngr_t DD_UI::file_manager(String root){
    return file_manager(false, root);
}

void render_setting_param(String value, uint16_t x, uint16_t y, bool active){
    ddeck.canvas->setCursor(x - ddeck.canvas->textWidth("< " + value + "> "), y);
    if(active){
        ddeck.canvas->setTextColor((active) ? TFT_GREEN : TFT_WHITE);
        ddeck.canvas->print("< " + value + " >");
    }
    else
        ddeck.canvas->print("  " + value + "  ");
    ddeck.canvas->setTextColor(TFT_WHITE);
}

uint8_t DD_UI::settings(System_data_t &data){
    bool update_disp = true;
    bool quit = false;
    bool changes = false;

    uint8_t cursor = 0;
    uint8_t scroll = 0;
    uint8_t selected;

    Graphics_params_t init_graphics = ddeck.canvas->graphicsParams();
    ddeck.canvas->setDefaultGraphicsParams();
    ddeck.canvas->setTextSize(2);
    ddeck.canvas->setTextColor(TFT_WHITE);

    String setting_names[] = {TXT_BUZZ_VOL, TXT_BRIGHTNESS, TXT_VIBRO, TXT_BATT_CALIBR, TXT_BATT_LIFETIME, TXT_FACTORY_RESET};
    uint8_t settings_n = sizeof(setting_names) / sizeof(String);
    selected = settings_n;
    uint8_t line_h = round(ddeck.canvas->fontHeight() * 1.5);
    uint8_t max_items = SETTINGS_H / line_h;

    while(!quit){
        int8_t change = 0;
        while(ddeck.buttons.event_available()){
            uint8_t* buttons = ddeck.buttons.get_event();

            if(buttons[DOWN_BUT_ID] == BUT_PRESSED){
                if(selected == settings_n){
                    cursor = (cursor + settings_n + 1) % settings_n;

                    if(cursor >= scroll + max_items)
                        scroll++;
                    if(cursor == 0)
                        scroll = 0;
                }
            }

            if(buttons[UP_BUT_ID] == BUT_PRESSED){
                if(selected == settings_n){
                    cursor = (cursor + settings_n - 1) % settings_n;

                    if(cursor < scroll)
                        scroll--;
                    if(cursor == settings_n - 1)
                        scroll = max((int) settings_n - max_items, 0);
                }
            }

            if(buttons[LEFT_BUT_ID] == BUT_PRESSED){
                if(selected != settings_n)
                    change -= 1;
            }

            if(buttons[RIGHT_BUT_ID] == BUT_PRESSED){
                if(selected != settings_n)
                    change += 1;
            }

            if(buttons[B_BUT_ID] == BUT_PRESSED || buttons[MENU_BUT_ID] == BUT_PRESSED){
                if(selected == settings_n){
                    if(changes){
                        std::vector < String > optns = {"Save", "Discard"};
                        changes = !message_box(TXT_SAVE_MSG, optns);
                    }
                    quit = true;
                }
                else
                    selected = settings_n;
            }

            if(buttons[A_BUT_ID] == BUT_PRESSED){
                if(selected == settings_n)
                    selected = cursor;
            }

            update_disp = true;
        }

        if(quit)
            break;
        else;

        if(selected == 3){
            ddeck.clear_canvas();
            ddeck.canvas->setCursor(0, 0);
            ddeck.canvas->setTextSize(2);
            ddeck.canvas->setTextWrap(1);
            ddeck.canvas->print(BATTERY_CALIBRATION_ALERT);
            ddeck.update_display();

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

            ddeck.apply_system_settings(&data);
        }
        
        if(update_disp){
            ddeck.clear_canvas();
            
            for(uint8_t i = scroll; i < min((int) settings_n, scroll + max_items); i++){
                ddeck.canvas->setCursor(0, (i - scroll) * line_h);
                if(cursor == i)
                    ddeck.canvas->print("> ");
                
                ddeck.canvas->print(setting_names[i]);

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
                    ddeck.canvas->setCursor(param_x - ddeck.canvas->textWidth(tmp), param_y);
                    ddeck.canvas->print(tmp);
                }

                ddeck.canvas->println();
            }

            if(settings_n > max_items){
                ddeck.canvas->drawRect(SETTINGS_W - 6, 0, 6, SETTINGS_H, TFT_WHITE);
                ddeck.canvas->fillRect(
                    SETTINGS_W - 4, 
                    round(((float) scroll / settings_n) * (SETTINGS_H - 6)) + 2,
                    2, 
                    round(((float) max_items / settings_n) * (SETTINGS_H - 6)) + 2,
                    TFT_WHITE
                );
            }

            ddeck.update_display();
            update_disp = true;
        }

        vTaskDelay(1);
    }

    ddeck.canvas->setGraphicsParams(init_graphics);

    return changes;
}

struct msgbox_buttons_params_t{
    DD_canvas_t* canvas;
    String text;
};

void render_msgbox_button(DD_UI_button *button, uint8_t skin, void* parameters){
    msgbox_buttons_params_t *params = (msgbox_buttons_params_t *) parameters;

    params->canvas->setTextSize(1);
    params->canvas->setTextColor((skin) ? TFT_BLACK : TFT_WHITE);
    if(skin)
        params->canvas->fillRoundRect(button->x, button->y, button->w, button->h, 2, TFT_WHITE);
    else
        params->canvas->drawRoundRect(button->x, button->y, button->w, button->h, 2, TFT_WHITE);
    params->canvas->drawCentreString(params->text, button->x + button->w / 2, button->y + 3, 1);
}

uint8_t DD_UI::message_box(String msg, std::vector < String > actions, uint16_t w, uint16_t h, int16_t dx, int16_t dy){
    bool quit = false;
    bool update_disp = true;
    uint8_t cursor = 0;
    
    if(w == 0) w = 200;
    if(h == 0) h = 100;
    Layer_id_t msg_layer = ddeck.create_sys_overlay(w, h, dx + (DISP_WIDTH - w) / 2, dy + (DISP_HEIGHT - h) / 2, 1);
    if(msg_layer == nullptr){
        Serial.println(TXT_UNABLE_CREATE_MSGBOX);
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
    uint16_t buttons_h = msg_layer->canvas->fontHeight() + 4;

    DD_UI_button buttons[buttons_n];
    for(uint8_t i = 0; i < buttons_n; i++){
        uint16_t text_w = msg_layer->canvas->textWidth(actions[i]);
        buttons[i] = DD_UI_button(i, indent * (0.5 + i) - (text_w / 2 + 3), h - buttons_h - 8, text_w + 6, buttons_h);
        buttons[i].set_neighbours(-1, -1, (i + buttons_n - 1) % buttons_n, (i + buttons_n + 1) % buttons_n);
        buttons[i].assign_render_function(render_msgbox_button);
    }

    msg_layer->canvas->setTextColor(TFT_WHITE);
    msg_layer->canvas->setTextSize(2);
    uint16_t font_h = msg_layer->canvas->fontHeight();
    
    while(!quit){
        while(ddeck.buttons.event_available()){
            uint8_t* event = ddeck.buttons.get_event();
            
            if(event[LEFT_BUT_ID] == BUT_PRESSED){
                cursor = buttons[cursor].left_id;
                update_disp = true;
            }
            if(event[RIGHT_BUT_ID] == BUT_PRESSED){
                cursor = buttons[cursor].right_id;
                update_disp = true;
            }

            if(event[A_BUT_ID] == BUT_PRESSED)
                quit = true;
            
        }

        if(quit)
            break;
        
        if(update_disp){
            ddeck.clear_layer(msg_layer);

            msg_layer->canvas->drawRect(0, 0, w, h, TFT_WHITE);

            msg_layer->canvas->setTextSize(2);
            msg_layer->canvas->setTextColor(TFT_WHITE);
            for(uint8_t i = 0; i < msg_lines.size(); i++)
                msg_layer->canvas->drawCentreString(msg_lines[i], w / 2, 5 + i * font_h, 1);
            
            for(uint8_t i = 0; i < buttons_n; i++){
                msgbox_buttons_params_t but_params = {msg_layer->canvas, actions[i]};
                buttons[i].render(&but_params, (i == cursor), true);
            }

            ddeck.update_layer(msg_layer);
            update_disp = false;
        }
    }

    ddeck.delete_sys_overlay();
    ddeck.update_display();

    return cursor;
}

bool DD_UI::notification(String msg){
    Layer_id_t notif_layer = ddeck.create_sys_overlay(200, 100, 60, 70, 1);
    if(notif_layer == nullptr){                     // layer creation failed or sys_overlay already exists
        Serial.println(TXT_UNABLE_CREATE_NOTIFF);
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

    notif_layer->canvas->setTextColor(TFT_WHITE);
    notif_layer->canvas->setTextSize(2);
    uint16_t font_h = notif_layer->canvas->fontHeight();

    notif_layer->canvas->drawRect(0, 0, 200, 100, TFT_WHITE);
    notif_layer->canvas->setTextSize(2);
    notif_layer->canvas->setTextColor(TFT_WHITE);
    for(uint8_t i = 0; i < msg_lines.size(); i++)
        notif_layer->canvas->drawCentreString(msg_lines[i], 100, 5 + i * font_h, 1);

    ddeck.update_display();

    DD_GLOBAL::notification_destruction_time = millis() + NOTIFICATION_PRESENSE_TIME;

    return 1;
}

void DD_UI::init_game_downloading_screen(Game_config_t game_data, String dir){
    ddeck.clear_canvas();

    ddeck.canvas->setDefaultGraphicsParams();
    ddeck.canvas->setTextSize(2);

    uint16_t cursor_x = (ddeck.canvas->width() - ddeck.canvas->textWidth(game_data.name)) / 2;
    uint16_t y0 = 20;
    ddeck.canvas->setCursor(cursor_x, y0);
    ddeck.canvas->print(game_data.name);

    ddeck.canvas->drawRect(10, ddeck.canvas->fontHeight() + GAME_ICON_SIZE + y0 + 8, ddeck.canvas->width() - 20, 8, TFT_WHITE);

    if(game_data.icon_path != ""){
        DD_SD_card temp_fs;
        temp_fs.init(dir);

        temp_fs.open_file(game_data.icon_path);
        ddeck.canvas->drawPNGFromFile(
            temp_fs.get_file_reference(), 
            (ddeck.canvas->width() - GAME_ICON_SIZE) / 2, 
            ddeck.canvas->fontHeight() + y0 + 4, 
            true
        );
        temp_fs.close_file();

        ddeck.canvas->setCursor(GAME_ICON_SIZE + 2, 0);
    }

    ddeck.update_display();
}

void DD_UI::game_downloading_screen(uint8_t percentage){
    ddeck.canvas->setTextSize(2);

    uint16_t width = round((float) percentage / 100.0 * (ddeck.canvas->width() - 24));
    uint16_t start_h = ddeck.canvas->fontHeight() + GAME_ICON_SIZE + 30;

    ddeck.canvas->fillRect(12, start_h, ddeck.canvas->width() - 24, 4, TFT_BLACK);
    ddeck.canvas->fillRect(12, start_h, width, 4, TFT_WHITE);

    ddeck.canvas->fillRect(50, start_h + ddeck.canvas->fontHeight(), ddeck.canvas->width() - 100, ddeck.canvas->fontHeight(), TFT_BLACK);
    String percentage_str = String(percentage) + "%";
    uint16_t cursor_x = (ddeck.canvas->width() - ddeck.canvas->textWidth(percentage_str)) / 2;
    ddeck.canvas->setCursor(cursor_x, start_h + ddeck.canvas->fontHeight());
    ddeck.canvas->print(percentage_str);

    ddeck.update_display();
}

void DD_UI::on_charge_screen(bool invert){
    Graphics_params_t init_graphics = ddeck.canvas->graphicsParams();
    ddeck.canvas->setDefaultGraphicsParams();

    ddeck.canvas->fillSprite((invert) ? TFT_WHITE : TFT_BLACK);

    ddeck.canvas->drawRect(100, 85, 120, 70, (invert) ? TFT_BLACK : TFT_WHITE);
    ddeck.canvas->setTextSize(2);
    ddeck.canvas->setTextColor((invert) ? TFT_BLACK : TFT_WHITE);
    ddeck.canvas->drawCentreString("CHARGING", 160, 120 - ddeck.canvas->fontHeight() / 2, 1);
    
    ddeck.update_display();

    ddeck.canvas->setGraphicsParams(init_graphics);
}





void empty_button_render(DD_UI_button *button, uint8_t skin, void *params){}

DD_UI_button::DD_UI_button(uint16_t id_, uint16_t x_, uint16_t y_, uint16_t w_, uint16_t h_){
    id = id_;
    x = x_;
    y = y_;
    w = w_;
    h = h_;

    render_func = empty_button_render;
}

void DD_UI_button::set_neighbours(int16_t up_id_, int16_t down_id_, int16_t left_id_, int16_t right_id_){
    up_id = up_id_;
    down_id = down_id_;
    left_id = left_id_;
    right_id = right_id_;
}

void DD_UI_button::assign_render_function(void (*render_func_)(DD_UI_button *, uint8_t, void *)){
    render_func = render_func_;
}

void DD_UI_button::render(void* params, uint8_t skin, bool clear){
    if(clear)
        ddeck.canvas->fillRect(x, y, w, h, TFT_BLACK);
    
    render_func(this, skin, params);
}

void DD_UI_button::render(uint8_t skin, bool clear){
    render(nullptr, skin, clear);
}



namespace DD_GLOBAL{
    DD_UI UI;

    uint32_t notification_destruction_time = 0;
}