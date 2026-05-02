#ifndef GAMEPAD_TACKTILE_RESPONSE_H
#define GAMEPAD_TACKTILE_RESPONSE_H

#include <Arduino.h>
#include <vector>

#include "config.h"

struct Buzzer_element_t{
    uint16_t freq;
    uint16_t timing;
};

class Gamepad_buzzer{
    uint8_t channel;
    uint8_t volume;
    uint8_t volume_level = DEFAULT_BUZZER_VOLUME;

    TaskHandle_t task_handler = NULL;
    void *task_params;
public:

    Gamepad_buzzer() = default;

    void init(uint16_t pin = BUZZ_PIN, uint8_t channel_ = BUZZ_LEDC_CHANNEL);

    void play_tone(uint16_t freq);
    void stop();

    void change_volume(uint8_t level);
    uint8_t get_volume();

    void play_for_time(uint16_t freq, uint16_t time);
    void play_sequence(std::vector < Buzzer_element_t > &sequence);
    void play_sequence(uint16_t *data, uint32_t size, bool nocopy = false);
};



class Gamepad_vibro{
    uint8_t channel;

    TaskHandle_t task_handler = NULL;
    void *task_params;

    uint8_t calc_strength(uint8_t strength_);
public:
    uint8_t strength = DEFAULT_VIBRO_STRENGTH;

    Gamepad_vibro() = default;

    void init(uint16_t pin = VIBRO_PIN, uint8_t channel_ = VIBRO_LEDC_CHANNEL);

    void enable(uint8_t strength_ = 255);
    void disable();

    void enable_for_time(uint16_t time, uint8_t strength_ = 255);
    void enable_periodic(uint16_t time_enabled, uint16_t time_disabled, uint8_t repeat_times, uint8_t strength_ = 255);
};

#endif