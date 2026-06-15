#include "buttons.h"

#include "DevelDeckAPI.h"

//ISR definiton
IRAM_ATTR void buttons_isr(void *args);

struct ISR_args_t{
    DD_buttons *buttons;
    int16_t pin_id;
    gpio_num_t target_pin;
};
ISR_args_t args_container[BUTTONS_N];

uint8_t latest_buttons_state;



void DD_buttons::init(){
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

        gpio_isr_handler_add(pin, buttons_isr, &args_container[i]);
    }

    uint8_t init_state = 0;
    for(uint8_t i = 0; i < BUTTONS_N; i++){
        init_state |= read_state(i) << i;
    }
    if(INVERT_BUTTONS_STATE)
        init_state = ~init_state;
    add_event(init_state);
}

bool DD_buttons::get_latest_state(uint8_t id){
    return DD_GLOBAL::get_latest_button_state(id);
}

bool DD_buttons::read_state(uint8_t id){
    return digitalRead(buttons_map[id]) ^ INVERT_BUTTONS_STATE;
}

void DD_buttons::add_event(uint8_t &state){
    previous_state = latest_buttons_state;

    latest_buttons_state = state;
    events.push(state);
}

But_events_t DD_buttons::get_event(){
    if(events.empty())
        return nullptr;
    
    uint8_t event = events.front();
    static uint8_t response[BUTTONS_N];
    for(int i = 0; i < BUTTONS_N; i++){
        bool button = (event >> i) & 1;
        bool prev = (previous_state >> i) & 1;
        
        if(button != prev)
            response[i] = (button) ? BUT_PRESSED : BUT_RELEASED;
        else
            response[i] = (button) ? BUT_STILL_PRESSED : BUT_STILL_RELEASED;
    }

    events.pop();

    return response;
}

bool DD_buttons::event_available(){
    return !events.empty();
}

void DD_buttons::clear_queue(){
    uint16_t size = events.size();
    for(int i = 0; i < size; i++)
        events.pop();
}



// ----------- BUTTON_ISR -------------

IRAM_ATTR void buttons_isr(void *args){
    DD_buttons *buttons = ((ISR_args_t *) args)->buttons;
    int16_t id = ((ISR_args_t *) args)->pin_id;
    gpio_num_t pin = ((ISR_args_t *) args)->target_pin;
    
    if(id == -1)
        return;

    uint8_t pin_state = gpio_get_level(pin);
    if(INVERT_BUTTONS_STATE)
        pin_state = !pin_state;

    if(DD_GLOBAL::get_latest_button_state(id) == pin_state)        // filter out false interrupts
        return;
    
    uint64_t now = millis();
    if (now - buttons->last_event_time[id] < BUTTON_FILTERING_TIME)     // filter out bouncing
        return;
    buttons->last_event_time[id] = now;                                 // update last button event time

    uint8_t new_state = ( latest_buttons_state & ~(1<<id) ) | ( pin_state<<id );    // change state bit
    buttons->add_event(new_state);
}


// ------------ GLOBAL ----------------

bool DD_GLOBAL::get_latest_button_state(uint8_t id){
    return ((latest_buttons_state >> id) & 1);                // extract last state of one specific button
}

void DD_GLOBAL::stop_button_interrupts(){
    for (int i = 0; i < BUTTONS_N; i++)
        gpio_isr_handler_remove((gpio_num_t) buttons_map[i]);
}

void DD_GLOBAL::resume_button_interrupts(){
    ddeck.buttons.init();
}