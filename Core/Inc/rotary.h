#ifndef ROTARY_H
#define ROTARY_H

#include "stm32l4xx_hal.h"
#include <stddef.h>

void init_rotary(TIM_HandleTypeDef *htim);
int read_rotary(uint8_t *pau8Digit, size_t *len);
void count_to_ascii(uint8_t *pau8, size_t size);

#endif /* ROTARY_H */
