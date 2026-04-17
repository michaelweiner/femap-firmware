#include "uart_debug.h"

#define UART_TX_FIFO_SIZE 100

struct uart_tx_fifo
{
    UART_HandleTypeDef *huart;
    uint8_t buf[UART_TX_FIFO_SIZE];
    size_t inpos;
    size_t outpos;
};

static struct uart_tx_fifo fifo_debug;
static struct uart_tx_fifo fifo_relay;

static void fifo_drain_isr(struct uart_tx_fifo *fifo, UART_HandleTypeDef *huart)
{
    if (fifo->inpos == fifo->outpos)
    {
        /* no more data — end transfer */
        ATOMIC_CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
        ATOMIC_SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);
    }
    else
    {
        huart->Instance->TDR = fifo->buf[fifo->outpos];
        fifo->outpos = (fifo->outpos + 1) % UART_TX_FIFO_SIZE;
    }
}

static void uart_debug_tx_isr(UART_HandleTypeDef *huart)
{
    fifo_drain_isr(&fifo_debug, huart);
}

static void uart_relay_tx_isr(UART_HandleTypeDef *huart)
{
    fifo_drain_isr(&fifo_relay, huart);
}

static void fifo_enqueue(struct uart_tx_fifo *fifo,
                         void (*tx_isr)(UART_HandleTypeDef *), uint8_t c)
{
    UART_HandleTypeDef *huart = fifo->huart;
    if ((fifo->outpos == fifo->inpos) && (huart->Instance->ISR & USART_ISR_TXE))
    {
        huart->Instance->TDR = c;
    }
    else
    {
        size_t used = (fifo->inpos + UART_TX_FIFO_SIZE - fifo->outpos) %
                      UART_TX_FIFO_SIZE;
        if (used < UART_TX_FIFO_SIZE - 1)
        {
            fifo->buf[fifo->inpos] = c;
            fifo->inpos = (fifo->inpos + 1) % UART_TX_FIFO_SIZE;
            huart->TxISR = tx_isr;
            ATOMIC_SET_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
        }
    }
}

/* ------------------------------------------------------------------ */
/* stdio hook — external linkage so newlib finds it by symbol name     */
/* ------------------------------------------------------------------ */

int __io_putchar(int ch)
{
    HAL_NVIC_DisableIRQ(LPUART1_IRQn);
    fifo_enqueue(&fifo_debug, uart_debug_tx_isr, ch & 0xff);
    HAL_NVIC_EnableIRQ(LPUART1_IRQn);
    return ch;
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

void uart_debug_init(UART_HandleTypeDef *huart_debug,
                     UART_HandleTypeDef *huart_relay_target)
{
    fifo_debug = (struct uart_tx_fifo){huart_debug, {0}, 0, 0};
    fifo_relay = (struct uart_tx_fifo){huart_relay_target, {0}, 0, 0};
}

void uart_debug_relay_to_bt(uint8_t c)
{
    fifo_enqueue(&fifo_relay, uart_relay_tx_isr, c);
}

int uart_debug_tx_done(void)
{
    return !HAL_IS_BIT_SET(fifo_debug.huart->Instance->CR1, USART_CR1_TXEIE) &&
           HAL_IS_BIT_SET(fifo_debug.huart->Instance->ISR, USART_ISR_TC);
}
