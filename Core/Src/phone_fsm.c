#include "phone_fsm.h"

#include <stdio.h>
#include <string.h>

#include "bt_hfp.h"
#include "easteregg.h"
#include "main.h"
#include "ringer.h"
#include "rotary.h"
#include "tone.h"
#include "uart_debug.h"

enum phone_state_t
{
    PHONE_IDLE,
    PHONE_DIALTONE,
    PHONE_ACTIVE_CALL,
    PHONE_INCOMING_CALL,
    PHONE_AUDIO_DISCONNECTED_FROM_CALL,
    PHONE_KEIN_ANSCHLUSS,
    PHONE_ERROR
};

static enum phone_state_t phone_state = PHONE_IDLE;
static UART_HandleTypeDef *huart_at = NULL;

void phone_fsm_init(UART_HandleTypeDef *huart) { huart_at = huart; }

void phone_fsm_process(void)
{
    GPIO_PinState pin_gu = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3);
    bt_hfp_process();
    enum hfpstat_t hfp_state = bt_hfp_get_stat();
    enum hfpaudio_t audio_state = bt_hfp_get_audio();
    enum phone_state_t old_phone_state = phone_state;
    switch (phone_state)
    {
    case PHONE_IDLE:
        if (pin_gu == GPIO_PIN_SET)
        {
            if (hfp_state == HFPSTAT_CONNECTED)
            {
                start_dialtone(DIALTONE_DEFAULT);
                phone_state = PHONE_DIALTONE;
            }
            else if (hfp_state == HFPSTAT_OUTGOING_CALL ||
                     hfp_state == HFPSTAT_ACTIVE_CALL)
            {
                if (audio_state == HFPAUDIO_CONNECTED)
                {
                    HAL_GPIO_WritePin(VOICE_EN_GPIO_Port, VOICE_EN_Pin,
                                      GPIO_PIN_SET);
                    phone_state = PHONE_ACTIVE_CALL;
                }
                else
                {
                    phone_state = PHONE_AUDIO_DISCONNECTED_FROM_CALL;
                }
            }
            else
            {
                start_dialtone(DIALTONE_GASSENBESETZTTON);
                phone_state = PHONE_ERROR;
            }
        }
        else if (hfp_state == HFPSTAT_INCOMING_CALL)
        {
            start_bell();
            phone_state = PHONE_INCOMING_CALL;
        }
        else
        {
            if (bt_hfp_audio_changed())
            {
                HAL_GPIO_WritePin(HV_EN_GPIO_Port, HV_EN_Pin,
                                  audio_state == HFPAUDIO_CONNECTED
                                      ? GPIO_PIN_SET
                                      : GPIO_PIN_RESET);
            }
        }
        break;

    case PHONE_DIALTONE:
        if (pin_gu == GPIO_PIN_RESET)
        {
            stop_dialtone();
            phone_state = PHONE_IDLE;
        }
        else if (HAL_GPIO_ReadPin(nsa_GPIO_Port, nsa_Pin) == GPIO_PIN_SET)
        {
            size_t num_len;
            uint8_t number[100] = {0};
            char command[113];
            stop_dialtone();
            num_len = sizeof(number) - 1;

            read_rotary(number, &num_len);
            count_to_ascii(number, num_len);
            iprintf("dialed=%s\n", number);
            siprintf(command, "AT+HFPDIAL=%s\r\n", number);
            /* check again if Gabelumschalter is still active */
            pin_gu = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3);
            if (pin_gu == GPIO_PIN_SET && num_len > 0)
            {
                if (memcmp((const char *)number, "03023125", 8) == 0)
                {
                    start_easteregg();
                    phone_state = PHONE_KEIN_ANSCHLUSS;
                }
                else
                {
                    HAL_UART_Transmit(huart_at, (const uint8_t *)command,
                                      strlen(command), 100);
                    HAL_GPIO_WritePin(VOICE_EN_GPIO_Port, VOICE_EN_Pin,
                                      GPIO_PIN_SET);
                    phone_state = PHONE_ACTIVE_CALL;
                }
            }
            else
            {
                phone_state = PHONE_IDLE;
            }
        }
        else if (((hfp_state == HFPSTAT_OUTGOING_CALL) ||
                  (hfp_state == HFPSTAT_ACTIVE_CALL)) &&
                 (audio_state == HFPAUDIO_CONNECTED))
        {
            stop_dialtone();
            HAL_GPIO_WritePin(VOICE_EN_GPIO_Port, VOICE_EN_Pin, GPIO_PIN_SET);
            phone_state = PHONE_ACTIVE_CALL;
        }
        break;

    case PHONE_INCOMING_CALL:
        if ((hfp_state != HFPSTAT_INCOMING_CALL) &&
            (audio_state != HFPAUDIO_CONNECTED))
        {
            stop_bell();
            phone_state = PHONE_IDLE;
        }
        else if (pin_gu == GPIO_PIN_SET)
        {
            uint8_t command[] = "AT+HFPANSW\r\n";
            stop_bell();
            HAL_UART_Transmit(huart_at, command, strlen((const char *)command),
                              100);
            HAL_Delay(100);
            HAL_GPIO_WritePin(VOICE_EN_GPIO_Port, VOICE_EN_Pin, GPIO_PIN_SET);
            phone_state = PHONE_ACTIVE_CALL;
        }
        break;

    case PHONE_ACTIVE_CALL:
        if (pin_gu == GPIO_PIN_RESET)
        {
            uint8_t command[] = "AT+HFPCHUP\r\n";
            HAL_UART_Transmit(huart_at, command, strlen((const char *)command),
                              100);
            HAL_GPIO_WritePin(VOICE_EN_GPIO_Port, VOICE_EN_Pin, GPIO_PIN_RESET);
            phone_state = PHONE_IDLE;
        }
        else if (audio_state == HFPAUDIO_DISCONNECTED)
        {
            HAL_GPIO_WritePin(VOICE_EN_GPIO_Port, VOICE_EN_Pin, GPIO_PIN_RESET);
            phone_state = PHONE_AUDIO_DISCONNECTED_FROM_CALL;
        }
        else if (HAL_GPIO_ReadPin(nsa_GPIO_Port, nsa_Pin) == GPIO_PIN_SET)
        {
            size_t num_len = 1;
            uint8_t digit = 0;
            char command[15];

            if (read_rotary(&digit, &num_len) && (num_len == 1))
            {
                count_to_ascii(&digit, 1);
                iprintf("dtmf=%c\n", digit);
                siprintf(command, "AT+HFPDTMF=%c\r\n", digit);
                HAL_UART_Transmit(huart_at, (const uint8_t *)command,
                                  strlen(command), 100);
            }
        }
        break;

    case PHONE_AUDIO_DISCONNECTED_FROM_CALL:
        if ((audio_state == HFPAUDIO_CONNECTED) &&
            (hfp_state == HFPSTAT_ACTIVE_CALL || hfp_state == HFPSTAT_OUTGOING_CALL))
        {
            HAL_GPIO_WritePin(VOICE_EN_GPIO_Port, VOICE_EN_Pin, GPIO_PIN_SET);
            phone_state = PHONE_ACTIVE_CALL;
        }
        else if (pin_gu == GPIO_PIN_RESET)
        {
            phone_state = PHONE_IDLE;
        }
        break;

    case PHONE_KEIN_ANSCHLUSS:
        if (pin_gu == GPIO_PIN_RESET)
        {
            stop_easteregg();
            phone_state = PHONE_IDLE;
        }
        break;

    case PHONE_ERROR:
        if (pin_gu == GPIO_PIN_RESET)
        {
            stop_dialtone();
            phone_state = PHONE_IDLE;
        }
        break;

    default:
        break;
    }
    if (old_phone_state != phone_state)
    {
        iprintf("state=%u\n", phone_state);
    }

    if ((phone_state == PHONE_IDLE) && bt_hfp_uart_done() &&
        uart_debug_tx_done() && (audio_state == HFPAUDIO_DISCONNECTED))
    {
        puts("s");
        while (!bt_hfp_uart_done() || !uart_debug_tx_done())
        {
        }
        SET_BIT(RCC->CFGR, RCC_CFGR_STOPWUCK);
        HAL_UARTEx_EnableStopMode(huart_at);
        HAL_SuspendTick();
        HAL_PWREx_EnterSTOP0Mode(PWR_STOPENTRY_WFI);
        HAL_ResumeTick();
        HAL_UARTEx_DisableStopMode(huart_at);
        puts("w");
    }
}
