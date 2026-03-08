#include "DevelDeckAPI.h"

#include "SPIFFS.h"


// ================================== GLOBAL GAMEPAD VARIABLE ====================================

Gamepad gamepad;

// ===============================================================================================



// =============================== NON-CLASS variables ===========================================

TaskHandle_t *main_loop_handler = nullptr;
Gamepad_UI ui;
uint64_t last_disp_update = 0;
float battery_critical_v;

// ===============================================================================================



// ============================= BATTERY ADJUSTMENT FUNCTIONS ====================================

// Battery voltage is calculated from analog value and converted to RAW voltage
// However, adjustment function is needed, due to internal resistanse, nonlinearity of ADC etc.

float batt_v_adj_0(float v){
    return BATTERY_VADJ_FUNC_0(v);
}

float batt_v_adj_1(float v){
    return BATTERY_VADJ_FUNC_1(v); 
}

// ===============================================================================================





// ==================================== RTOS PROCESSES ===========================================

void battery_listener(void *params){
    Gamepad_battery *batt = (Gamepad_battery *) params;
    uint64_t last_check = 0;
    uint64_t last_alarm = 0;

    while(true){
        if(millis() - last_check >= BATTERY_CHECK_TIMEOUT){
            if(xSemaphoreTake(gamepad.semaphore, portMAX_DELAY)){
                if(batt -> get_device_mode() == Gamepad_battery::POWER_ON){
                    bool suspended = false;
                    float threshold = 0;
                    uint8_t disp_brightness = gamepad.get_display_brightness();
                    while(batt -> get_battery_voltage() <= battery_critical_v + threshold){
                        if(!suspended){
                            if(batt -> is_calibrating()){
                                float *levels = batt -> finish_calibration();

                                if(levels == nullptr){
                                    ui.notification(TXT_FAILED_CALIBR);
                                    vTaskDelay(pdMS_TO_TICKS(2000));
                                }
                                else{
                                    gamepad.save_system_settings();
                                }
                            }

                            vTaskSuspend(*main_loop_handler);
                            ui.notification("Discharged");
                            delay(NOTIFICATION_HOLD_TIME);
                            gamepad.set_display_brightness(0);      // TODO: print to display
                            delay(10);
                            threshold = 0.1;
                            suspended = true;
                        }

                        esp_sleep_enable_timer_wakeup(1000ULL * BATTERY_LIGHT_SLEEP_CHECK_TIMEOUT);
                        esp_light_sleep_start();
                        delay(50);
                    }
                    if(suspended){
                        gamepad.set_display_brightness(disp_brightness);
                        vTaskResume(*main_loop_handler);
                    }
                    
                    if(batt -> get_battery_charge() == 0 && millis() - last_alarm >= BATTERY_LOW_CHARGE_ALARM_TIMEOUT){
                        ui.notification("Low charge");
                        last_alarm = millis();
                    }
                }
            }

            if(batt -> is_calibrating()){
                if(batt -> calibration_failed()){
                    if(ui.notification(TXT_FAILED_CALIBR))
                        batt -> finish_calibration();
                }
            }

            xSemaphoreGive(gamepad.semaphore);

            last_check = millis();
        }

        if(batt -> get_device_mode() == Gamepad_battery::ONLY_CHARHING)
            ESP.restart();

        vTaskDelay(pdMS_TO_TICKS(DEVICE_MODE_CHECK_TIMEOUT));
    }
}

void forced_main_menu_listener(void *params){
    while(true){
        if(xSemaphoreTake(gamepad.semaphore, portMAX_DELAY)){
            if(gamepad.buttons.get_latest_state(MENU_BUT_ID)){
                xSemaphoreGive(gamepad.semaphore);
                
                uint64_t timer = millis();
                while(true){
                    if(xSemaphoreTake(gamepad.semaphore, portMAX_DELAY)){
                        if(!gamepad.buttons.get_latest_state(MENU_BUT_ID) || millis() - timer >= FORCED_MENU_HOLD_TIME)
                            break;
                    }

                    xSemaphoreGive(gamepad.semaphore);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                
                if(millis() - timer >= FORCED_MENU_HOLD_TIME){
                    vTaskSuspend(*main_loop_handler);

                    gamepad.buttons.clear_queue();
                    gamepad.main_menu();

                    vTaskResume(*main_loop_handler);
                }
            }

            xSemaphoreGive(gamepad.semaphore);

            vTaskDelay(pdMS_TO_TICKS(FORCED_MENU_CHECK_TIMEOUT));
        }
    }
}

void display_update_thread(void *params){
    float *dt = (float *) params;
    xSemaphoreTake(gamepad.semaphore, portMAX_DELAY);

    while(micros() - last_disp_update < *dt)
        vTaskDelay(1);
    last_disp_update = micros();
    delete dt;

    gamepad.update_display();
    
    xSemaphoreGive(gamepad.semaphore);
    vTaskDelay(pdMS_TO_TICKS(2));
    vTaskDelete(NULL);
}

// ===============================================================================================







// ===================================== MAIN LOOP ===============================================


void Gamepad::main_loop(){
    while(true){
        game_func();
        
        give_access_to_subprocess();
    }
}

void Gamepad::give_access_to_subprocess(){
    xSemaphoreGive(semaphore);
    delay(1);
    xSemaphoreTake(semaphore, portMAX_DELAY);
}

// ===============================================================================================





// ================================ TINITIALIZATION ROUTINE ======================================

void Gamepad::init(void (*game_func_)()){
    Serial.begin(115200);

    game_func = game_func_;

    sys_param(READY_TO_PLAY, 1);

    main_loop_handler = new TaskHandle_t;
    *main_loop_handler = xTaskGetCurrentTaskHandle();
    semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(semaphore);

    init_SPIFFS();
    init_system_data();

    init_display();
    init_buttons();

    init_vibro();
    init_buzzer();
    init_accel();
    
    init_SD();

    locate_game();
    if(sys_param(GAME_FILES_REQ))
        sys_param(READY_TO_PLAY, sys_param(GAME_FILES_LOCATED));
    
    if(sys_param(SYSTEM_SETTINGS_TO_DEFAULT)){
        board_selection_menu();
        save_system_settings();
    }
    else
        apply_system_settings();

    if(buttons.read_state(MENU_BUT_ID))
        main_menu();
    
    init_battery();
    battery_critical_v = system_data -> battery_critical_v;
    
    xTaskCreatePinnedToCore(
        forced_main_menu_listener,
        "menu",
        FORCED_MENU_STACK_SIZE,
        &main_loop_handler,
        1,
        &forced_main_menu_handler,
        xPortGetCoreID()
    );

    if(!sys_param(READY_TO_PLAY))
        main_menu();

}



void Gamepad::init_display(){
    disp = new Gamepad_display();

    if(system_data -> brightness == 0){
        system_data -> brightness = BRIGHTNESS_LEVELS;
        brightness = BRIGHTNESS_LEVELS;
    }
    set_display_brightness(brightness);

    canvas = disp -> get_canvas_reference();
    if(!(disp -> init())){
        Serial.println(TXT_DISPAY_FAILED);
        return;
    }

    clear_canvas();

    sys_param(DISPLAY_ENABLED, 1);
}



bool Gamepad::init_buttons(){
    buttons.init();

    sys_param(BUTTONS_ENABLED, 1);

    return 1;
}



bool Gamepad::init_vibro(){
    vibro.init();

    sys_param(VIBRO_ENABLED, 1);

    return 1;
}



bool Gamepad::init_buzzer(){
    buzzer.init();

    sys_param(BUZZER_ENABLED, 1);

    return 1;
}



bool Gamepad::init_accel(){
    accel.init();

    sys_param(ACCEL_ENABLED, 1);

    return 1;
}



void Gamepad::init_battery(){
    batt.init(
        system_data -> battery_critical_v,
        system_data -> battery_full_v,
        system_data -> battery_charging_v,
        system_data -> battery_only_charging_v
    );

    switch (system_data -> hardware_config_id){
        case 0: batt.set_voltage_adjustment(batt_v_adj_0); break;
        case 1: batt.set_voltage_adjustment(batt_v_adj_1); break;
    }

    if(batt.get_device_mode() == Gamepad_battery::ONLY_CHARHING)
        on_charge_screen();

    xTaskCreatePinnedToCore(
        battery_listener,
        "batt",
        BATTERY_LISTENER_STACK_SIZE,
        &batt,
        1,
        &battery_listener_handler,
        xPortGetCoreID()
    );
}



bool Gamepad::init_SD(){
    uint8_t resp = sd_card.init();

    if(resp == Gamepad_SD_card::SD_FAILED){
        Serial.println(TXT_SD_FAILED);
        return 0;
    }
    if(resp == Gamepad_SD_card::SD_DISCONNECT){
        Serial.println(TXT_SD_DISCONNECT);
        return 0;
    }

    sys_param(SD_ENABLED, 1);
    return 1;
}

bool Gamepad::init_SPIFFS(){
    if(!SPIFFS.begin(true)){
        Serial.println(TXT_SPIFFS_FAILED);
        return 0;
    }
    
    sys_param(SPIFFS_ENABLED, 1);
    return 1;
}



void Gamepad::game_files_required(){
    sys_param(GAME_FILES_REQ, 1);
}

// ===============================================================================================




// ================================== USER-USE FUNCTIONS =========================================

// ------------------------- Wrappers ----------------------------

uint8_t Gamepad::get_battery_charge(){
    return batt.get_battery_charge();
}

// ---------------------------------------------------------------




// ------------------------- Display -----------------------------

void Gamepad::clear_canvas(){
    disp -> clear();
}

void Gamepad::update_display(){
    if(!sys_param(DISPLAY_ENABLED))
		return;
    disp -> update();

    for(uint8_t i = 0; i < layers.size(); i++)
        disp -> display_sprite(layers[i] -> canvas, layers[i] -> x, layers[i] -> y);
}



void Gamepad::update_display_threaded(float maintain_fps){            // abusing RTOS a bit ;)    (meet v-sync issues)
    if(!sys_param(DISPLAY_ENABLED))
		return;
    
    if(!update_display_threaded_available())
        return;
    
    float *dt = new float;
    *dt = 0;
    if(maintain_fps != 0)
        *dt = 1000000.0 / maintain_fps;

    xTaskCreatePinnedToCore(
        display_update_thread,
        "disp",
        DISPLAY_UPDATE_THREAD_STACK_SIZE,
        dt,
        1,
        &display_updater_handler,
        !xPortGetCoreID()
    );
    xSemaphoreGive(semaphore);
}

bool Gamepad::update_display_threaded_available(){
    return (xTaskGetHandle("disp") == NULL);
}



void Gamepad::set_display_brightness(uint8_t brightness_){
    brightness = brightness_;

    if(brightness >= BRIGHTNESS_LEVELS)
        disp -> set_brightness(255);
    else if(brightness == 0)
        disp -> set_brightness(0);
    else
        disp -> set_brightness(254.0 / (BRIGHTNESS_LEVELS - 1) * (brightness - 1) + 1);
}

uint8_t Gamepad::get_display_brightness(){
    return brightness;
}

// ---------------------------------------------------------------




// --------------------- Display layers --------------------------

Layer_id_t Gamepad::create_layer(uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t color_depth){
    Gamepad_canvas_t *layer_canvas = disp -> create_sprite(width, height, color_depth);
    if(layer_canvas == nullptr)
        return nullptr;
    
    Layer_t *layer = new Layer_t;
    layer -> canvas = layer_canvas;
    layer -> x = x;
    layer -> y = y;
    layers.push_back(layer);

    return layer;
}

bool Gamepad::layer_exists(Layer_id_t id){
    return (id != nullptr);
}

Gamepad_canvas_t* Gamepad::layer(Layer_id_t id){
    return id -> canvas;
}

void Gamepad::delete_layer(Layer_id_t id){
    disp -> delete_sprite(id -> canvas);
    for(uint8_t i = 0; i < layers.size(); i++){
        if(layers[i] == id){
            layers.erase(layers.begin() + i);
            break;
        }
    }
    delete id;
}



void Gamepad::clear_layer(Layer_id_t id){
    disp -> clear_sprite(id ->canvas);
}

void Gamepad::move_layer(Layer_id_t id, uint16_t new_x, uint16_t new_y){
    id -> x = new_x;
    id -> y = new_y;
}

// ---------------------------------------------------------------




// ------------------------ UI backend ---------------------------

void Gamepad::board_selection_menu(){
    buttons.clear_queue();

    system_data -> hardware_config_id = ui.board_selection_menu();
    switch (system_data -> hardware_config_id){
    case 0:
        system_data -> battery_critical_v = BATTERY_CRITICAL_V_0;
        system_data -> battery_full_v = BATTERY_FULL_V_0;
        system_data -> battery_charging_v = BATTERY_CHARGING_V_0;
        system_data -> battery_only_charging_v = BATTERY_ONLY_CHARGING_V_0;
        break;
    case 1:
        system_data -> battery_critical_v = BATTERY_CRITICAL_V_1;
        system_data -> battery_full_v = BATTERY_FULL_V_1;
        system_data -> battery_charging_v = BATTERY_CHARGING_V_1;
        system_data -> battery_only_charging_v = BATTERY_ONLY_CHARGING_V_1;
        break;
    }

    save_system_settings();

    buttons.clear_queue();
}

void Gamepad::main_menu(){
    buttons.clear_queue();
    uint8_t cursor = 0;

    while(true){
        cursor = ui.main_menu(sys_param(READY_TO_PLAY), sys_param(SD_ENABLED), cursor);
        buttons.clear_queue();

        if(cursor == 0){
            if(sys_param(READY_TO_PLAY))
                break;  
            else{
                std::vector < String > buttons = {"Ok", "Cancel"};
                uint8_t response = ui.message_box(GAME_FILES_NOT_FOUND_MSG, buttons);
                if(response == 0){
                    if(!sys_param(SD_ENABLED))
                        ui.message_box(NO_SD_CARD_MSG);
                    else{
                        user_locate_game_folder();
                    }
                }
            }
        }
        if(cursor == 1){
            settings_menu();
        }
        if(cursor == 2)
            select_game_menu();
    }

    buttons.clear_queue();
    clear_canvas();
    update_display();
}

void Gamepad::settings_menu(){
    buttons.clear_queue();

    System_data_t updated_data = *system_data;
    uint8_t resp = ui.settings(updated_data);

    if(resp == 1){
        *system_data = updated_data;
        save_system_settings();

        if(!sys_param(SD_ENABLED))
            ui.notification(TXT_SETTINGS_SAVE_WARINING);
    }
    if(resp == 1 || resp == 0)
        apply_system_settings();
    if(resp == 2){
        SPIFFS.remove(GAMEPAD_DATA_FILE_NAME);
        ESP.restart();
    }
    if(resp == 3){
        if(!batt.is_calibrating()){
            batt.start_calibration();
            ui.notification(BATTERY_CALIBRATION_MSG);
        }
    }

    buttons.clear_queue();
}



void Gamepad::select_game_menu(){
    if(!sys_param(SD_ENABLED)){
        ui.message_box(NO_SD_CARD_MSG);
        return;
    }

    File_mngr_t file = ui.file_manager(true);
    if(file.file == "")
        return;

    if(file.game_config -> minimum_flash * 1024 * 1024 > ESP.getFlashChipSize()){
        ui.notification(TXT_UNSUPPORTED_DEVICE + String(file.game_config -> minimum_flash) + "MB required");
        return;
    }

    if(file.game_config != nullptr){
        sd_card.open_dir(file.dir, 1);
        sd_card.open_file(file.game_config -> game_path);

        ui.init_game_downloading_screen(*file.game_config, file.dir);
        if( OTA_update(*sd_card.get_file_reference()) ){
            system_data -> game_path_size = file.dir.length();
            for(uint8_t i = 0; i < system_data -> game_path_size; i++)
                system_data -> game_path[i] = file.dir[i];
            
            save_system_settings();

            ESP.restart();
        }
    }
}



String Gamepad::file_manager(){
    if(!sys_param(GAME_FILES_REQ))
        return "";
    
    buttons.clear_queue();

    File_mngr_t file = ui.file_manager(game_path);
    String target = file.dir.substring(game_path.length(), file.dir.length());
    target += "/" + file.file;

    return target;
}

// ---------------------------------------------------------------

// ===============================================================================================





// ============================= FUNCTIONS FOR ONLY API-USE ======================================

// -------------- Gamepad settings and parameters ----------------

bool Gamepad::sys_param(Sys_param_t id){
    return system_params >> id & 1;
}

void Gamepad::sys_param(Sys_param_t id, bool val){
    system_params &= ~(1 << id);
    system_params |= ((uint8_t) val) << id;
}



void Gamepad::locate_game(){
    if(sys_param(GAME_FILES_REQ) && sys_param(SD_ENABLED)){
        game_path = "";
        for (uint8_t i = 0; i < system_data -> game_path_size; i++)
            game_path += system_data -> game_path[i];
        
        if(!sd_card.exists(game_path, 1) || game_path.length() == 0)
            return;
        
        uint8_t init_status = game_files.init(game_path);

        sys_param(GAME_FILES_LOCATED, (init_status == Gamepad_SD_card::SD_OK));
    }
}



void Gamepad::init_system_data(){
    system_data = new System_data_t;

    if(!sys_param(SPIFFS_ENABLED))
        return;

    File sys_data = SPIFFS.open(GAMEPAD_DATA_FILE_NAME);
    
    if(sys_data.size() < sizeof(System_data_t)){
        delete system_data;

        sys_data.close();
        sys_data = SPIFFS.open(GAMEPAD_DATA_FILE_NAME, "w");

        System_data_t *empty_data = new System_data_t;
        sys_data.write((uint8_t *) empty_data, sizeof(System_data_t));
        system_data = empty_data;

        sys_param(SYSTEM_SETTINGS_TO_DEFAULT, 1);
    }
    else
        sys_data.read((uint8_t *) system_data, sizeof(System_data_t));

    sys_data.close();
}



void Gamepad::apply_system_settings(System_data_t *settings){
    if (settings == nullptr)
        return;
    
    buzzer.change_volume(settings -> buzzer_volume);
    set_display_brightness(settings -> brightness);
    vibro.strength = settings -> vibro_strength;

    if(settings -> battery_levels_n == 0)
        batt.set_calibration_data(nullptr);
    else{
        batt.set_calibration_data(settings -> battery_levels);
        batt.lifetime = settings -> battery_lifetime;
    }
}

void Gamepad::apply_system_settings(){
    apply_system_settings(system_data);
}



void Gamepad::save_system_settings(){    
    system_data -> buzzer_volume = buzzer.get_volume();
    system_data -> brightness = get_display_brightness();
    system_data -> vibro_strength = vibro.strength;

    if(batt.calibrated()){
        system_data -> battery_levels_n = BATTERY_LEVELS;
        float* batt_data = batt.get_calibration_data();
        for(uint8_t i = 0; i < BATTERY_LEVELS; i++)
            system_data -> battery_levels[i] = batt_data[i];
    }
    else{
        system_data -> battery_levels_n = 0;
    }
    system_data -> battery_lifetime = batt.lifetime;

    File sys_data = SPIFFS.open(GAMEPAD_DATA_FILE_NAME, "w");
    sys_data.write((uint8_t *) system_data, sizeof(System_data_t));
    sys_data.close();
}



void Gamepad::user_locate_game_folder(){
    while(true){
        File_mngr_t selected = ui.file_manager();
        
        sd_card.open_dir(selected.dir, true);
        sd_card.open_file(GAME_CONFIG_FILE_NAME);
        String file = sd_card.file_read_string();
        sd_card.close_file();
        Game_config_t config = read_game_config(file);

        if(config.game_path != ""){
            game_path = selected.dir;
            system_data -> game_path_size = game_path.length();
            for(uint8_t i = 0; i < game_path.length(); i++)
                system_data -> game_path[i] = game_path[i];
            save_system_settings();

            uint8_t init_status = game_files.init(game_path);
            sys_param(GAME_FILES_LOCATED, (init_status == Gamepad_SD_card::SD_OK));
            sys_param(READY_TO_PLAY, (init_status == Gamepad_SD_card::SD_OK));

            break;
        }
        ui.message_box(NOT_GAME_FOLDER_MSG);
    }
}



Game_config_t Gamepad::read_game_config(String &config){
    int i = 0;
    config += '\n';
    Game_config_t res = {"", "", "", "", 0};

    while(i < config.length()){
        String param = "";
        while(config[i] != '=' && config[i] != '\n' && i < config.length()){
            if(config[i] >= '0')
                param += config[i];
            i++;
        }
        
        if(config[i] != '='){
            i++;
            continue;
        }
        
        i++;
        while(config[i] == ' ')
            i++;
        
        String val = "";
        while(config[i] != '\n' && i < config.length()){
            if(config[i] == '\\' && (config[i + 1] == '\n' || config[i + 1] == '\r')){
                i += 3;
                val += '\n';
                continue;
            }

            val += config[i];
            i++;
        }
        i++;

        if(param == "name")
            res.name = val;
        if(param == "description")
            res.description = val;
        if(param == "game_path")
            res.game_path = val;
        if(param == "icon")
            res.icon_path = val;
        if(param == "minimum_flash")
            res.minimum_flash = atoi(val.c_str());
    }

    return res;
}

// ---------------------------------------------------------------




// ---------------------- UI only for API ------------------------

void Gamepad::game_downloading_screen(uint8_t percentage){
    ui.game_downloading_screen(percentage);
}



void Gamepad::on_charge_screen(){
    uint8_t brightness = get_display_brightness();
    bool initial = true;

    for(uint8_t i = 0; i < BUTTONS_N; i++)
        gpio_wakeup_enable((gpio_num_t) buttons_map[i], (INVERT_BUTTONS_STATE) ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    
    while(batt.get_device_mode() == Gamepad_battery::ONLY_CHARHING){
        if(buttons.event_available() || initial){
            ui.on_charge_screen();
            delay(30);
            set_display_brightness(brightness);

            delay(2000);

            set_display_brightness(0);
            ui.on_charge_screen(true);

            buttons.clear_queue();
            initial = false;
        }

        esp_sleep_enable_timer_wakeup(2000000);
        esp_light_sleep_start();
    }
    
    ESP.restart();
}

// ---------------------------------------------------------------

// ===============================================================================================