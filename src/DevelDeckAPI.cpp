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
TaskHandle_t sys_event_listener_task_handler = NULL;
TaskHandle_t game_task_handler = NULL;
TaskHandle_t battery_listener_handler = NULL;
TaskHandle_t forced_main_menu_handler = NULL;
TaskHandle_t display_updater_handler = NULL;

bool disp_transaction_block = false;
TaskHandle_t disp_transaction_owner = NULL;

Gamepad_battery::Charge_mode_t batt_mode;
float battery_critical_v;

bool GAMEPAD_GLOBAL::forced_display_update = false;
bool forced_main_menu_call = false;
bool distruct_notification = false;
bool create_notification = false;
String notification_msg = "";

enum UI_call_t{
    UI_NONE,
    CALL_MAIN_MENU,
    CALL_GAME_SELECTION_MENU,
    CALL_SETTING_MENU,
    CALL_FILE_MANAGER
};
uint8_t UI_call_info = UI_NONE;
String file_manager_response = "";

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

#define trigger_system() xTaskNotifyGive(sys_task_handler)

void suspend_game(){
    if(eTaskGetState(game_task_handler) == eSuspended)
        return;
    
    while(disp_transaction_owner == game_task_handler){
        disp_transaction_block = true;
        vTaskDelay(0);
    }
    disp_transaction_block = false;

    gamepad.buzzer.stop();
    gamepad.vibro.disable();
    vTaskSuspend(game_task_handler);
}

inline void make_notification_helper(const char* msg){
    notification_msg = msg;
    create_notification = true;
    trigger_system();
}

void display_update_thread_task(void *params){
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



// ====================== SYSTEM EVENT LISTENER PROCESS ==========================================

inline void Gamepad::battery_listener_implementation(){
    Gamepad_battery::Charge_mode_t batt_mode = battery.get_device_mode();

    if(batt_mode == Gamepad_battery::POWER_OFF)
        ESP.restart();
    
    if(batt_mode == Gamepad_battery::POWER_ON){
        if(millis() - last_charge_check > BATTERY_LEVEL_CHECK_TIMEOUT || is_discharged){
            last_charge_check = millis();

            if(battery.get_battery_voltage() <= battery_critical_v + deadband_v){
                if(!is_discharged){
                    deadband_v = BATTERY_DISCHARGED_DEADBAND;
                    is_discharged = true;

                    // finishing calibration if is present
                    if(battery.is_calibrating()){
                        if(battery.finish_calibration() != nullptr)
                            gamepad.save_system_settings();
                        else{
                            make_notification_helper(TXT_FAILED_BATT_CALIBRATION);
                            vTaskDelay(pdMS_TO_TICKS(NOTIFICATION_PRESENSE_TIME));
                        }
                    }
                    
                    // suspension
                    suspend_game();

                    // notification
                    make_notification_helper(TXT_DISCHARGED);
                    vTaskDelay(pdMS_TO_TICKS(NOTIFICATION_PRESENSE_TIME));
                    disp_transaction_block = true;      // save the world
                    brightness_before_suspension = gamepad.get_display_brightness();
                    gamepad.set_display_brightness(0);
                    vTaskDelay(50);

                    esp_sleep_enable_timer_wakeup(1000ULL * BATTERY_LIGHT_SLEEP_CHECK_TIMEOUT);

                    vTaskSuspend(sys_task_handler);
                }
                
                // sleep
                esp_light_sleep_start();
            }
            else if(is_discharged){
                deadband_v = 0;
                resume_system = true;
            }
            
            // low charge alarm
            if(battery.get_battery_charge() == 0 || is_discharged == false){
                if(millis() - last_low_charge_alarm >= BATTERY_LOW_CHARGE_ALARM_TIMEOUT){
                    make_notification_helper(TXT_LOW_CHARGE_ALARM);
                    last_low_charge_alarm = millis();
                }
            }
        }
    }

    if(batt_mode == Gamepad_battery::CHARGING){
        if(is_discharged)
            resume_system = true;
    }

    if(resume_system){
        is_discharged = false;
        resume_system = false;

        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

        vTaskResume(sys_task_handler);
        disp_transaction_block = false;
        forced_display_update = true;
        trigger_system();
        gamepad.set_display_brightness(brightness_before_suspension);
        vTaskResume(game_task_handler);
    }
}

inline void Gamepad::forced_main_menu_listener_implementation(){
    uint64_t now_time = esp_timer_get_time();

    if(get_latest_button_state(MENU_BUT_ID)){
        if(!menu_pressed){
            menu_pressed = true;
            menu_pressed_st = now_time;
        }
    }
    else
        menu_pressed = false;

    if(menu_pressed){
        if(now_time - menu_pressed_st >= FORCED_MENU_HOLD_TIME * 1000){
            forced_main_menu_call = true;
            suspend_game();
            trigger_system();
        }
    }
}



void Gamepad::sys_event_listener_task(void *params){
    while(true){
        gamepad.forced_main_menu_listener_implementation();

        gamepad.battery_listener_implementation();

        if(forced_display_update)
            trigger_system();

        if(notification_destruction_time != 0){
            if(millis() > notification_destruction_time){
                gamepad.delete_sys_overlay();
                notification_destruction_time = 0;
                distruct_notification = true;
                trigger_system();
            }
        }

        if(battery.is_calibrating() && battery.calibration_failed()){
            make_notification_helper(TXT_FAILED_BATT_CALIBRATION);
            battery.finish_calibration();
        }

        if(!gamepad.is_discharged)
            ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(SYSTEM_EVENT_CHECK_TIMEOUT));
        else
            vTaskDelay(1);
    }
}

// =================================================================================================









// ===================================== SYSTEM MAIN ===============================================

void game_task(void *params){
    while(true){
        game_loop();
        vTaskDelay(0);

        if (serialEventRun) serialEventRun();   // from core main.cpp
    }
}

void Gamepad::main_loop(void (*game_func_)()){
    game_loop = game_func_;

    vTaskPrioritySet(NULL, SYS_TASK_PRIORITY);
    sys_task_handler = xTaskGetCurrentTaskHandle();
    
    xTaskCreatePinnedToCore(
        game_task,
        "game",
        GAME_STACK_SIZE,
        NULL,
        GAME_LOOP_TASK_PRIORITY,
        &game_task_handler,
        THIS_CORE
    );
    
    xTaskCreatePinnedToCore(
        sys_event_listener_task,
        "sys_events",
        SYS_EVENT_LISTENER_STACK_SIZE,
        this,
        SYS_EVENTS_TASK_PRIORITY,
        &sys_event_listener_task_handler,
        THIS_CORE
    );

    while(true){
        if(forced_display_update){
            gamepad.update_display();
            forced_display_update = false;
        }

        if(forced_main_menu_call){
            suspend_game();
            __main_menu();
            vTaskResume(game_task_handler);
            forced_main_menu_call = false;
        }

        if(create_notification){
            UI.notification(notification_msg);
            create_notification = false;
        }
        if(distruct_notification){
            gamepad.update_display();
            distruct_notification = false;
        }

        if(UI_call_info != UI_NONE){
            switch (UI_call_info){
                case CALL_MAIN_MENU:            __main_menu(); break;
                case CALL_GAME_SELECTION_MENU:  __select_game_menu(); break;
                case CALL_SETTING_MENU:         __settings_menu(); break;
                case CALL_FILE_MANAGER:         file_manager_response = __file_manager(); break;
                
                default: break;
            }
            UI_call_info = UI_NONE;
        }
        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
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
        __main_menu();
    
    init_battery();
    battery_critical_v = system_data->battery_critical_v;

    if(!sys_param(READY_TO_PLAY))
        __main_menu();

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
    battery.init(
        system_data->battery_critical_v,
        system_data->battery_full_v,
        system_data->battery_charging_v,
        system_data->battery_only_charging_v
    );

    battery.set_voltage_adjustment(BATTERY_VADJ_FUNC);

    batt_mode = battery.get_device_mode();
    if(batt_mode == Gamepad_battery::POWER_OFF)
        on_charge_mode();
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
    return battery.get_battery_charge();
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

    if(sys_overlay_layer.canvas != nullptr)
        disp->display_sprite(sys_overlay_layer.canvas, sys_overlay_layer.x, sys_overlay_layer.y);

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
        display_update_thread_task,
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
    if(id == nullptr)
        return;
    
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
        display_update_thread_task,
        "disp",
        DISPLAY_UPDATE_THREAD_STACK_SIZE,
        update_job,
        DISP_THREADED_TASK_PRIORITY,
        &display_updater_handler,
        DIFFERENT_CORE
    );
}

// ---------------------------------------------------------------




// --------------------------- UI user call -----------------------------

void Gamepad::main_menu(){
    UI_call_info = CALL_MAIN_MENU;
    trigger_system();
    while(UI_call_info != UI_NONE)
        vTaskDelay(1);
}

void Gamepad::settings_menu(){
    UI_call_info = CALL_SETTING_MENU;
    trigger_system();
    while(UI_call_info != UI_NONE)
        vTaskDelay(1);
}

void Gamepad::select_game_menu(){
    UI_call_info = CALL_GAME_SELECTION_MENU;
    trigger_system();
    while(UI_call_info != UI_NONE)
        vTaskDelay(1);
}

String Gamepad::file_manager(){
    UI_call_info = CALL_FILE_MANAGER;
    trigger_system();
    while(UI_call_info != UI_NONE)
        vTaskDelay(1);

    return file_manager_response;
}

// ---------------------------------------------------------------




// ------------------------ UI implementation ---------------------------

void Gamepad::__main_menu(){
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
            __settings_menu();
        }
        if(cursor == 2)
            __select_game_menu();
    }

    buttons.clear_queue();
    clear_canvas();
    update_display();
}

void Gamepad::__settings_menu(){
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
        if(!battery.is_calibrating()){
            //battery.start_calibration();
            UI.notification(BATTERY_CALIBRATION_MSG);
        }
    }

    buttons.clear_queue();
}



void Gamepad::__select_game_menu(){
    if(!sys_param(SD_ENABLED)){
        UI.message_box(NO_SD_CARD_MSG);
        return;
    }

    buttons.clear_queue();

    File_mngr_t file = UI.file_manager(true);
    if(file.file == ""){
        buttons.clear_queue();
        return;
    }

    if(file.game_config->minimum_flash * 1024 * 1024 > ESP.getFlashChipSize()){
        UI.notification(TXT_UNSUPPORTED_DEVICE + String(file.game_config->minimum_flash) + "MB required");
        buttons.clear_queue();
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
    
    buttons.clear_queue();
}



String Gamepad::__file_manager(){
    if(!GAME_FILES_REQUIRED)
        return "";
    
    buttons.clear_queue();

    File_mngr_t file = UI.file_manager(game_path);
    String target = file.dir.substring(game_path.length(), file.dir.length());
    target += "/" + file.file;

    buttons.clear_queue();
    return target;
}

// ---------------------------------------------------------------

// ===============================================================================================





// ============================= FUNCTIONS FOR ONLY API-USE ======================================

// --------------------- System level layers ---------------------

Layer_id_t Gamepad::create_sys_overlay(uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t color_depth){
    if(sys_overlay_layer.canvas != nullptr)
        return nullptr;
    
    sys_overlay_layer.canvas = disp->create_sprite(width, height, color_depth);
    if(sys_overlay_layer.canvas == nullptr)
        return nullptr;

    sys_overlay_layer.x = x;
    sys_overlay_layer.y = y;

    return &sys_overlay_layer;
}

void Gamepad::delete_sys_overlay(){
    if(sys_overlay_layer.canvas == nullptr)
        return;

    disp->delete_sprite(sys_overlay_layer.canvas);
    sys_overlay_layer.canvas = nullptr;
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
        battery.set_calibration_data(nullptr);
    else{
        battery.set_calibration_data(settings->battery_levels);
        battery.lifetime = settings->battery_lifetime;
    }
}

void Gamepad::apply_system_settings(){
    apply_system_settings(system_data);
}



void Gamepad::save_system_settings(){    
    system_data->buzzer_volume = buzzer.get_volume();
    system_data->brightness = get_display_brightness();
    system_data->vibro_strength = vibro.strength;

    if(battery.calibrated()){
        system_data->battery_levels_n = BATTERY_LEVELS;
        float* batt_data = battery.get_calibration_data();
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

    system_data->battery_lifetime = battery.lifetime;

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



void Gamepad::on_charge_mode(){
    uint8_t brightness_before = get_display_brightness();
    bool initial = true;
    esp_sleep_wakeup_cause_t cause;

    GAMEPAD_GLOBAL::stop_button_interrupts();
    for(uint8_t i = 0; i < BUTTONS_N; i++)
        gpio_wakeup_enable((gpio_num_t) buttons_map[i], (INVERT_BUTTONS_STATE) ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    
    while(battery.get_device_mode() == Gamepad_battery::POWER_OFF){
        if(initial || cause == ESP_SLEEP_WAKEUP_GPIO){
            UI.on_charge_screen();
            delay(50);
            set_display_brightness(brightness_before);

            delay(NOTIFICATION_PRESENSE_TIME);

            set_display_brightness(0);
            delay(50);
            UI.on_charge_screen(true);
            initial = false;
        }

        esp_sleep_enable_timer_wakeup(2000000);
        esp_light_sleep_start();
        cause = esp_sleep_get_wakeup_cause();
    }
    
    ESP.restart();
}

// ---------------------------------------------------------------

// ===============================================================================================