#ifndef TONE_H
#define TONE_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx_hal_dac.h"
#include "stm32l4xx_hal_opamp.h"

enum dialtone_t { DIALTONE_DEFAULT, DIALTONE_GASSENBESETZTTON };
void init_dialtone(TIM_HandleTypeDef *htim_dutycycle, TIM_HandleTypeDef *htim_dac, DAC_HandleTypeDef *hdac, OPAMP_HandleTypeDef *hopamp);
void start_dialtone(enum dialtone_t type);
void stop_dialtone(void);
#endif /* TONE_H */
