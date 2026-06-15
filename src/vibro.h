#ifndef DD_VIBRO_H
#define DD_VIBRO_H

#include <Arduino.h>

#include "config.h"

class DD_vibro{
    uint8_t ledc_ch;

    TaskHandle_t task_handler = NULL;
    void *task_params;

    uint8_t calc_strength(uint8_t strength_);
public:
    uint8_t strength = DEFAULT_VIBRO_STRENGTH;

    DD_vibro() = default;

    void init(uint16_t pin = DD_VIBRO_PIN, uint8_t channel_ = DD_VIBRO_LEDC_CHANNEL);


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
     * @brief Vibrate for a period of time
     * 
     * @param time (ms) vibration duration
     * @param strength_ vibro strength from 0 to 255
     */
    void pulse(uint16_t time, uint8_t strength_ = 255);

    /**
     * @brief Vibrate multiple times with delay between
     * 
     * @param time_enabled (ms) vibration duration
     * @param time_disabled (ms) time delay between vibrations
     * @param repeat_times number of vibrations
     * @param strength_ vibro strength from 0 to 255
     */
    void multipulse(uint16_t time_enabled, uint16_t time_disabled, uint8_t repeat_times, uint8_t strength_ = 255);
};

#endif