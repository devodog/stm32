/*
 * driver.c
 *
 *  Created on: 5. mai 2026
 *      Author: dagak
 */
#include "main.h"
#include <stdint.h>

enum State{
   IDLE,
   STARTED,
   RUNNING_FORWARD,
   RUNNING_REVERSE
};

extern uint32_t dutyCycle;
extern TIM_HandleTypeDef htim2;
extern uint8_t hallStateChanged;
extern enum State motorState;

uint8_t gateState[7];
void setGates() {
   //GPIOA_BSRR = gateState[hallState];
}
/*
 * The start function will try to start a bldc motor, and it is called when the
 * rotational potensiometer,w whitch is the user input, is turned away from its
 * center postion. The motor is IDLE When the potensiometer is in the center 
 * position. Turning the potentiometer in the clockwize drirection should rotate
 * the motor in the forward direction.
 * 
 * The start sequence should ramp up the duty cycle from zero to user input 
 * duty cycle .
 * 
 * 
 * input argument
 *  - dutyCycle: a value that is the stady state motor operation.
 * 
 * Other info:
 * Using timer TIM2 for 3 individual PWM channels that is connected to PA0, PA1 
 * and PA2.
 * 
 * PA0 is connected to the gate on high-side transistor for phase R
 * PA1 is connedted to the gate on high-side transistor for pahes S
 * PA2 is connedted to the gate on high-side transistor for pahes T
 * 
 * For six step commutiation the switching table is as follows:
 * PA0                                        inverted low side
 *  x    x    x   01    10    01  = (0x10) -> (0x28) using that PA0 is the least significant bit.
 *  x    x    x   01    01    10  = (0x20) -> (0x18)
 *  x    x    x   01    01    10  = (0x20) -> (0x18)
 *  x    x    x   10    01    01  = (0x08) -> (0x30)
 *  x    x    x   10    01    01  = (0x08) -> (0x30)
 *  x    x    x   01    10    01  = (0x10) -> (0x28)
*/
//
uint8_t low_side[6] = {0x28, 0x18, 0x18, 0x30, 0x30, 0x28};
enum phase {
   N,
   R,
   S,
   T
};

void pwmChannel(int ch) {
   if (ch == R) {
         HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
         HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2 | TIM_CHANNEL_3);
   }
   else if (ch == S) {
      HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1 | TIM_CHANNEL_3);
   }
   else if (ch == T) {
      HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2 | TIM_CHANNEL_1);
   }
   else {
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1 | TIM_CHANNEL_2 | TIM_CHANNEL_3);
   }
}

int start(int dutyCycle) {
   // Start with 2 - 5% duty cycle...
   // 16 bit => 65535 clock cycles for a max frequency...
   // ~ 1kHz for mcu clock at 72 MHz...
   //
   int activeHighSidePhase = N;
   pwmChannel(activeHighSidePhase); // Disabling all high side gates...
   // Make a loop that will include all commutation steps
   for (int i = 0; i < 6; i++) {
      GPIOA->ODR |= 0x38; // set all low side gate signals high as the driver will invert these and cut off the transistors...
      if (i == 0) {
         activeHighSidePhase = R;
      }
      else if (i == 2) {
         activeHighSidePhase = S;
      }
      else if (i == 4) {
         activeHighSidePhase = T;
      }
      pwmChannel(activeHighSidePhase);

      // Now that the PWM is "running" on one of the high side gates, the relevant low side transister han be "opened".
      GPIOA->ODR &= low_side[i]; 

      // Entering a loop for increasing duty cycle to move the motor in either direction? -should know which direction is the user input...
      for (int j = 0; j < dutyCycle; j++) {
         if (activeHighSidePhase == R)
            TIM2->CCR1 = 65535*j/100;
         else if (activeHighSidePhase == S)
            TIM2->CCR2 = 65535*j/100;
         else if (activeHighSidePhase == T)
            TIM2->CCR3 = 65535*j/100;
         
         HAL_Delay(10);
         if (hallStateChanged == 1) {
            // The interrupt is triggered again...
            
            return 1;
         }
      }
   }
   return 0;
}

void run() {
   //hallState = readHallSensors();
   while (motorState !=IDLE) {

   }
}
