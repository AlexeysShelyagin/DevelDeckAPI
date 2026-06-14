#ifndef DD_VIBRO_H
#define DD_VIBRO_H

#include <Arduino.h>

#include "config.h"

class DD_vibro{
    uint8_t channel;

    TaskHandle_t task_handler = NULL;
    void *task_params;

    uint8_t calc_strength(uint8_t strength_);
public:
    uint8_t strength = DEFAULT_VIBRO_STRENGTH;

    DD_vibro() = default;

    void init(uint16_t pin = VIBRO_PIN, uint8_t channel_ = VIBRO_LEDC_CHANNEL);

    void enable(uint8_t strength_ = 255);
    void disable();

    void enable_for_time(uint16_t time, uint8_t strength_ = 255);
    void enable_periodic(uint16_t time_enabled, uint16_t time_disabled, uint8_t repeat_times, uint8_t strength_ = 255);
};

#endif