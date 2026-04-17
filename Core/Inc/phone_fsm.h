#ifndef PHONE_FSM_H
#define PHONE_FSM_H

#include "stm32l4xx_hal.h"

void phone_fsm_init(UART_HandleTypeDef *huart_at);
void phone_fsm_process(void);

#endif /* PHONE_FSM_H */
