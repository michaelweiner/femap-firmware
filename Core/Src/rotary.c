#include "rotary.h"
#include "main.h"
#include <string.h>
#include <sys/types.h>

static TIM_HandleTypeDef *htim_rotary;

void init_rotary(TIM_HandleTypeDef *htim) { htim_rotary = htim; }

int read_rotary(uint8_t *pau8Digit, size_t *len)
{
    size_t max_len = *len;
    ssize_t pos = -1;
    memset(pau8Digit, 0, max_len);
    *len = 0;

    uint32_t timer_value = 0;
    HAL_TIM_PWM_Start(htim_rotary, TIM_CHANNEL_1);
    htim_rotary->Instance->EGR |= TIM_EGR_UG;

    while (1)
    {
        HAL_Delay(1);
        if (HAL_GPIO_ReadPin(nsi_GPIO_Port, nsi_Pin) == GPIO_PIN_RESET)
        {
            HAL_Delay(1);
            /* pulse received */
            timer_value = htim_rotary->Instance->CNT;
            htim_rotary->Instance->EGR |= TIM_EGR_UG;

            if ((pos < 0) || (timer_value > 200))
            {
                ++pos;
                if (pos >= max_len)
                {
                    /* buffer overflow */
                    HAL_TIM_PWM_Stop(htim_rotary, TIM_CHANNEL_1);
                    return 0;
                }
            }
            ++pau8Digit[pos];
            while (HAL_GPIO_ReadPin(nsi_GPIO_Port, nsi_Pin) == GPIO_PIN_RESET)
            {
                HAL_Delay(1);
                if (htim_rotary->Instance->CNT > 200)
                {
                    /* nsi stuck in low state, error */
                    HAL_TIM_PWM_Stop(htim_rotary, TIM_CHANNEL_1);
                    return 0;
                }
            }
        }
        else
        {
            if ((htim_rotary->Instance->CNT > 3000) &&
                (HAL_GPIO_ReadPin(nsa_GPIO_Port, nsa_Pin) != GPIO_PIN_SET))
            {
                HAL_TIM_PWM_Stop(htim_rotary, TIM_CHANNEL_1);
                /* good case timeout, dial */
                *len = (pos + 1);
                return 1;
            }
            else if (HAL_GPIO_ReadPin(GU_GPIO_Port, GU_Pin) == GPIO_PIN_RESET)
            {
                /* GU inactive */
                HAL_TIM_PWM_Stop(htim_rotary, TIM_CHANNEL_1);
                return 0;
            }
        }
    }
}

void count_to_ascii(uint8_t *pau8, size_t size)
{
    while (size-- > 0)
    {
        *pau8 %= 10;
        *pau8 += '0';
        ++pau8;
    }
}
