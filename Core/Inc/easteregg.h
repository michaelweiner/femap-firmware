#ifndef EASTEREGG_H
#define EASTEREGG_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_dac.h"
#include "stm32l4xx_hal_opamp.h"
#include "stm32l4xx_hal_tim.h"

void init_easteregg(TIM_HandleTypeDef *htim_dac, DAC_HandleTypeDef *hdac,
                    OPAMP_HandleTypeDef *hopamp);

/* Begin looping playback. Returns immediately; the clip repeats until
 * stop_easteregg() is called. */
void start_easteregg(void);
void stop_easteregg(void);

#endif /* EASTEREGG_H */
