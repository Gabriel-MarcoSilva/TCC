#include "motors.h"
#include "led.h"
#include <esp_log.h>

static const char* TAG = "MOTORS";

#define MOTOR_1_LEDC_CHANNEL LEDC_CHANNEL_0
#define MOTOR_2_LEDC_CHANNEL LEDC_CHANNEL_1
#define MOTOR_LEDC_TIMER     LEDC_TIMER_0
#define MOTOR_LEDC_MODE      LEDC_LOW_SPEED_MODE
#define MOTOR_LEDC_FREQ_HZ   5000
#define MOTOR_LEDC_RES       LEDC_TIMER_8_BIT

void motors_init() {
    ledc_timer_config_t timer_conf = {
        .speed_mode       = MOTOR_LEDC_MODE,
        .duty_resolution  = MOTOR_LEDC_RES,
        .timer_num        = MOTOR_LEDC_TIMER,
        .freq_hz          = MOTOR_LEDC_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t ch1 = {
        .gpio_num   = MOTOR_PIN_1,
        .speed_mode = MOTOR_LEDC_MODE,
        .channel    = MOTOR_1_LEDC_CHANNEL,
        .timer_sel  = MOTOR_LEDC_TIMER,
        .duty       = 0
    };
    ledc_channel_config(&ch1);

    ledc_channel_config_t ch2 = {
        .gpio_num   = MOTOR_PIN_2,
        .speed_mode = MOTOR_LEDC_MODE,
        .channel    = MOTOR_2_LEDC_CHANNEL,
        .timer_sel  = MOTOR_LEDC_TIMER,
        .duty       = 0
    };
    ledc_channel_config(&ch2);

    ESP_LOGI(TAG,"Motores inicializados.");
}

void motor_1_set(uint8_t i) {
    ledc_set_duty(MOTOR_LEDC_MODE, MOTOR_1_LEDC_CHANNEL, i);
    ledc_update_duty(MOTOR_LEDC_MODE, MOTOR_1_LEDC_CHANNEL);
}

void motor_2_set(uint8_t i) {
    ledc_set_duty(MOTOR_LEDC_MODE, MOTOR_2_LEDC_CHANNEL, i);
    ledc_update_duty(MOTOR_LEDC_MODE, MOTOR_2_LEDC_CHANNEL);
}

void motors_off() {
    motor_1_set(0);
    motor_2_set(0);
    led_set(0);
    ESP_LOGI(TAG, "Motores desligados.");
}
