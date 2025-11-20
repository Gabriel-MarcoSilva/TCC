#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define MOTOR_PIN 18  // pino PWM conectado ao transistor
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_FREQUENCY 5000 // Hz
#define LEDC_RESOLUTION LEDC_TIMER_8_BIT // 8 bits (0-255)

void app_main(void)
{
    // Configura o timer do PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_RESOLUTION,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Configura o canal PWM
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = MOTOR_PIN,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0, // inicial: desligado
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);

    printf("Controle de intensidade do motor vibracall iniciado!\n");

    int intensidade = 0;

    while (1)
    {
        printf("1024\n");
        intensidade = 1023; // meio termo
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, intensidade);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 2 segundos

        intensidade = 0; // meio termo
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, intensidade);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 2 segundos

        printf("512\n");
        intensidade = 511; // meio termo
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, intensidade);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 2 segundos

        intensidade = 0; // meio termo
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, intensidade);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 2 segundos

        printf("256\n");
        intensidade = 255; // meio termo
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, intensidade);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 2 segundos

        intensidade = 0; // meio termo
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, intensidade);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 2 segundos

        printf("128\n");
        intensidade = 127; // meio termo
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, intensidade);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 2 segundos

        intensidade = 0; // meio termo
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, intensidade);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 2 segundos

    }
}
