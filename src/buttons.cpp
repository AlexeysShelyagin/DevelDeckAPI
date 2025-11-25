#include "buttons.h"

#include "DevelDeckAPI.h"

//ISR definiton
IRAM_ATTR void handle_button_interrupt();
//

void Gamepad_buttons::init(){
    for (int i = 0; i < BUTTONS_N; i++){
        if (buttons_map[i] == -1) 
            continue;
        
        pinMode(buttons_map[i], INPUT);
        
        attachInterrupt(buttons_map[i], handle_button_interrupt, CHANGE);
        
    }
    Serial.println();
}

bool Gamepad_buttons::get_latest_state(uint8_t id){
    return ((latest_state >> id) & 1);                // extract last state of one specific button
}

bool Gamepad_buttons::read_state(uint8_t id){
    return digitalRead(buttons_map[id]) ^ INVERT_BUTTONS_STATE;
}

void Gamepad_buttons::add_button_event(uint8_t &state){
    previous_state = latest_state;

    latest_state = state;
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

IRAM_ATTR void handle_button_interrupt(){
    uint8_t state = 0;
    uint64_t current_time = millis();
    bool update = false;

    for (int i = 0; i < BUTTONS_N; i++){
        if (buttons_map[i] == -1)                                                                       // check button enabled
            continue;

        bool button_state = digitalRead(buttons_map[i]);
        if (gamepad.buttons.get_latest_state(i) != button_state){                                         // does any button really change its state?
            if (current_time - gamepad.buttons.last_event_time[i] < BUTTON_FILTERING_TIME)              // filter out bouncing
                continue;

            gamepad.buttons.last_event_time[i] = current_time;                                          // update last button event time
            update = true;                                                                              // mark that changes actually occured
        }

        state |= digitalRead(buttons_map[i]) << i;                                                      // write full keyboard state
    }

    if(!update)
        return;

    if(INVERT_BUTTONS_STATE)
        state = ~state;

    gamepad.buttons.add_button_event(state);
}