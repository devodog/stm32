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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "appver.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TARGET_HIT 1
#define COVER_RESET 0
#define ALL_HIT 0x001f

#define GPIO_EXTI_READY
#define STOPWATCH_DISPLAY_READY
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim16;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM16_Init(void);
/* USER CODE BEGIN PFP */

int _write(int fd, char *ptr, int len) {
	//HAL_UART_Transmit(&huart1, (uint8_t *) ptr, len, HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart1, (uint8_t*) ptr, len, 1000);
	return len;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t UART1_rxBuffer = 0;
uint8_t cmdComplete;
char termInputBuffer[80];
int bytesReceived = 0;
uint16_t targetState;
uint16_t targetHitIndication = 0;

enum stop_watch_state {
   RUNNING,
   STOPPED,
   RESET_
} stopWatchState = RESET_;

// The HAL_UART_TxCpltCallback(), HAL_UART_RxCpltCallback() user callbacks
// will be executed respectively at the end of the transmit or Receive process
// ref. stm32g0xx_hal_uart.c line 1037.
#ifdef UART_RX_READY
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (UART1_rxBuffer == 13) {
		if (bytesReceived > 0) {
			//executeCmd(&termInputBuffer[0], bytesReceived);
			bytesReceived = 0;
			memset(termInputBuffer, 0, 80);
		} else {
			promt();
		}
		HAL_UART_Receive_IT(&huart1, &UART1_rxBuffer, 1);
		return;
	}
	HAL_UART_Transmit(&huart1, &UART1_rxBuffer, 1, 100);
	termInputBuffer[bytesReceived] = UART1_rxBuffer;
	bytesReceived++;
	// re-trigger the interrupt...
	HAL_UART_Receive_IT(&huart1, &UART1_rxBuffer, 1);
}
#endif

void delay_us(volatile uint16_t au16_us)
{
   htim16.Instance->CNT = 0;
   while (htim16.Instance->CNT < au16_us);
}

// _delay_us(2200); max counterclockwise (165 degrees)
// _delay_us(1300); middle? (90 degrees)
void coverReset(uint16_t servoPin) {
   for (int i=0; i<20; i++) {
      HAL_GPIO_WritePin(GPIOA, servoPin, GPIO_PIN_SET);
      delay_us(2000);
      HAL_GPIO_WritePin(GPIOA, servoPin, GPIO_PIN_RESET);
      HAL_Delay(20);
   }
   targetState &= ~servoPin;
}

#ifdef GPIO_EXTI_READY
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
   // See https://www.geeksforgeeks.org/c-switch-statement/ especially for how
   // the flowchart for the switch-statement is drawn...
   switch (GPIO_Pin) {
      case TargetInt1_Pin:
         targetHitIndication = Servo1_Pin;
         break;
      case TargetInt2_Pin:
         targetHitIndication = Servo2_Pin;
         break;
      case TargetInt3_Pin:
         targetHitIndication = Servo3_Pin;
         break;
      case TargetInt4_Pin:
         targetHitIndication = Servo4_Pin;
         break;
      case TargetInt5_Pin:
         targetHitIndication = Servo5_Pin;
         break;
      case StopwatchStart_Pin:
         stopWatchState = RUNNING;
         break;
      case TargetsReset_Pin:
         if (stopWatchState == RUNNING) {
            stopWatchState = STOPPED;
         }
         coverReset(Servo1_Pin);
         coverReset(Servo2_Pin);
         coverReset(Servo3_Pin);
         coverReset(Servo4_Pin);
         coverReset(Servo5_Pin);
         break;
      break;
      default:
   }
}
#endif

// Common cathode
// 7-segment digit   0    1    2    3    4    5    6    7    8    9
uint8_t ssCode[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x67};


#ifdef STOPWATCH_DISPLAY_READY
void displaySeconds(uint16_t fourDigitNumber) {
   uint8_t sLine1 = 0;
   uint8_t sLine2 = 0;
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
   //printf("Digit string to send: %d %d %d %d\r\n", digits[3],digits[2],digits[1],digits[0]);
   //uint8_t digit = fourDigitNumber%modulo;

   // Start sending
   while (element < 2) {
      for (int i = 0; i < 8; i++) {
         // msb (most significant bit) on the line first => big endian (lowest value at the highest address at the receiving side)
         sLine1 = (ssCode[digits[element]] >> (7-i)) & 0x1;
         sLine2 = (ssCode[digits[element+2]] >> (7-i)) & 0x1;
         // Data on sData_Pin
         HAL_GPIO_WritePin(GPIOB, Hundreth7seg_Pin, sLine1); //PC1 <=> D-SUB#4 = Orange&White = DATA for display element 1
         HAL_GPIO_WritePin(GPIOB, Seconds7seg_Pin, sLine2); //PC3 <=> D-SUB#4 = Green&White = DATA for display element 2
         //HAL_Delay(delay);
         delay_us(250);

         // Clock goes HIGH latching the data Neg. Logic
         HAL_GPIO_WritePin(GPIOA, serClk_Pin, GPIO_PIN_SET); //PC0 <=> D-SUB#5 = Green = CLK
         //HAL_Delay(delay);
         delay_us(250);
         //
         HAL_GPIO_WritePin(GPIOA, serClk_Pin, GPIO_PIN_RESET);
         //HAL_Delay(delay);
         delay_us(250);
      }
      element++;
   }
   // Transmission done - strobe / latch new data onto the output on the shift registers.
   HAL_GPIO_WritePin(GPIOA, SerStrobe_Pin, GPIO_PIN_SET); //PC2 <=> D-SUB#3 = Orange = STROBE
   HAL_Delay(1);
   HAL_GPIO_WritePin(GPIOA, SerStrobe_Pin, GPIO_PIN_RESET);
}

void displayMinutes(uint16_t fourDigitNumber) {
   //
}
#endif


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
   /* USER CODE BEGIN 1 */
   uint16_t stopWachTime = 0;
   uint8_t servoPulses = 0;
   uint8_t resetSent = 0; // Unsure if this is necessary...
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
  MX_USART1_UART_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */
  printf("\r\nControl Hub initialized. Version: %d.%d - Build: %d\r\n", MAJOR_VERSION, MINOR_VERSION, BUILD);
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	  HAL_Delay(1000);
	  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
#ifdef STOPWATCH_DISPLAY_READY
     if (stopWatchState == RUNNING) {
        resetSent = 0;
        // if a hit is registered and the servo is activated then an additional
        // 500 ms must be added to the stopwatch time... - can this be part of the stopwatch loop?
        if (targetHitIndication != 0) {
           HAL_GPIO_WritePin(GPIOA, targetHitIndication, GPIO_PIN_SET);
           // Target hit! Cover the target.
           delay_us(1000);
           HAL_GPIO_WritePin(GPIOA, targetHitIndication, GPIO_PIN_RESET);
           HAL_Delay(10); // this while loop takes approximately 10 ms, so an additional 10 ms is added to comply with the servo requirements of a pwm-frequency of 50 Hz.
           stopWachTime++;
           if (++servoPulses > 20) {
              targetHitIndication = 0;
              servoPulses = 0;
           }
        }
        if (++stopWachTime > 5999) {
           stopWachTime = 0;
        }
        displaySeconds(stopWachTime);
        if (targetState == ALL_HIT) {
           stopWatchState = STOPPED;
        }
     }

     if (stopWatchState == RESET_) {
        if (resetSent == 0) {
           //printf("Stop-watch RESET*\r\n");
           stopWachTime = 0;
           displaySeconds(stopWachTime);
           resetSent = 1;
        }
     }
     HAL_Delay(7); // = approx. 0.01 sec.
#endif

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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 0;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 65535;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

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
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Servo5_Pin|Servo4_Pin|Servo3_Pin|Servo2_Pin
                          |Servo1_Pin|Hundreth7seg_Pin|Seconds7seg_Pin|Minutes7seg_Pin
                          |Hours7seg_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, serClk_Pin|SerStrobe_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : StopwatchStart_Pin */
  GPIO_InitStruct.Pin = StopwatchStart_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(StopwatchStart_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TargetInt1_Pin TargetInt2_Pin TargetInt3_Pin TargetInt4_Pin
                           TargetInt5_Pin TargetsReset_Pin */
  GPIO_InitStruct.Pin = TargetInt1_Pin|TargetInt2_Pin|TargetInt3_Pin|TargetInt4_Pin
                          |TargetInt5_Pin|TargetsReset_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Servo5_Pin Servo4_Pin Servo3_Pin Servo2_Pin
                           Servo1_Pin Hundreth7seg_Pin Seconds7seg_Pin Minutes7seg_Pin
                           Hours7seg_Pin */
  GPIO_InitStruct.Pin = Servo5_Pin|Servo4_Pin|Servo3_Pin|Servo2_Pin
                          |Servo1_Pin|Hundreth7seg_Pin|Seconds7seg_Pin|Minutes7seg_Pin
                          |Hours7seg_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : serClk_Pin SerStrobe_Pin */
  GPIO_InitStruct.Pin = serClk_Pin|SerStrobe_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

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
