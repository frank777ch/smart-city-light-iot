#ifndef SIMPLEPWM_H
#define SIMPLEPWM_H

#include <Arduino.h>
#include "driver/ledc.h"

class SimplePWM {
public:
    gpio_num_t pin;
    ledc_channel_t channel;
    int freq;
    int resolution;
    int maxDuty;

    void begin(int pin, int channel, int freq, int resolution) {
        this->pin = (gpio_num_t)pin;
        this->channel = (ledc_channel_t)channel;
        this->freq = freq;
        this->resolution = resolution;
        this->maxDuty = (1 << resolution) - 1;

        ledc_timer_config_t timer_conf = {
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .duty_resolution = (ledc_timer_bit_t)resolution,
            .timer_num = LEDC_TIMER_0,
            .freq_hz = (uint32_t)freq,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ledc_timer_config(&timer_conf);

        ledc_channel_config_t ch_conf = {
            .gpio_num = (int)pin,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .channel = (ledc_channel_t)channel,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            .hpoint = 0
        };
        ledc_channel_config(&ch_conf);
    }

    int readDuty() {
        return ledc_get_duty(LEDC_HIGH_SPEED_MODE, channel);
    }

    void writeDuty(int duty) {
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
    }

    void fadePercent(float percent, int timeMs) {
        percent = constrain(percent, 0.0f, 1.0f);
        int targetDuty = percent * maxDuty;
        int currentDuty = readDuty();

        int steps = 50;
        int delayStep = timeMs / steps;
        float delta = (float)(targetDuty - currentDuty) / steps;

        for (int i = 0; i <= steps; i++) {
            writeDuty(currentDuty + (delta * i));
            delay(delayStep);
        }
    }
};

#endif