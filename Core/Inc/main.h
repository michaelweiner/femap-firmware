/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SW2_Pin GPIO_PIN_13
#define SW2_GPIO_Port GPIOC
#define BT_EN_Pin GPIO_PIN_2
#define BT_EN_GPIO_Port GPIOC
#define GU_Pin GPIO_PIN_3
#define GU_GPIO_Port GPIOC
#define GU_EXTI_IRQn EXTI3_IRQn
#define BT_RESET_Pin GPIO_PIN_0
#define BT_RESET_GPIO_Port GPIOA
#define Dialtone_VDIV_Pin GPIO_PIN_5
#define Dialtone_VDIV_GPIO_Port GPIOA
#define VOICE_EN_Pin GPIO_PIN_2
#define VOICE_EN_GPIO_Port GPIOB
#define HV_EN_Pin GPIO_PIN_13
#define HV_EN_GPIO_Port GPIOB
#define PGOOD_Pin GPIO_PIN_14
#define PGOOD_GPIO_Port GPIOB
#define nsa_Pin GPIO_PIN_6
#define nsa_GPIO_Port GPIOC
#define nsi_Pin GPIO_PIN_7
#define nsi_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
