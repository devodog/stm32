/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/**
 * Version 1.0 07MAR24 /Dkv
 * Code for serial data transmission.
 * Receiving end-point is 2 8-bit cascade connected shift registers which is to
 * output data to two digit 7-segment display (each shit register is assigned to
 * one large 7-segment display).
 *
 * The shift registers is MC14094 or cd4094.
 * Must use a TTL(3.3V) to CMOS(12V) interface level shifter.
 *
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// 7-segment digit      0    1    2    3    4    5    6    7    8    9
// Common anode
//uint8_t ssCode[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90}; // from the Internet

// Common cathode
//uint8_t ssCode[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0xf7,0x4f}; // from the Internet
uint8_t ssCode[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x67};
int delay = 1;
enum stop_watch_state {
   RUNNING,
   STOPPED,
   RESET_
} stopWatchState = RESET_;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
   if (stopWatchState == RUNNING) {
      stopWatchState = STOPPED;
   }
   else if (stopWatchState == STOPPED) {
      stopWatchState = RESET_;
   }
   else {
      // START
      stopWatchState = RUNNING;
   }
}
/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
   HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
   HAL_TIM_Base_Start_IT(&htim2);
}
*/

int _write(int fd, char *ptr, int len) {
   //HAL_UART_Transmit(&huart1, (uint8_t *) ptr, len, HAL_MAX_DELAY);
   HAL_UART_Transmit(&huart1, (uint8_t*) ptr, len, 1000);
   return len;
}

void updateDisplay(uint8_t twoDigitNumber) {
   uint8_t sLine = 0;
   uint8_t element = 0;

   if (twoDigitNumber > 99)
      twoDigitNumber = 99;

   // First digit.
   uint8_t digit = twoDigitNumber%10;
   HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

   // Start sending
   while (element < 2) {
      for (int i = 0; i < 8; i++) {
         // msb (most significant bit) on the line first => big endian (lowest value at the highest address at the receiving side)
         sLine = (ssCode[digit] >> (7-i)) & 0x1;
         // Data on sData_Pin
         HAL_GPIO_WritePin(GPIOC, sData_Pin, sLine); //PC1 <=> D-SUB#4 = Orange&White = DATA
         HAL_Delay(delay);

         // Clock goes HIGH latching the data Neg. Logic
         HAL_GPIO_WritePin(GPIOC, sClk_Pin, GPIO_PIN_SET); //PC0 <=> D-SUB#5 = Green = CLK
         HAL_Delay(delay);
         //
         HAL_GPIO_WritePin(GPIOC, sClk_Pin, GPIO_PIN_RESET);
         HAL_Delay(delay);
      }
      // Second digit
      digit = twoDigitNumber/10;
      element++;
   }
   // Transmission done - strobe / latch new data onto the output on the shift registers.
   HAL_GPIO_WritePin(GPIOC, strobe_Pin, GPIO_PIN_SET); //PC2 <=> D-SUB#3 = Orange = STROBE
   HAL_Delay(delay);
   HAL_GPIO_WritePin(GPIOC, strobe_Pin, GPIO_PIN_RESET);
}

void update4Displays(uint16_t fourDigitNumber) {
   uint8_t sLine = 0;
   uint8_t element = 0;
   uint16_t modulo = 10;
   uint16_t digitPos = 1;
   uint8_t digits[4];
   // Extract digits to send
   for (int i = 0; i < 3; i++) {
      digits[i] = (fourDigitNumber%modulo)/digitPos;
      digitPos *= 10;
      modulo *= 10;
   }
   digits[3] = fourDigitNumber/digitPos;
   printf("Digit string to send: %d %d %d %d\r\n", digits[3],digits[2],digits[1],digits[0]);
   //uint8_t digit = fourDigitNumber%modulo;
   HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

   // Start sending
   while (element < 4) {
      for (int i = 0; i < 8; i++) {
         // msb (most significant bit) on the line first => big endian (lowest value at the highest address at the receiving side)
         sLine = (ssCode[digits[element]] >> (7-i)) & 0x1;
         // Data on sData_Pin
         HAL_GPIO_WritePin(GPIOC, sData_Pin, sLine); //PC1 <=> D-SUB#4 = Orange&White = DATA
         HAL_Delay(delay);

         // Clock goes HIGH latching the data Neg. Logic
         HAL_GPIO_WritePin(GPIOC, sClk_Pin, GPIO_PIN_SET); //PC0 <=> D-SUB#5 = Green = CLK
         HAL_Delay(delay);
         //
         HAL_GPIO_WritePin(GPIOC, sClk_Pin, GPIO_PIN_RESET);
         HAL_Delay(delay);
      }

      // Next digit
      // 1. digit sent, element == 0
      // digit fourDigitNumber%100)/10, element == 1
      // 2. digit sent
      // digit = fourDigitNumber%1000)/100, element == 2
      // 3. digit sent
      // if (element == 2)
      //    digit = fourDigitNumber/1000, element == 3
      // 4. digit sent
      // if(element >= 3)
      //    break;
      /***************
      digitPos *= 10;   // ...the denominator for the result from the modulo operation...
      if(element < 2) { // 0 for tens, 1 for hundreds and 2 for thousens
         modulo *= 10;
         digit = (fourDigitNumber%modulo)/digitPos;
      }
      else if (element == 2) {
         digit = fourDigitNumber/digitPos;
      }
      else {
         break;
      }
      if ((digit < 0) || (digit > 9)) {
         digit = 0;
      }
      ***/
      element++;
   }
   // Transmission done - strobe / latch new data onto the output on the shift registers.
   HAL_GPIO_WritePin(GPIOC, strobe_Pin, GPIO_PIN_SET); //PC2 <=> D-SUB#3 = Orange = STROBE
   HAL_Delay(delay);
   HAL_GPIO_WritePin(GPIOC, strobe_Pin, GPIO_PIN_RESET);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint16_t stopWachTime = 0;
  uint16_t hundreds = 0;
  uint8_t seconds = 0;
  uint8_t resetSent = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  //HAL_Delay(2000);
  //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  //HAL_TIM_Base_Start_IT(&htim2);
  update4Displays(1234);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
     if (stopWatchState == RUNNING) {
        update4Displays(stopWachTime);
        //updateDisplay(seconds);
        //if (seconds++ > 59) // compares then increments
        //if (++seconds > 59) { // increments then compare
        if (++hundreds > 100) { // increments then compare
           stopWachTime += 100;
           hundreds = 0;
           //seconds = 0;
        }
        else {
           stopWachTime += hundreds;
        }
        HAL_Delay(10);
     }
     else if (stopWatchState == RESET_) {
        if (resetSent == 0) {
           update4Displays(0);
           resetSent = 1;
        }
     }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, sClk_Pin|sData_Pin|strobe_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : sClk_Pin sData_Pin strobe_Pin */
  GPIO_InitStruct.Pin = sClk_Pin|sData_Pin|strobe_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
