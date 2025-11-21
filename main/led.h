#ifndef LED_H
#define LED_H

#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_2

void led_init(void);
void led_set(int level);

#endif
