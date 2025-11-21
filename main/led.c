#include "led.h"
#include <esp_log.h>

static const char* TAG = "LED";

void led_init() {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);
    ESP_LOGI(TAG, "LED inicializado");
}

void led_set(int level) {
    gpio_set_level(LED_PIN, level);
}
