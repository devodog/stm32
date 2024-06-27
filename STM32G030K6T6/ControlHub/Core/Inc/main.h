/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "appver.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define StopwatchStart_Pin GPIO_PIN_9
#define StopwatchStart_GPIO_Port GPIOB
#define TargetInt1_Pin GPIO_PIN_4
#define TargetInt1_GPIO_Port GPIOA
#define TargetInt1_EXTI_IRQn EXTI4_15_IRQn
#define TargetInt2_Pin GPIO_PIN_5
#define TargetInt2_GPIO_Port GPIOA
#define TargetInt2_EXTI_IRQn EXTI4_15_IRQn
#define TargetInt3_Pin GPIO_PIN_6
#define TargetInt3_GPIO_Port GPIOA
#define TargetInt3_EXTI_IRQn EXTI4_15_IRQn
#define TargetInt4_Pin GPIO_PIN_7
#define TargetInt4_GPIO_Port GPIOA
#define TargetInt4_EXTI_IRQn EXTI4_15_IRQn
#define Servo5_Pin GPIO_PIN_0
#define Servo5_GPIO_Port GPIOB
#define Servo4_Pin GPIO_PIN_1
#define Servo4_GPIO_Port GPIOB
#define Servo3_Pin GPIO_PIN_2
#define Servo3_GPIO_Port GPIOB
#define TargetInt5_Pin GPIO_PIN_8
#define TargetInt5_GPIO_Port GPIOA
#define TargetInt5_EXTI_IRQn EXTI4_15_IRQn
#define LED_Pin GPIO_PIN_6
#define LED_GPIO_Port GPIOC
#define serClk_Pin GPIO_PIN_11
#define serClk_GPIO_Port GPIOA
#define SerStrobe_Pin GPIO_PIN_12
#define SerStrobe_GPIO_Port GPIOA
#define TargetsReset_Pin GPIO_PIN_15
#define TargetsReset_GPIO_Port GPIOA
#define Servo2_Pin GPIO_PIN_3
#define Servo2_GPIO_Port GPIOB
#define Servo1_Pin GPIO_PIN_4
#define Servo1_GPIO_Port GPIOB
#define Hundreth7seg_Pin GPIO_PIN_5
#define Hundreth7seg_GPIO_Port GPIOB
#define Seconds7seg_Pin GPIO_PIN_6
#define Seconds7seg_GPIO_Port GPIOB
#define Minutes7seg_Pin GPIO_PIN_7
#define Minutes7seg_GPIO_Port GPIOB
#define Hours7seg_Pin GPIO_PIN_8
#define Hours7seg_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
