#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>
#include "driver/ledc.h"

#define MOTOR_PIN_1 GPIO_NUM_18
#define MOTOR_PIN_2 GPIO_NUM_19

void motors_init(void);
void motor_1_set(uint8_t intensity);
void motor_2_set(uint8_t intensity);
void motors_off(void);

#endif
