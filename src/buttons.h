#ifndef DD_BUTTONS_H
#define DD_BUTTONS_H

#include <queue>
#include <Arduino.h>

#include "config.h"

enum Button_event_type_t : uint8_t{
    BUT_NONE = 0,
    BUT_PRESSED,
    BUT_RELEASED,
    BUT_STILL_PRESSED,
    BUT_STILL_RELEASED
};

typedef uint8_t* But_events_t;



class DD_buttons{
    std::queue < uint8_t > events;

    uint8_t previous_state = DD_BUTTONS_INV * 0xFF;
public:
    uint64_t last_event_time[DD_BUTTONS_N];

    DD_buttons() = default;

    void init();


    /**
     * @brief Get latest state of the specific button stored by system
     * 
     * @param id button id
     * @return true: pressed
     * @return false: unpressed
     */
    bool get_latest_state(uint8_t id);

    /**
     * @brief Get current state of the specific button
     * 
     * @param id button id
     * @return true 
     * @return false 
     */
    bool read_state(uint8_t id);



    /**
     * @brief Add button event artificially
     * 
     * @param state new state raw data
     */
    void add_event(uint8_t &state);

    /**
     * @brief Get buttons data for the next event in queue. After call the event is considered as handled
     * 
     * @return But_events_t: array of `DD_BUTTONS_N` elements with state of each button according to its id
     */
    But_events_t get_event();

    /**
     * @brief Checks are there any unhandled buttons events
     * 
     * @return true: unhandled events are present
     * @return false: no new events
     */
    bool event_available();

    /**
     * @brief Forced button event queue clear
     * 
     */
    void clear_queue();
};


namespace DD_GLOBAL{
    bool get_latest_button_state(uint8_t id);
    
    void stop_button_interrupts();
    void resume_button_interrupts();
}

#endif