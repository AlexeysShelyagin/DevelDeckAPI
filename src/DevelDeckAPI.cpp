#include "DevelDeckAPI.h"

#include "SPIFFS.h"

using namespace GAMEPAD_GLOBAL;



// ================================= GLOBAL GAMEPAD VARIABLES ====================================

Gamepad gamepad;

bool __attribute__((weak)) GAME_FILES_REQUIRED = false;

// ===============================================================================================



// =============================== NON-CLASS variables ===========================================

void (*game_loop)() = loop;

TaskHandle_t sys_task_handler = NULL;
TaskHandle_t game_task_handler = NULL;
TaskHandle_t battery_listener_handler = NULL;
TaskHandle_t forced_main_menu_handler = NULL;
TaskHandle_t display_updater_handler = NULL;

bool disp_transaction_block = false;
TaskHandle_t disp_transaction_owner = NULL;

Gamepad_battery::Charge_mode_t batt_mode;
float battery_critical_v;

bool system_event_flag = false;
bool forced_menu_call = false;
bool is_discharged = false;
bool notify_low_charge = false;

uint64_t last_disp_update = 0;

struct threaded_update_params_t{
    float dt;
    bool ignore_layers;
    Layer_id_t target;

    int16_t x0, y0;
    uint16_t w, h;
};

// ===============================================================================================



// ========================= GAMEPAD INITIALIZATION BEFORE USER CODE =============================

void init(){
    gamepad.init__();
}

// ===============================================================================================





// ==================================== RTOS PROCESSES ===========================================

void battery_listener(void *params){
    Gamepad_battery *batt = (Gamepad_battery *) params;
    uint32_t last_charge_check = 0;
    float deadband = 0;

    while(true){
        Gamepad_battery::Charge_mode_t batt_mode = batt->get_device_mode();

        if(batt_mode == Gamepad_battery::POWER_OFF)
            ESP.restart();
        
        if(batt_mode == Gamepad_battery::POWER_ON){
            if(millis() - last_charge_check > BATTERY_LEVEL_CHECK_TIMEOUT){
                last_charge_check = millis();

                if(batt->get_battery_voltage() <= battery_critical_v + deadband){
                    deadband = BATTERY_DISCHARGED_DEADBAND;
                    is_discharged = true;
                    system_event_flag = true;
                }
                else{
                    deadband = 0;
                    is_discharged = false;
                }
                
                if(batt->get_battery_charge() == 0){
                    notify_low_charge = true;
                    system_event_flag = true;
                }
            }
        }

        if(batt_mode == Gamepad_battery::CHARGING){
            if(is_discharged)
                is_discharged = false;
        }

        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(DEVICE_MODE_CHECK_TIMEOUT));
    }
}

void forced_main_menu_listener(void *params){
    bool pressed = false;
    uint64_t st;

    while(true){
        uint64_t now_time = esp_timer_get_time();

        if(get_latest_button_state(MENU_BUT_ID)){
            if(!pressed){
                pressed = true;
                st = now_time;
            }
        }
        else
            pressed = false;

        if(pressed){
            if(now_time - st >= FORCED_MENU_HOLD_TIME * 1000){
                system_event_flag = true;
                forced_menu_call = true;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(FORCED_MENU_CHECK_TIMEOUT));
    }
}

void display_update_thread(void *params){
    threaded_update_params_t job = * (threaded_update_params_t *) params;
    delete (float *) params;

    while(micros() - last_disp_update < job.dt)
        vTaskDelay(0);
    
    last_disp_update = micros();

    if(job.target == nullptr)
        gamepad.update_display(job.ignore_layers, job.x0, job.y0, job.w, job.h);
    else
        gamepad.update_layer(job.target, job.x0, job.y0, job.w, job.h);
    
    vTaskDelete(NULL);
}

// ===============================================================================================







// ===================================== MAIN LOOP ===============================================

void game_task(void *params){
    while(true){
        game_loop();
        vTaskDelay(0);

        if (serialEventRun) serialEventRun();   // from core main.cpp
    }
}

void suspend_game(){
    while(disp_transaction_owner == game_task_handler)
        disp_transaction_block = true;
    disp_transaction_block = false;

    gamepad.buzzer.stop();
    gamepad.vibro.disable();
    vTaskSuspend(game_task_handler);
}

void Gamepad::main_loop(void (*game_func_)()){
    game_loop = game_func_;

    vTaskPrioritySet(NULL, SYS_TASK_PRIORITY);
    sys_task_handler = xTaskGetCurrentTaskHandle();

    uint32_t last_low_charge_alarm = 0;

    xTaskCreatePinnedToCore(
        game_task,
        "game",
        GAME_STACK_SIZE,
        NULL,
        GAME_LOOP_TASK_PRIORITY,
        &game_task_handler,
        THIS_CORE
    );

    while(true){        
        if(system_event_flag){
            if(forced_menu_call){
                suspend_game();

                gamepad.main_menu();

                vTaskResume(game_task_handler);
                forced_menu_call = false;
            }

            if(notify_low_charge){
                if(millis() - last_low_charge_alarm >= BATTERY_LOW_CHARGE_ALARM_TIMEOUT){
                    UI.notification(TXT_LOW_CHARGE_ALARM);
                    last_low_charge_alarm = millis();
                }
            }

            if(is_discharged){
                // finishing calibration if is present
                if(batt.is_calibrating()){
                    if(batt.finish_calibration() != nullptr)
                        gamepad.save_system_settings();
                    else{
                        UI.notification(TXT_FAILED_BATT_CALIBRATION);
                        vTaskDelay(pdMS_TO_TICKS(NOTIFICATION_HOLD_TIME));
                    }
                }
                
                // suspension
                suspend_game();

                // notification
                UI.notification(TXT_DISCHARGED);
                disp_transaction_block = true;      // save the world
                vTaskDelay(pdMS_TO_TICKS(NOTIFICATION_HOLD_TIME));
                uint8_t brightness_before_suspension = get_display_brightness();
                set_display_brightness(0);

                // wait until charged
                while(is_discharged){
                    esp_sleep_enable_timer_wakeup(1000ULL * BATTERY_LIGHT_SLEEP_CHECK_TIMEOUT);
                    esp_light_sleep_start();
                    xTaskNotifyGive(battery_listener_handler);
                    vTaskDelay(pdMS_TO_TICKS(50));
                }

                // resume system
                disp_transaction_block = false;
                gamepad.update_display();
                gamepad.set_display_brightness(brightness_before_suspension);
                vTaskResume(game_task_handler);
            }

            system_event_flag = false;
        }

        if(notification_layer_id != nullptr){
            if( (int64_t) notification_destruction_time - millis() <= 0){
                delete_layer(notification_layer_id);
                notification_destruction_time = 0;
                notification_layer_id = nullptr;
                update_display();
            }
        }

        // notify if battery calibration failed 
        /*
        if(batt.is_calibrating() && batt.calibration_failed()){
            UI.notification(TXT_FAILED_BATT_CALIBRATION);
            batt.finish_calibration();
        }
        */

        vTaskSuspend(game_task_handler);
        vTaskDelay(1);
        vTaskResume(game_task_handler);
        
        vTaskDelay(pdMS_TO_TICKS(SYSTEM_EVENT_CHECK_TIMEOUT));
    }
}

// ===============================================================================================





// ================================ TINITIALIZATION ROUTINE ======================================

void Gamepad::init__(){
    if(sys_param(INITIALIZED))
        return;
    sys_param(INITIALIZED, 1);
    sys_param(READY_TO_PLAY, 1);

    Serial.begin(115200);

    init_SPIFFS();
    init_system_data();

    init_display();
    init_buttons();

    init_vibro();
    init_buzzer();
    init_accel();
    
    init_SD();

    locate_game();
    if(GAME_FILES_REQUIRED)
        sys_param(READY_TO_PLAY, sys_param(GAME_FILES_LOCATED));
    
    if(sys_param(SYSTEM_SETTINGS_TO_DEFAULT))
        save_system_settings();
    else
        apply_system_settings();

#ifdef DUMP_SYS_DATA_ON_INIT
    system_data_dump();
#endif

    if(buttons.read_state(MENU_BUT_ID))
        main_menu();
    
    init_battery();
    battery_critical_v = system_data->battery_critical_v;

    xTaskCreatePinnedToCore(
        forced_main_menu_listener,
        "menu",
        FORCED_MENU_STACK_SIZE,
        NULL,
        FORCED_MENU_TASK_PRIORITY,
        &forced_main_menu_handler,
        DIFFERENT_CORE
    );

    if(!sys_param(READY_TO_PLAY))
        main_menu();

}



void Gamepad::init_display(){
    disp = new Gamepad_display();

    if(system_data->brightness == 0){
        system_data->brightness = BRIGHTNESS_LEVELS;
        brightness = BRIGHTNESS_LEVELS;
    }
    set_display_brightness(brightness);

    canvas = disp->get_canvas_reference();
    if(!(disp->init())){
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
        system_data->battery_critical_v,
        system_data->battery_full_v,
        system_data->battery_charging_v,
        system_data->battery_only_charging_v
    );

    batt.set_voltage_adjustment(BATTERY_VADJ_FUNC);

    batt_mode = batt.get_device_mode();
    if(batt_mode == Gamepad_battery::POWER_OFF)
        on_charge_screen();

    xTaskCreatePinnedToCore(
        battery_listener,
        "batt",
        BATTERY_LISTENER_STACK_SIZE,
        &batt,
        BATTERY_LISTENER_TASK_PRIORITY,
        &battery_listener_handler,
        DIFFERENT_CORE
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



// ===============================================================================================




// ================================== USER-USE FUNCTIONS =========================================

// ------------------------- Wrappers ----------------------------

uint8_t Gamepad::get_battery_charge(){
    return batt.get_battery_charge();
}

// ---------------------------------------------------------------




// ------------------------- Display -----------------------------

void Gamepad::clear_canvas(){
    disp->clear_canvas();
}

void Gamepad::update_display(bool ignore_layers, int16_t x0, int16_t y0, uint16_t w, uint16_t h){
    if(!sys_param(DISPLAY_ENABLED))
		return;

    if(disp_transaction_block || disp_transaction_owner != NULL)
        return;
    
    disp_transaction_owner = xTaskGetCurrentTaskHandle();
    
    if(w == 0 || h == 0)
        disp->display_canvas();
    else
        disp->display_canvas(x0, y0, w, h);

    if(!ignore_layers){
        for(uint8_t i = 0; i < layers.size(); i++)
            disp->display_sprite(layers[i]->canvas, layers[i]->x, layers[i]->y);
    }

    for(uint8_t i = 0; i < sys_layers.size(); i++)
        disp->display_sprite(sys_layers[i]->canvas, sys_layers[i]->x, sys_layers[i]->y);

    disp_transaction_owner = NULL;
}



void Gamepad::update_display_threaded(bool ignore_layers, float fps_max, 
                                    int16_t x0, int16_t y0, uint16_t w, uint16_t h){
    if(!sys_param(DISPLAY_ENABLED))
		return;
    
    if(!update_display_threaded_available())
        return;
    
    threaded_update_params_t *update_job = new threaded_update_params_t;
    update_job->target = nullptr;
    update_job->ignore_layers = ignore_layers;
    update_job->dt = 0;
    if(fps_max != 0)
        update_job->dt = 1000000.0 / fps_max;
    
    update_job->x0 = x0; update_job->y0 = y0;
    update_job->w = w; update_job->h = h;

    xTaskCreatePinnedToCore(
        display_update_thread,
        "disp",
        DISPLAY_UPDATE_THREAD_STACK_SIZE,
        update_job,
        DISP_THREADED_TASK_PRIORITY,
        &display_updater_handler,
        DIFFERENT_CORE
    );
}

bool Gamepad::update_display_threaded_available(){
    if(display_updater_handler != NULL)
        return (eTaskGetState(display_updater_handler) == eDeleted);
    return (display_updater_handler == NULL);
}



void Gamepad::set_display_brightness(uint8_t brightness_){
    brightness = brightness_;

    if(brightness >= BRIGHTNESS_LEVELS)
        disp->set_brightness(255);
    else if(brightness == 0)
        disp->set_brightness(0);
    else
        disp->set_brightness(254.0 / (BRIGHTNESS_LEVELS - 1) * (brightness - 1) + 1);
}

uint8_t Gamepad::get_display_brightness(){
    return brightness;
}

// ---------------------------------------------------------------




// --------------------- Display layers --------------------------

Layer_id_t Gamepad::create_layer(uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t color_depth){
    Gamepad_canvas_t *layer_canvas = disp->create_sprite(width, height, color_depth);
    if(layer_canvas == nullptr)
        return nullptr;
    
    Layer_t *layer = new Layer_t;
    layer->canvas = layer_canvas;
    layer->x = x;
    layer->y = y;
    layers.push_back(layer);

    return layer;
}

bool Gamepad::layer_exists(Layer_id_t &id){
    return (id != nullptr);
}

Gamepad_canvas_t* Gamepad::layer(Layer_id_t &id){
    return id->canvas;
}

void Gamepad::delete_layer(Layer_id_t &id){
    disp->delete_sprite(id->canvas);
    for(uint8_t i = 0; i < layers.size(); i++){
        if(layers[i] == id){
            layers.erase(layers.begin() + i);
            break;
        }
    }
    delete id;
    id = nullptr;
}



void Gamepad::clear_layer(Layer_id_t &id){
    disp->clear_sprite(id ->canvas);
}

void Gamepad::move_layer(Layer_id_t &id, uint16_t new_x, uint16_t new_y){
    id->x = new_x;
    id->y = new_y;
}



void Gamepad::update_layer(Layer_id_t &id, int16_t x0, int16_t y0, uint16_t w, uint16_t h){
     if(!sys_param(DISPLAY_ENABLED))
		return;

    if(disp_transaction_block || disp_transaction_owner != NULL)
        return;

    disp_transaction_owner = xTaskGetCurrentTaskHandle();
    
    if(id == nullptr)
        return;
    
    if(w == 0 || h == 0)
        disp->display_sprite(id->canvas, id->x, id->y);
    else
        disp->display_sprite(id->canvas, id->x, id->y, x0, y0, w, h);

    disp_transaction_owner = NULL;
}

void Gamepad::update_layer_threaded(Layer_id_t &id, float fps_max, int16_t x0, int16_t y0, uint16_t w, uint16_t h){
    if(!sys_param(DISPLAY_ENABLED))
		return;

    if(id == nullptr)
        return;
    
    if(!update_display_threaded_available())
        return;
    
    threaded_update_params_t *update_job = new threaded_update_params_t;
    update_job->target = id;
    update_job->dt = 0;
    if(fps_max != 0)
        update_job->dt = 1000000.0 / fps_max;

    update_job->x0 = x0; update_job->y0 = y0;
    update_job->w = w; update_job->h = h;

    xTaskCreatePinnedToCore(
        display_update_thread,
        "disp",
        DISPLAY_UPDATE_THREAD_STACK_SIZE,
        update_job,
        DISP_THREADED_TASK_PRIORITY,
        &display_updater_handler,
        DIFFERENT_CORE
    );
}

// ---------------------------------------------------------------




// ------------------------ UI backend ---------------------------

void Gamepad::main_menu(){
    buttons.clear_queue();
    uint8_t cursor = 0;

    while(true){
        cursor = UI.main_menu(sys_param(READY_TO_PLAY), sys_param(SD_ENABLED), cursor);
        buttons.clear_queue();

        if(cursor == 0){
            if(sys_param(READY_TO_PLAY))
                break;  
            else{
                std::vector < String > buttons = {"Ok", "Cancel"};
                uint8_t response = UI.message_box(GAME_FILES_NOT_FOUND_MSG, buttons);
                if(response == 0){
                    if(!sys_param(SD_ENABLED))
                        UI.message_box(NO_SD_CARD_MSG);
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
    uint8_t resp = UI.settings(updated_data);

    if(resp == 1){
        *system_data = updated_data;
        save_system_settings();
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
            UI.notification(BATTERY_CALIBRATION_MSG);
        }
    }

    buttons.clear_queue();
}



void Gamepad::select_game_menu(){
    if(!sys_param(SD_ENABLED)){
        UI.message_box(NO_SD_CARD_MSG);
        return;
    }

    File_mngr_t file = UI.file_manager(true);
    if(file.file == "")
        return;

    if(file.game_config->minimum_flash * 1024 * 1024 > ESP.getFlashChipSize()){
        UI.notification(TXT_UNSUPPORTED_DEVICE + String(file.game_config->minimum_flash) + "MB required");
        return;
    }

    if(file.game_config != nullptr){
        sd_card.open_dir(file.dir, 1);
        sd_card.open_file(file.game_config->game_path);

        UI.init_game_downloading_screen(*file.game_config, file.dir);
        if( OTA_update(*sd_card.get_file_reference()) ){
            system_data->game_path_size = file.dir.length();
            for(uint8_t i = 0; i < system_data->game_path_size; i++)
                system_data->game_path[i] = file.dir[i];
            
            save_system_settings();

            ESP.restart();
        }
    }
}



String Gamepad::file_manager(){
    if(!GAME_FILES_REQUIRED)
        return "";
    
    buttons.clear_queue();

    File_mngr_t file = UI.file_manager(game_path);
    String target = file.dir.substring(game_path.length(), file.dir.length());
    target += "/" + file.file;

    return target;
}

// ---------------------------------------------------------------

// ===============================================================================================





// ============================= FUNCTIONS FOR ONLY API-USE ======================================

// --------------------- System level layers ---------------------

Layer_id_t Gamepad::create_system_layer(uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t color_depth){
    Gamepad_canvas_t *layer_canvas = disp->create_sprite(width, height, color_depth);
    if(layer_canvas == nullptr)
        return nullptr;
    
    Layer_t *layer = new Layer_t;
    layer->canvas = layer_canvas;
    layer->x = x;
    layer->y = y;
    sys_layers.push_back(layer);

    return layer;
}

// -------------- Gamepad settings and parameters ----------------

bool Gamepad::sys_param(Sys_param_t id){
    return system_params >> id & 1;
}

void Gamepad::sys_param(Sys_param_t id, bool val){
    system_params &= ~(1 << id);
    system_params |= ((uint8_t) val) << id;
}

void Gamepad::system_data_dump(){
    Serial.println("==================System data dump====================");

    char path[system_data->game_path_size];
    for(uint8_t i = 0; i < system_data->game_path_size; i++)
        path[i] = system_data->game_path[i];
    
    Serial.printf("Game path len:   %d\n", system_data->game_path_size);
    Serial.print("Game path:       ");
    Serial.println(path);
    
    Serial.printf("Buzzer volume:   %d\n", system_data->buzzer_volume);
    Serial.printf("Brightness:      %d\n", system_data->brightness);
    Serial.printf("Vibro strength:  %d\n", system_data->vibro_strength);

    Serial.printf("Critical V:      %f\n", system_data->battery_critical_v);
    Serial.printf("Charging V:      %f\n", system_data->battery_charging_v);
    Serial.printf("Only charging V: %f\n", system_data->battery_only_charging_v);
    Serial.printf("Full V:          %f\n", system_data->battery_full_v);
    
    Serial.printf("Batt levels N:   %d\n", system_data->battery_levels_n);
    for(uint8_t i = 0; i < system_data->battery_levels_n; i++)
        Serial.printf("\t%d: %f\n", i, system_data->battery_levels[i]);
    Serial.printf("Batt lifetime:   %d\n", system_data->battery_lifetime);

    Serial.println("======================================================");
}



void Gamepad::locate_game(){
    if(GAME_FILES_REQUIRED && sys_param(SD_ENABLED)){
        game_path = "";
        for (uint8_t i = 0; i < system_data->game_path_size; i++)
            game_path += system_data->game_path[i];
        
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
    
    buzzer.change_volume(settings->buzzer_volume);
    set_display_brightness(settings->brightness);
    vibro.strength = settings->vibro_strength;

    if(settings->battery_levels_n == 0)
        batt.set_calibration_data(nullptr);
    else{
        batt.set_calibration_data(settings->battery_levels);
        batt.lifetime = settings->battery_lifetime;
    }
}

void Gamepad::apply_system_settings(){
    apply_system_settings(system_data);
}



void Gamepad::save_system_settings(){    
    system_data->buzzer_volume = buzzer.get_volume();
    system_data->brightness = get_display_brightness();
    system_data->vibro_strength = vibro.strength;

    if(batt.calibrated()){
        system_data->battery_levels_n = BATTERY_LEVELS;
        float* batt_data = batt.get_calibration_data();
        for(uint8_t i = 0; i < BATTERY_LEVELS; i++)
            system_data->battery_levels[i] = batt_data[i];
    }
    else{
        system_data->battery_levels_n = 0;
    }

    system_data->battery_critical_v = BATTERY_CRITICAL_V;
    system_data->battery_full_v = BATTERY_FULL_V;
    system_data->battery_charging_v = BATTERY_CHARGING_V;
    system_data->battery_only_charging_v = BATTERY_ONLY_CHARGING_V;

    system_data->battery_lifetime = batt.lifetime;

    File sys_data = SPIFFS.open(GAMEPAD_DATA_FILE_NAME, "w");
    sys_data.write((uint8_t *) system_data, sizeof(System_data_t));
    sys_data.close();
}



void Gamepad::user_locate_game_folder(){
    while(true){
        File_mngr_t selected = UI.file_manager();
        
        sd_card.open_dir(selected.dir, true);
        sd_card.open_file(GAME_CONFIG_FILE_NAME);
        String file = sd_card.file_read_string();
        sd_card.close_file();
        Game_config_t config = read_game_config(file);

        if(config.game_path != ""){
            game_path = selected.dir;
            system_data->game_path_size = game_path.length();
            for(uint8_t i = 0; i < game_path.length(); i++)
                system_data->game_path[i] = game_path[i];
            save_system_settings();

            uint8_t init_status = game_files.init(game_path);
            sys_param(GAME_FILES_LOCATED, (init_status == Gamepad_SD_card::SD_OK));
            sys_param(READY_TO_PLAY, (init_status == Gamepad_SD_card::SD_OK));

            break;
        }
        UI.message_box(NOT_GAME_FOLDER_MSG);
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
    UI.game_downloading_screen(percentage);
}



void Gamepad::on_charge_screen(){
    uint8_t brightness = get_display_brightness();
    bool initial = true;

    for(uint8_t i = 0; i < BUTTONS_N; i++)
        gpio_wakeup_enable((gpio_num_t) buttons_map[i], (INVERT_BUTTONS_STATE) ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    
    while(batt.get_device_mode() == Gamepad_battery::POWER_OFF){
        if(buttons.event_available() || initial){
            UI.on_charge_screen();
            delay(30);
            set_display_brightness(brightness);

            delay(2000);

            set_display_brightness(0);
            UI.on_charge_screen(true);

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