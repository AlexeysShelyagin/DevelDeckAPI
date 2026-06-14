#ifndef DD_BUZZER_H
#define DD_BUZZER_H

#include <Arduino.h>
#include <vector>

#include "config.h"

struct Buzz_tone_t{
    uint16_t freq;
    uint16_t duration;
};

class DD_buzzer{
    uint8_t channel;
    uint8_t volume;
    uint8_t volume_level = DEFAULT_BUZZER_VOLUME;

    TaskHandle_t task_handler = NULL;
    void *current_seq;
public:

    DD_buzzer() = default;

    void init(uint16_t pin = BUZZ_PIN, uint8_t channel_ = BUZZ_LEDC_CHANNEL);

    void play_tone(uint16_t freq);
    void stop();

    void change_volume(uint8_t level);
    uint8_t get_volume();

    void play_for_time(uint16_t freq, uint16_t time);
    void play_sequence(std::vector < Buzz_tone_t > &sequence);
    void play_sequence(uint16_t *data, uint32_t size, bool nocopy = false);
};

#endif