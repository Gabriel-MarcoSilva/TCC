#include "vibration.h"
#include "motors.h"
#include "led.h"
#include <string.h>
#include <stdlib.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "VIBRATION";

typedef struct {
    char padrao[32];
    bool m1;
    bool m2;
} vib_params_t;

static TaskHandle_t vib_task_handle = NULL;

static void vibration_task(void *param) {
    vib_params_t *p = (vib_params_t *)param;

    while (1) {
        for (int i = 0; p->padrao[i] != 0; i++) {
            uint8_t intensity = (p->padrao[i]=='1') ? 255 : 255;
            uint32_t duration = (p->padrao[i]=='1') ? 133 : 400;

            // LED
            led_set(1);

            // Motores
            motor_1_set(p->m1 ? intensity : 0);
            motor_2_set(p->m2 ? intensity : 0);

            vTaskDelay(pdMS_TO_TICKS(duration));

            // GAP
            motors_off();
            vTaskDelay(pdMS_TO_TICKS(133));
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    free(p);
    vTaskDelete(NULL);
}

void vibration_start(const char *padrao, bool m1, bool m2) {
    vibration_stop();

    vib_params_t *p = malloc(sizeof(vib_params_t));
    strcpy(p->padrao, padrao);
    p->m1 = m1;
    p->m2 = m2;

    xTaskCreate(vibration_task, "vib_task", 4096, p, 5, &vib_task_handle);
    ESP_LOGI(TAG, "Vibration iniciada.");
}

void vibration_stop() {
    if (vib_task_handle != NULL) {
        vTaskDelete(vib_task_handle);
        vib_task_handle = NULL;
        motors_off();
        ESP_LOGI(TAG, "Vibration parada.");
    }
}
