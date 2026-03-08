#ifndef GAMEPAD_TACKTILE_RESPONSE_H
#define GAMEPAD_TACKTILE_RESPONSE_H

#include <Arduino.h>
#include <vector>

#include "config.h"


/**
 * @brief Element of buzzer sequence
 * 
 */
struct Buzzer_element_t{
    uint16_t freq;
    uint16_t timing;
};

class Gamepad_buzzer{
    uint8_t channel;
    uint8_t volume;
    uint8_t volume_level = DEFAULT_BUZZER_VOLUME;

    TaskHandle_t buzz_task_handler = NULL;
public:

    Gamepad_buzzer() = default;

    void init(uint16_t pin = BUZZ_PIN, uint8_t channel_ = BUZZ_LEDC_CHANNEL);


    /**
     * @brief Start playing tone on buzzer
     * 
     * @param freq frequency of sound
     */
    void play_tone(uint16_t freq);

    /**
     * @brief Stop playing any sound
     * 
     */
    void stop();



    /**
     * @brief Change buzzer volume
     * 
     * @param level Volume level in range from 0 to BUZZER_VOLUME_LEVELS
     */
    void change_volume(uint8_t level);

    /**
     * @brief Get current buzzer volume
     * 
     * @return uint8_t 
     */
    uint8_t get_volume();



    /**
     * @brief Play one frequency for some period of time
     * 
     * @param freq (Hz) frequency of sound
     * @param time (ms) duration of sound
     */
    void play_for_time(uint16_t freq, uint16_t time);

    /**
     * @brief Play tone sequence. Each tone play for specified duration one by one
     * 
     * @param sequence `std::vector` filled with `Gamepad_buzzer::sequence_element` sound data
     */
    void play_sequence(std::vector < Buzzer_element_t > sequence);

    /**
     * @brief Play tone sequence. Each tone play for specified duration one by one
     * 
     * @param data pointer to the `uint16_t` array with tone data in a format [freq1 (Hz), time1 (ms), freq2, time2, ...]
     * @param size number of tones in the sequence
     */
    void play_sequence(uint16_t *data, uint32_t size);
};



class Gamepad_vibro{
    uint8_t channel;

    TaskHandle_t vib_task_handler = NULL;
    uint8_t calc_strength(uint8_t strength_);
public:
    uint8_t strength = DEFAULT_VIBRO_STRENGTH;

    Gamepad_vibro() = default;

    void init(uint16_t pin = VIBRO_PIN, uint8_t channel_ = VIBRO_LEDC_CHANNEL);


    /**
     * @brief Turn vibro on
     * 
     * @param strength_ vibro strength from 0 to 255
     */
    void enable(uint8_t strength_ = 255);

    /**
     * @brief Disable any vibration
     * 
     */
    void disable();


    /**
     * @brief Turn vibro on for a period of time
     * 
     * @param time (ms) vibration duration
     * @param strength_ vibro strength from 0 to 255
     */
    void enable_for_time(uint16_t time, uint8_t strength_ = 255);

    /**
     * @brief Vibrate multiple times with delay between
     * 
     * @param time_enabled (ms) vibration duration
     * @param time_disabled (ms) time delay between vibrations
     * @param repeat_times number of vibrations
     * @param strength_ vibro strength from 0 to 255
     */
    void enable_periodic(uint16_t time_enabled, uint16_t time_disabled, uint8_t repeat_times, uint8_t strength_ = 255);
};

#endif