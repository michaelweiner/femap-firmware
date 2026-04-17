#include "bt_hfp.h"
#include "main.h"
#include "uart_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UART_BUFFER_SIZE 256

static volatile uint8_t uart_buffer[UART_BUFFER_SIZE];
static volatile uint8_t uart_response[UART_BUFFER_SIZE];
static volatile uint8_t uart_response_ready = 0;
static volatile size_t uart_buffer_pos = 0;
static uint8_t dummy_buffer[10];
static uint8_t uart_response_nv[UART_BUFFER_SIZE];

static enum hfpstat_t hfpstat = HFPSTAT_UNSUPPORTED;
static enum hfpaudio_t hfpaudio = HFPAUDIO_DISCONNECTED;
static int audio_changed = 0;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                     */
/* ------------------------------------------------------------------ */

static size_t strcpy_v(uint8_t *psz_dest, const volatile uint8_t *psz_src,
                       size_t max)
{
    size_t ui = 0;
    while (ui < max)
    {
        psz_dest[ui] = psz_src[ui];
        if (psz_src[ui] == '\0')
        {
            break;
        }
        ++ui;
    }
    if (ui == max)
    {
        psz_dest[0] = '\0';
        ui = 0;
    }
    return ui;
}

/* ------------------------------------------------------------------ */
/* ISR callbacks                                                        */
/* ------------------------------------------------------------------ */

static void UART_RxISR_AT(UART_HandleTypeDef *huart)
{
    uint16_t uhdata;
    uint8_t uart_byte;

    uhdata = (uint16_t)READ_REG(huart->Instance->RDR);
    uart_byte = (uint8_t)(uhdata & 0xff);

    if (uart_byte == '\r')
    {
        /* do nothing */
    }
    else if (uart_byte == '\n')
    {
        if (uart_buffer_pos > 0)
        {
            for (size_t i = 0; i < uart_buffer_pos; ++i)
            {
                uart_response[i] = uart_buffer[i];
            }
            uart_response[uart_buffer_pos] = '\0';
            uart_response_ready = 1;
        }
        uart_buffer_pos = 0;
    }
    else if (uart_buffer_pos < sizeof(uart_buffer))
    {
        uart_buffer[uart_buffer_pos++] = uart_byte;
    }
    else /* overflow, disable interrupt */
    {
        ATOMIC_CLEAR_BIT(huart->Instance->CR1,
                         (USART_CR1_RXNEIE | USART_CR1_PEIE));
    }
}

static void UART_RxISR_Relay(UART_HandleTypeDef *huart)
{
    uint16_t uhdata = (uint16_t)READ_REG(huart->Instance->RDR);
    uart_debug_relay_to_bt((uint8_t)(uhdata & 0xff));
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

void bt_hfp_init(UART_HandleTypeDef *huart_at, UART_HandleTypeDef *huart_relay)
{
    HAL_GPIO_WritePin(BT_RESET_GPIO_Port, BT_RESET_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BT_EN_GPIO_Port, BT_EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(2000);
    HAL_GPIO_WritePin(BT_RESET_GPIO_Port, BT_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(2000);
    HAL_GPIO_WritePin(BT_EN_GPIO_Port, BT_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(2000);
    HAL_GPIO_WritePin(BT_EN_GPIO_Port, BT_EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(200);

    HAL_UART_Receive_IT(huart_relay, dummy_buffer, sizeof(dummy_buffer));
    huart_relay->RxISR = UART_RxISR_Relay;

    HAL_UART_Receive_IT(huart_at, dummy_buffer, sizeof(dummy_buffer));
    huart_at->RxISR = UART_RxISR_AT;

    while (uart_response_ready == 1)
        ;
    uart_response_ready = 0;
}

void bt_hfp_process(void)
{
    audio_changed = 0;
    if (uart_response_ready != 1)
    {
        return;
    }
    uart_response_ready = 0;

    size_t response_len =
        strcpy_v(uart_response_nv, uart_response, UART_BUFFER_SIZE);
    response_len = (response_len < UART_BUFFER_SIZE) ? response_len : 0;
    printf("recv='%s' (%u)\n", uart_response_nv, response_len);

    if ((response_len > 9) && (!memcmp(uart_response_nv, "+HFPSTAT=", 9)))
    {
        hfpstat = atoi((char *)&uart_response_nv[9]);
    }
    else if ((response_len > 10) &&
             (!memcmp(uart_response_nv, "+HFPAUDIO=", 10)))
    {
        enum hfpaudio_t new_audio = atoi((char *)&uart_response_nv[10]);
        if (new_audio != hfpaudio)
        {
            hfpaudio = new_audio;
            audio_changed = 1;
        }
    }
    printf("s=%u,a=%u\r\n", hfpstat, hfpaudio);
}

int bt_hfp_uart_done(void)
{
    return (uart_buffer_pos == 0) && (uart_response_ready == 0);
}

enum hfpstat_t bt_hfp_get_stat(void) { return hfpstat; }

enum hfpaudio_t bt_hfp_get_audio(void) { return hfpaudio; }

int bt_hfp_audio_changed(void) { return audio_changed; }
