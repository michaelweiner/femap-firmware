#ifndef UART_DEBUG_H
#define UART_DEBUG_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

void uart_debug_init(UART_HandleTypeDef *huart_debug,
                     UART_HandleTypeDef *huart_relay_target);
void uart_debug_relay_to_bt(uint8_t c);
int uart_debug_tx_done(void);

#endif /* UART_DEBUG_H */
