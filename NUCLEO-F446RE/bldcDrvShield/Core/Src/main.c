/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "appver.h"
#include "cmd.h"
#include "driver.h"
#include "test.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IDLE_STATE 2048
#define POTMETER_TOLERANCE 32
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

int _write(int fd, char *ptr, int len) {
	//HAL_UART_Transmit(&huart1, (uint8_t *) ptr, len, HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart1, (uint8_t*) ptr, len, 1000);
	return len;
}


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/************** NOTE on using GPIO PA2, PA3 and PA5 ***************************/
/*
 * The NUCLEO-F446RE Development board has excluded PA2 and PA3 from the
 * Morpho and Arduino extension connectors as the solder bridge (SB62 and SB63)
 * for these are open. The LED LD2 is connected to the PA5 as SB21 is closed.
 * So, to make use of these GPIOs the NUCLEO-F446RE Development board has been
 * modified in order to utilize these pins.
 *
 */


//
extern uint8_t gateDriverStates[6];
// mapping between hall states(index) and commutation states(values)
extern uint8_t hallStates[7];

int highSide[6] = {1,1,2,2,3,3};
extern uint8_t low_side[6];

int dutyCycle = 0;
int dcStart = 0;
int stopTest = 1;

enum State motorState = IDLE;
uint16_t hallState;
uint8_t hallStateChanged = 0;
/**
 * The interrupt service routine (ISR) is to drive the motor WHEN the user
 * input is not zero.
 * The following must be updated in the ISR.:
 * - The timer that controls the PWM signals
 * - Next Commutation State -> activating relevant driver transistors
 * - - PWM on high-side transistor
 *
 *
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {   
   uint8_t commutationState;
   uint16_t hallState = (GPIOB->IDR & 0x70) >> 4;

   // Switch off the low side transistors (signals inverted in the gate driver)
   GPIOA->ODR |= 0x38; //

   // If the motor is in starting mode, we'll just set the hallStateChange and
   // return and hope that the startup process can finish by it self...
   if ((motorState == STARTING_FORWARD) || (motorState == STARTING_REVERSE)) {
      hallStateChanged = 1;
      return;
   }
   // We now have a running motor and we'll determine the commutation step
   // to use.
   commutationState = hallStates[hallState];
   //printf("hallState=0x%x  comm=%d  lowside=0x%x\r\n", hallState, commutationState, low_side[commutationState]);

   if (motorState == RUNNING_REVERSE) {
      // reverse...
      // need to know if the decrement will give an none existent state (negative)...
      if (--commutationState < 0) {
         commutationState = 5;
      }
   }
   if (++commutationState > 5) {
      commutationState = 0;
   }


   // Using the initial user input for PWM settings update the pwmUpdate()
   // routine in steps until it matches the users input
   if (++dcStart < dutyCycle) {
      pwmUpdate(dcStart);
   }
   // and turn on the relevant high side transistor.
   pwmChannel(highSide[commutationState]);
   // Turn on the low side transistor...
   GPIOA->ODR = (GPIOA->ODR & ~(0x7 << 3)) | (low_side[commutationState]);
}

uint8_t UART1_rxBuffer = 0;
uint8_t cmdComplete;
char termInputBuffer[80];
int bytesReceived = 0;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    //uint8_t UARTnewLine = 10;
    if (UART1_rxBuffer == 13) {
        //HAL_UART_Transmit(&huart1, &UARTnewLine, 1, 100);
        if (bytesReceived > 0) {
            //HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

            executeCmd(&termInputBuffer[0], bytesReceived);
            bytesReceived = 0;
            memset(termInputBuffer, 0, 80);
            //HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
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

/**
 * getUserInput reads the voltage on the center pin of a 10k potentiometer
 * connected to gnd and vcc, by the use of the mcu's adc.
 * The adc value is then converted to a percentage value of the max adc value,
 * which is then used as a Duty Cycle value for the PWM signals.
 */
int c = 0;
void getUserInput() {
   uint32_t adcReading = 0;
   float dc = 0;

   adcReading = HAL_ADC_GetValue(&hadc1);

   if ((adcReading < (IDLE_STATE + POTMETER_TOLERANCE)) && (adcReading > (IDLE_STATE - POTMETER_TOLERANCE))) {
      if (motorState != IDLE) {
         motorState = IDLE;
         // Silence the high side gate signals...
         stop();

         printf("Motor is halted...");
         promt();
      }
      // some kind of indication that the motor is idle
      if (++c > 19) {
         printf("Motor is idle...");
         promt();
         c = 0;
      }

      return;
   }

   dc = 200*((adcReading-2048.0)/4096.0);

   if (motorState == IDLE) {
      if (dc < 0) {
         motorState = STARTING_REVERSE;
         printf("Starting the Motor in reverse...\r\n");
      }
      else {
         motorState = STARTING_FORWARD;
         printf("Starting the Motor forward...\r\n");
      }

      if (dc < 0)
         dc = dc*(-1);

      start((int)dc);
   }
   // The following code is used for TEST of the start up sequence...
   else if ((motorState == STARTING_FORWARD) || (motorState == STARTING_REVERSE)) {
      if (dc < 0)
         dc = dc*(-1);

      if ((dc > dutyCycle + 2) || (dc < dutyCycle - 2)) { // Only interested in change in user input.
         // If dc is more than the previous set dutyCycle plus some tolerance, then update the dytyCycle.
         // If dc is less than the previous set dutyCycle minus some tolerance, then update the dytyCycle.
         dutyCycle = (int)dc;
         //printf("ADC Readings: %ld = ", adcReading);
         //printf("dc: %.3f \r\n", dc);
         printf("User input(start): %d%% Duty Cycle\r\n", (int) dutyCycle);
         start((int)dc);
      }
   }
   // The following code is to handle dynamic motor speed operation.
   else if ((motorState == RUNNING_FORWARD) || (motorState == RUNNING_REVERSE)) {
      if (dc < 0)
         dc = dc*(-1);

      if ((dc > dutyCycle + 1) || (dc < dutyCycle - 1)) { // Only interested in change in user input.
         // If dc is more than the previous set dutyCycle plus some tolerance, then update the dytyCycle.
         // If dc is less than the previous set dutyCycle minus some tolerance, then update the dytyCycle.
         dutyCycle = (int)dc;
         //printf("ADC Readings: %ld = ", adcReading);
         //printf("dc: %.3f \r\n", dc);
         printf("User input: %d%% Duty Cycle\r\n", (int) dutyCycle);
      }
      pwmUpdate(dutyCycle);
   }
   // The following code is for TESTING the PWM dynamics...
   else if (motorState == TESTING) {
     runTest((int)dc);
   }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, &UART1_rxBuffer, 1);

  HAL_ADC_Start(&hadc1);
  HAL_TIM_Base_Start(&htim2);
  //HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  // Output debug information
  //debug_PA2_configuration();

  printf("\r\n>>> BLDC driver Ver. %d.%d >>>", MAJOR_VERSION, MINOR_VERSION);
  printf("\r\n>>> Build Number: %d, Build Date: %s >>>\r\n", BUILD, BUILD_DATE_AND_TIME);
  promt();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // Polling for user potmeter-input...
    getUserInput();
    HAL_Delay(1000); // using 5 sec for test without motor...

    __WFI(); // optional: wait for interrupt to save power
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 5;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA3 PA4 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB4 PB5 PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

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
#ifdef USE_FULL_ASSERT
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
