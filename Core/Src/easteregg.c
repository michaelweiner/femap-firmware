#include "easteregg.h"

#include "easteregg_samples.h"
#include "main.h"

/*
 * Loops an 8 kHz / 8-bit unsigned PCM clip through DAC1 until stopped.
 *
 * The dial-tone path uses the same DAC, but with 12-bit half-word
 * samples at ~400 kHz. For voice playback we temporarily reprogram
 * TIM4 (the DAC sample clock) for an 8 kHz tick, disable its
 * slave-mode gating from TIM3, and reconfigure the DMA channel for
 * byte-wide transfers — restoring all of that on stop so the
 * dial-tone path keeps working.
 *
 * DMA stays in CIRCULAR mode, so the buffer plays on repeat with no
 * CPU involvement until stop_easteregg() tears it down.
 */

#define TIM4_ARR_DEFAULT 39u  /* CubeMX default — keep in sync with MX_TIM4_Init */
#define TIM4_ARR_8KHZ 1999u   /* 16 MHz / 2000 = 8 kHz */

static TIM_HandleTypeDef *s_htim_dac = NULL;
static DAC_HandleTypeDef *s_hdac = NULL;
static OPAMP_HandleTypeDef *s_hopamp = NULL;

static uint32_t s_saved_smcr_sms = 0;

void init_easteregg(TIM_HandleTypeDef *htim_dac, DAC_HandleTypeDef *hdac,
                    OPAMP_HandleTypeDef *hopamp)
{
    s_htim_dac = htim_dac;
    s_hdac = hdac;
    s_hopamp = hopamp;
}

void start_easteregg(void)
{
    DMA_HandleTypeDef *hdma = s_hdac->DMA_Handle1;

    s_saved_smcr_sms = s_htim_dac->Instance->SMCR & TIM_SMCR_SMS;

    /* Free-run TIM4 so the DAC sees a steady 8 kHz trigger regardless
     * of TIM3 (which the dial-tone path uses for on/off chopping). */
    s_htim_dac->Instance->SMCR &= ~TIM_SMCR_SMS;
    s_htim_dac->Instance->ARR = TIM4_ARR_8KHZ;

    HAL_DMA_DeInit(hdma);
    hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    HAL_DMA_Init(hdma);

    HAL_GPIO_WritePin(VOICE_EN_GPIO_Port, VOICE_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_TIM_PWM_Start(s_htim_dac, TIM_CHANNEL_1);

    /* Bring the analog path up against an explicit mid-scale silence:
     * pre-load the 8-bit holding register, enable the DAC channel so
     * the next TIM4 trigger latches 0x80 into DOR, wait for the OPAMP
     * to wake up against that quiet level, *then* hand the DAC over to
     * DMA. Avoids the otherwise-audible click of the OPAMP snapping on
     * to whatever sample value the clip happens to start at. */
    s_hdac->Instance->DHR8R1 = 0x80;
    __HAL_DAC_ENABLE(s_hdac, DAC_CHANNEL_1);
    HAL_Delay(1);
    HAL_OPAMP_Start(s_hopamp);
    HAL_Delay(1);

    HAL_DAC_Start_DMA(s_hdac, DAC_CHANNEL_1, (uint32_t *)samples_easteregg,
                      EASTEREGG_SAMPLES_LEN, DAC_ALIGN_8B_R);
}

void stop_easteregg(void)
{
    DMA_HandleTypeDef *hdma = s_hdac->DMA_Handle1;

    HAL_OPAMP_Stop(s_hopamp);
    HAL_DAC_Stop_DMA(s_hdac, DAC_CHANNEL_1);
    HAL_TIM_PWM_Stop(s_htim_dac, TIM_CHANNEL_1);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    s_htim_dac->Instance->ARR = TIM4_ARR_DEFAULT;
    s_htim_dac->Instance->SMCR =
        (s_htim_dac->Instance->SMCR & ~TIM_SMCR_SMS) | s_saved_smcr_sms;

    HAL_DMA_DeInit(hdma);
    hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma->Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    HAL_DMA_Init(hdma);
}
