#include "buttons.h"

#include "DevelDeckAPI.h"

//ISR definiton
IRAM_ATTR void handle_button_interrupt(void *args);

struct ISR_args_t{
    Gamepad_buttons *buttons;
    int16_t pin_id;
    gpio_num_t target_pin;
};
ISR_args_t args_container[BUTTONS_N];

void Gamepad_buttons::init(){
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

    for (int16_t i = 0; i < BUTTONS_N; i++){
        if (buttons_map[i] == -1) 
            continue;
        
        gpio_num_t pin = (gpio_num_t) buttons_map[i];

        gpio_config_t pin_config = {};
        pin_config.pin_bit_mask = (1ULL << pin);
        pin_config.mode = GPIO_MODE_INPUT;
        pin_config.pull_up_en = GPIO_PULLUP_DISABLE;
        pin_config.intr_type = GPIO_INTR_ANYEDGE;
        gpio_config(&pin_config);

        args_container[i] = {this, i, pin};

        gpio_isr_handler_add(pin, handle_button_interrupt, &args_container[i]);
    }

    uint8_t init_state = 0;
    for(uint8_t i = 0; i < BUTTONS_N; i++){
        init_state |= read_state(i) << i;
    }
    if(INVERT_BUTTONS_STATE)
        init_state = ~init_state;
    add_button_event(init_state);
}

bool Gamepad_buttons::get_latest_state(uint8_t id){
    return GAMEPAD_GLOBAL::get_latest_button_state(id);
}

bool Gamepad_buttons::read_state(uint8_t id){
    return digitalRead(buttons_map[id]) ^ INVERT_BUTTONS_STATE;
}

void Gamepad_buttons::add_button_event(uint8_t &state){
    previous_state = GAMEPAD_GLOBAL::latest_buttons_state;

    GAMEPAD_GLOBAL::latest_buttons_state = state;
    button_buff.push(state);
}

uint8_t* Gamepad_buttons::get_button_event(){
    if(button_buff.empty())
        return nullptr;
    
    uint8_t event = button_buff.front();
    static uint8_t response[BUTTONS_N];
    for(int i = 0; i < BUTTONS_N; i++){
        bool button = (event >> i) & 1;
        bool prev = (previous_state >> i) & 1;
        
        if(button != prev)
            response[i] = (button) ? BUT_PRESSED : BUT_RELEASED;
        else
            response[i] = (button) ? BUT_STILL_PRESSED : BUT_STILL_RELEASED;
    }

    button_buff.pop();

    return response;
}

bool Gamepad_buttons::event_available(){
    return !button_buff.empty();
}

void Gamepad_buttons::clear_queue(){
    uint16_t size = button_buff.size();
    for(int i = 0; i < size; i++)
        button_buff.pop();
}



// ----------- BUTTON_ISR -------------

IRAM_ATTR void handle_button_interrupt(void *args){
    Gamepad_buttons *buttons = ((ISR_args_t *) args)->buttons;
    int16_t id = ((ISR_args_t *) args)->pin_id;
    gpio_num_t pin = ((ISR_args_t *) args)->target_pin;
    
    if(id == -1)
        return;

    uint8_t pin_state = gpio_get_level(pin);
    if(INVERT_BUTTONS_STATE)
        pin_state = !pin_state;

    if(GAMEPAD_GLOBAL::get_latest_button_state(id) == pin_state)        // filter out false interrupts
        return;
    
    uint64_t now = millis();
    if (now - buttons->last_event_time[id] < BUTTON_FILTERING_TIME)     // filter out bouncing
        return;
    buttons->last_event_time[id] = now;                                 // update last button event time

    uint8_t new_state = ( GAMEPAD_GLOBAL::latest_buttons_state & ~(1<<id) ) | ( pin_state<<id );    // change state bit
    buttons->add_button_event(new_state);
}


// ------------ GLOBAL ----------------

namespace GAMEPAD_GLOBAL{
    uint8_t latest_buttons_state;
}

bool GAMEPAD_GLOBAL::get_latest_button_state(uint8_t id){
    return ((GAMEPAD_GLOBAL::latest_buttons_state >> id) & 1);                // extract last state of one specific button
}

void GAMEPAD_GLOBAL::stop_button_interrupts(){
    for (int i = 0; i < BUTTONS_N; i++)
        gpio_intr_disable((gpio_num_t) buttons_map[i]);
}

void GAMEPAD_GLOBAL::resume_button_interrupts(){
    for (int i = 0; i < BUTTONS_N; i++)
        gpio_intr_enable((gpio_num_t) buttons_map[i]);
}