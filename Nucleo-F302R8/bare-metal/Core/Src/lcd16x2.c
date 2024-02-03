/*
 * lcd16x2.c
 *
 *  Created on: Jan 27, 2024
 *      Author: dagak
 *
 * This is a driver for the Adafruit 16x2 LCD display
 * It is based on the Hitachi HD44780 display controller
 * It uses a 4-bit interface to the display
 * It is written for the stm32Nucleo-f302r8 microcontroller.
 *
 * Wiring:
   LCD pin 1 (GND) to GND
   LCD pin 2 (VCC) to 5V

   LCD pin 3 (contrast voltage) to 5V w/ 10K potentiometer

   LCD pin 4 (RS) to PB6
   LCD pin 5 (R/W) (grounded)
   LCD pin 6 (EN) to PB7

   LCD pin 7 (D0) to not used
   LCD pin 8 (D1) to not used
   LCD pin 9 (D2) to not used
   LCD pin 10 (D3) to not used

   LCD pin 11 (D4) to PB0
   LCD pin 12 (D5) to PB1
   LCD pin 13 (D6) to PB2
   LCD pin 14 (D7) to PB3

   LCD pin 15 (LED+) to 5V
   LCD pin 16 (LED-) to GND = RED Backlight
 *
 *
 * See more at
 * https://
 *
 */
#include "main.h"
#include "lcd16x2.h"

extern void delay_us(uint16_t delay);

int lcdInterfaceInit() {
   // As indicated by the header, we'll need to configure 4 GPIO registers to
   // deploy the necessary I/O pins for output operation.
   // Setting the mode register
   // 01: General purpose output mode - for PB0 - PB3 & PB6 & PB7 the 32bit-map
   // PB8 is used for test.
   // will look like this:
   //                 B8B7B6----B3B2B1B0
   // 0b00000000000000010101000001010101 = 0x0015055
   GPIOB->MODER |= 0x15055UL;
   // GPIO port output type: 0 = Output push-pull (reset state)
   //GPIOB_OTYPER |= 0b0000000000000000;

   // GPIO port output speed
   // x0: Low speed

   // 01: Medium speed
   //                 B8B7B6----B3B2B1B0
   // 0b00000000000000010101000001010101 = 0x0015055
   //GPIOB->OSPEEDR |= 0x15055UL;

   // 10: High speed
   //                 B8B7B6----B3B2B1B0
   // 0b00000000000000011010000010101010 = 0x1A0AA
   GPIOB->OSPEEDR |= 0x1A0AAUL;

   // GPIO port pull-up/pull-down register
   // 00: No pull-up, pull-down
   //GPIOB_PUPDR |= 0b0000000000000000;
   // Setting PB8 high to indicate LCD Interface Init.
   GPIOB->ODR |= 0b0000000100000000;

   return 0;
}

void pulseEnable() {
   // pulseEnable
   GPIOB->ODR &= ~0x80; // Enable 0
   delay_us(1);
   GPIOB->ODR |= 0x80; // Enable 1
   delay_us(1);
   GPIOB->ODR &= ~0x80; // Enable 0
   delay_us(100);
}

void lcd4wireHwInit() {
   HAL_Delay(50); // Wait 15 ms after power is applied.
   GPIOB->ODR &= ~0xCf; // RS = 0 (PB6) & EN = 0 (PB7) & D4-D7 = 0 (PB0-PB3)

   GPIOB->ODR |= 0x3; // Set D4-D7 = 0b0011 (PB0-PB3)
   pulseEnable();
   delay_us(4500);

   //GPIOB->ODR |= 0x3; // Set D4-D7 = 0b0011 (PB0-PB3)
   pulseEnable();
   delay_us(4500);

   //GPIOB->ODR |= 0x3; // Set D4-D7 = 0b0011 (PB0-PB3)
   pulseEnable();
   delay_us(150);

   GPIOB->ODR &= ~0x1; // Set D4-D7 = 0b0010 (PB0-PB3)
   pulseEnable();
}

void loadLcdRegister(uint8_t regValue, uint8_t regType) {
   // Loads a 8-bit value into either a Instruction register or a Data register
   // depending on the regType.
   // regType 0 = Instruction register
   // regType 1 = Data register
   if (regType == 1)
      GPIOB->ODR |= 0x80;
   else
      GPIOB->ODR &= ~0x80;

   GPIOB->ODR |= (regValue >> 4) & 0xf;
   pulseEnable();
   GPIOB->ODR |= regValue & 0xf;
   pulseEnable();
}

int lcdConfig() {
   lcd4wireHwInit();
   loadLcdRegister(FUNCTION_SET | FUNCTION_4BIT_BUS | FUNCTION_2LINE_DISPLAY, 0);
   //lcdRegLatch(0x28);

   loadLcdRegister(DISPLAY_SETUP | DISPLAY_SETUP_ON, 0);
   //lcdRegLatch(0x0c);

   loadLcdRegister(ENTRY_MODE | ENTRY_MODE_INCR, 0);
   //lcdRegLatch(0x06);

   return 0;
}

void lcdInit() {
   lcdInterfaceInit();
   lcdConfig();

   // Write a character to the display?
   uint8_t txt[] = {'N', 'U', 'C', 'L', 'E', 'O'};
   for (int i = 0; i < sizeof(txt); i++) {
      loadLcdRegister(txt[i], 1);
      HAL_Delay(1);
   }
}

int string2lcd(uint8_t* sbuf, uint8_t len) {
   if (len > MAX_LEN)
      len = MAX_LEN;
   for (int i = 0; i < len; i++) {
      loadLcdRegister(sbuf[i], 1);
      HAL_Delay(1);
   }

   return len;
}
