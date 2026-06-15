#ifndef DD_BUZZER_H
#define DD_BUZZER_H

#include <Arduino.h>
#include <vector>

#include "config.h"

/**
 * @brief Element of buzzer sequence
 * 
 */
struct Buzz_tone_t{
    uint16_t freq;
    uint16_t duration;
};

class DD_buzzer{
    uint8_t ledc_ch;
    uint8_t volume;
    uint8_t volume_level = DEFAULT_BUZZER_VOLUME;

    TaskHandle_t task_handler = NULL;
    void *current_seq;
public:

    DD_buzzer() = default;

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
     * @param sequence `std::vector` filled with `DD_buzzer::Buzz_tone_t` sound data
     */
    void play_sequence(std::vector < Buzz_tone_t > &sequence);

    /**
     * @brief Play tone sequence. Each tone play for specified duration one by one
     * 
     * @param data pointer to the `uint16_t` array with tone data in a format [freq1 (Hz), time1 (ms), freq2, time2, ...]
     * @param size number of tones in the sequence
     */
    void play_sequence(uint16_t *data, uint32_t size, bool nocopy = false);
};

#endif