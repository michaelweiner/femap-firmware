#ifndef BT_HFP_H
#define BT_HFP_H

#include "stm32l4xx_hal.h"

enum hfpstat_t
{
    HFPSTAT_UNSUPPORTED = 0,
    HFPSTAT_STANDBY = 1,
    HFPSTAT_CONNECTING = 2,
    HFPSTAT_CONNECTED = 3,
    HFPSTAT_OUTGOING_CALL = 4,
    HFPSTAT_INCOMING_CALL = 5,
    HFPSTAT_ACTIVE_CALL = 6,
};
enum hfpaudio_t
{
    HFPAUDIO_DISCONNECTED = 0,
    HFPAUDIO_CONNECTED = 1,
};

void bt_hfp_init(UART_HandleTypeDef *huart_at, UART_HandleTypeDef *huart_relay);
void bt_hfp_process(void);
int bt_hfp_uart_done(void);
enum hfpstat_t bt_hfp_get_stat(void);
enum hfpaudio_t bt_hfp_get_audio(void);
int bt_hfp_audio_changed(void);

#endif /* BT_HFP_H */
