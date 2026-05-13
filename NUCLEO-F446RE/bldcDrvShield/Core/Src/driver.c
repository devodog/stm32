/*
 * driver.c
 *
 *  Created on: 5. mai 2026
 *      Author: dagak
 */
#include "main.h"
#include <stdint.h>
#include "driver.h"
///////////////////////////////////////////////////////////////////////////////
//                           FIX THE COMMENTS!                               //
///////////////////////////////////////////////////////////////////////////////
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
/******************************************************************************
// Using the capital letters R, S, and T as notation for describing the three
// phases, motivated by the notation used in the DRV8300 documentation.
// In addition the abbreviation HS = High Side transistor of the each
// phase and LS = Low Side transistor of each phase.

// The three phase inverter is implemented 6 mosfet transistors, which in total
// can have only 6 conduction states - Six Step commutation
// These are as follows:
// 1. R-HS & S-LS
// 2. R-HS & T-LS
// 3. S-HS & C-LS
// 4. S-HS & R-LS
// 5. T-HS & R-LS
// 6. T-HS & S-LS
//
// Hardware driver to be used have the following interface

-------------------------------------------
|   1  |   2  |   3  |   4  |   5  |   6  |
| T-LS | S-LS | R-LS | T-HS | S-HS | R-HS |
| PA5  | PA4  | PA3  | PA2  | PA1  | PA0  | STATE
-------------------------------------------------
   !0     !1     !0      0      0      1      1 (0x29)
   !1     !0     !0      0      0      1      2 (0x19)
   !1     !0     !0      0      1      0      3 (0x1A)
   !0     !0     !1      0      1      0      4 (0x32)
   !0     !0     !1      1      0      0      5 (0x34)
   !0     !1     !0      1      0      0      6 (0x2C)

Notice the use of the "NOT" sign to indicate that the specific gate driver
inverts the low side gate drivers.

******************************************************************************/
//uint8_t gateDriverStates[6] = {0x29, 0x19, 0x1a, 0x32, 0x34, 0x2c};

/******************************************************************************
 * We will need a gate-signal distribution plan or algorithm.
 * The gates on the high side transistors will be sourced by a PWM signal,
 * while the gates on the low side transistor will either on or off depending
 * on the commutation state.
 *
 * We'll assign PA0, PA1 and PA2 for PWM gate signals.
 * The PA3, PA4 and PA5 are configured for on off logic gate signals.
 *
******************************************************************************/

// Need a hall-state to gateDriverState conversion map...
/////////////////////////////////////////////////////////

/*
 * Sequence of the hall sensors in forward direction might be as the following:
 * 0x05, 0x01, 0x03, 0x02, 0x06, 0x04, 0x05,,,,
 *
 * We'll assume that the hall sensor states are related to the motor wiring
 * configuration and these hall sensor states can be used to control the
 * drivers gate signals.
 *
 * To map the hall sensor states to each of the three phases will require a
 * start up sequence, which will start by exciting each gate driver state in
 * small steps until a hall state changes. For this change the current gate
 * driver state is mapped to a hall sensor state. After a complete motor
 * rotation the mapping between the gate driver state and the hall sensor state
 * should be also completed.

       [hall readout] - defining the rotor's position
             v
   hallState[5] = 1; <-- switching states...(gateDriverState)
   hallState[1] = 2;
   hallState[3] = 3;
   hallState[2] = 4;
   hallState[6] = 5;
   hallState[4] = 6;

 * PWM for one of the high side transistors
 *
 *
 */
uint8_t gateDriverStates[6] = {0x29, 0x19, 0x1a, 0x32, 0x34, 0x2c};
// mapping between hall states(index) and commutation states(values)
uint8_t hallStates[7] = {0, 2, 4, 3, 6, 1, 5};

uint8_t low_side[6] = {0x28, 0x18, 0x18, 0x30, 0x30, 0x28};


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


//uint8_t gateStates[7];

uint16_t readHallSensors() {
   uint16_t hallState = (GPIOB->IDR & 0x70) >> 4;
   printf("\r\nhall state = 0x%02x", hallState);
   return hallState;
}

//
//uint8_t low_side[6] = {0x28, 0x18, 0x18, 0x30, 0x30, 0x28};
enum phase {
   N,
   R,
   S,
   T, // = 4 // = 0b0100 which will be the binary value of the relevant phase mapped to the actual PWM GPIO
};

int pwmChannel(int ch) {
   if (ch == R) {
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
      HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
   }
   else if (ch == S) {
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
      HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
   }
   else if (ch == T) {
      HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
   }
   else {
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
      return N;
   }
   return ch;
}
void phaseTest(int dutyCycle) {
   int channel = 1;
   TIM2->CCR3 = 65535*dutyCycle/100;
   HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
   printf("PWM on channel %d\r\n", channel);
}

int start(int dutyCycle) {
   // Start with 2 - 5% duty cycle...
   // 16 bit => 65535 clock cycles for a max frequency...
   // ~ 1kHz for mcu clock at 72 MHz...
   //

   int activeHighSidePhase = N;
   pwmChannel(activeHighSidePhase); // Disabling all high side gates...
   printf("Slow start...\r\n");
   // Make a loop that will include all commutation steps
   for (int i = 0; i < 6; i++) {

      //GPIOA->ODR |= 0x38; // set all low side gate signals high as the driver will invert these and cut off the transistors...
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

      // Now that the PWM is "running" on one of the high side gates, the relevant low side transistors can be "opened".
      //GPIOA->ODR &= low_side[i];

      // Entering a loop for increasing duty cycle to move the motor in either direction? -should know which direction is the user input...
      // Clock frequency is 70 MHz => 14,3 ns per tick. For a 16 bit CCR register a full count-down will take approx. 936 µs
      //
      for (int j = 0; j < dutyCycle; j++) {
         if (activeHighSidePhase == R)
            TIM2->CCR1 = 65535*j/100;
         else if (activeHighSidePhase == S)
            TIM2->CCR2 = 65535*j/100;
         else if (activeHighSidePhase == T)
            TIM2->CCR3 = 65535*j/100;
         
         //printf("activeHighSidePhase: %d low-side: 0x%02x\r\n", activeHighSidePhase, (uint8_t)GPIOA->ODR);
         HAL_Delay(200);
         /***
         uint16_t hall = readHallSensors();
         if ((hallStateChanged == 1)&&(states < 7)) {
            // The interrupt is triggered again...
            if (activeHighSidePhase != N) {
               gateStates[i] = (GPIOA->ODR & 0b111000) | (0x1 << (activeHighSidePhase - 1));
               hallStates[i] = hall;
               states++;
            }
            hallStateChanged = 0;
         }
         else {
            return 1;
         }
         ***/
      }
   }
   pwmChannel(N);
   return 0;
}

void stop() {
   pwmChannel(N);
}

void run(int dc) {
   //hallState = readHallSensors();
   // The run() function will only update the compare register for all timer channels...
   // The hall interrupt service routine will supply the pwm signal to the relevant high side transistor-gate.
   while (motorState !=IDLE) {
      TIM2->CCR1 = 65535*dc/100;
      TIM2->CCR2 = 65535*dc/100;
      TIM2->CCR3 = 65535*dc/100;
   }
}
