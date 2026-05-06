/*
 * driver.c
 *
 *  Created on: 5. mai 2026
 *      Author: dagak
 */
#include <stdint.h>

extern uint32_t dutyCycle;
extern htim2;

uint8_t gateState[7];
void setGates() {
   //GPIOA_BSRR = gateState[hallState];
}

void start() {
   // Start with 2 - 5% duty cycle...
   // 16 bit => 65535 clock cycles for a max frequency...
   // ~ 1kHz for mcu clock at 72 MHz...
   for () {

   }
   TIM2->CCR1 = /;
   HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

void run() {

}
