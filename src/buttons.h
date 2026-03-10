#ifndef GAMEPAD_BUTTONS_H
#define GAMEPAD_BUTTONS_H

#include <queue>
#include <Arduino.h>

#include "config.h"

enum Button_state_t{
    BUT_NONE,
    BUT_PRESSED,
    BUT_RELEASED,
    BUT_STILL_PRESSED,
    BUT_STILL_RELEASED
};

class Gamepad_buttons{
    std::queue < uint8_t > button_buff;

    uint8_t latest_state, previous_state = INVERT_BUTTONS_STATE * -1;

public:
    uint64_t last_event_time[BUTTONS_N];

    Gamepad_buttons() = default;

    void init();

    bool get_latest_state(uint8_t id);
    bool read_state(uint8_t id);

    void add_button_event(uint8_t &state);
    uint8_t* get_button_event();
    bool event_available();
    void clear_queue();
};

#endif