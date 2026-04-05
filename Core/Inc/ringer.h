#ifndef RINGER_H
#define RINGER_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_tim.h"

void init_bell(TIM_HandleTypeDef *htim_dutycycle, TIM_HandleTypeDef *htim_pmos,
               TIM_HandleTypeDef *htim_nmos);
void start_bell(void);
void stop_bell(void);

#endif /* RINGER_H */
