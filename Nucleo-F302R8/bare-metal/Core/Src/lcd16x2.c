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

   Be careful, there are already solder bridges and used connections
   on the NUCLEO Board

   PC0 - SB51 - A5 - PB8
   PC1 - SB56 - A4 - PB9
   PB3 - D3 - SWO - CN4.6 (SWD header)


   LCD pin 4 (RS) to PB6 --> PB0
   LCD pin 5 (R/W) (grounded)
   LCD pin 6 (EN) to PB7 --> PB1

   LCD pin 7 (D0) to not used
   LCD pin 8 (D1) to not used
   LCD pin 9 (D2) to not used
   LCD pin 10 (D3) to not used

   LCD pin 11 (D4) to PB0 --> PB4
   LCD pin 12 (D5) to PB1 --> PB5
   LCD pin 13 (D6) to PB2 --> PB6
   LCD pin 14 (D7) to PB3 --> PB7

   LCD pin 15 (LED+) to 5V
   LCD pin 16 (LED-) to GND = RED Backlight
 *
 *
 *
 * See more at
 * https://
 *
 */
#include "stdio.h"
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
   //                 B8B7B6B5B4B3B2B1B0
   // 0b00000000000000010101000001010101 = 0x0015055
   //GPIOB->MODER |= 0x15055UL;
   // ----------------------------------------------
   //                   D7D6D5D4    E SR
   //                 B8B7B6B5B4    B1B0
   // 0b00000000000000010101010100000101 = 0x15505
   //GPIOB->MODER |= 0x15500U;

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
   //GPIOB->OSPEEDR |= 0x1A0AAUL;
   // ----------------------------------------------
   //                   D7D6D5D4    E SR
   //                 B8B7B6B5B4    B1B0
   // 0b00000000000000011010101000001010 = 0x1AA0A
   //GPIOB->OSPEEDR |= 0x1AA00U;

   // GPIO port pull-up/pull-down register
   // 00: No pull-up, pull-down
   //GPIOB->PUPDR |= 0b0000000000000000;
/*** Left inside to indicate that there are many ways to access the GPIO output ports...
 * The GPIO is configured through the ioc-graphical user interface...
 **/
  return 0;
}

void load4BitBus(uint8_t bits) {
   // B7 B6 B5 B4
   GPIOB->ODR &= ~0xf0; //
   GPIOB->ODR |= bits & 0xf0;
}

void pulseEnable() {
   GPIOB->ODR &= ~0x2; // Enable 0
   delay_us(1);
   GPIOB->ODR |= 0x2; // Enable 1
   delay_us(1);
   GPIOB->ODR &= ~0x2; // Enable 0
   delay_us(100);
}

void lcd4wireHwInit() {
   HAL_Delay(50); // Wait 50 ms after power is applied.
   // Clear the bits in the GPIO Port B that is used for communication with the
   // LCD, alike Hitachi HD44780 display controller.
   GPIOB->ODR &= ~0xf3; // RS = 0 (PB6) & EN = 0 (PB7) & D4-D7 = 0 (PB0-PB3)

   load4BitBus(0x30); // Set D4-D7 = 0b0011 (PB4-PB7)
   pulseEnable();
   delay_us(4500);
   //printf("\r\nODR=0x%x", (unsigned int)GPIOB->ODR);

   load4BitBus(0x30); // Set D4-D7 = 0b0011 (PB4-PB7)
   pulseEnable();
   delay_us(4500);
   //printf("\r\nODR=0x%x", (unsigned int)GPIOB->ODR);
   load4BitBus(0x30); // Set D4-D7 = 0b0011 (PB4-PB7)
   pulseEnable();
   delay_us(150);
   //printf("\r\nODR=0x%x", (unsigned int)GPIOB->ODR);

   load4BitBus(0x20); // Set D4-D7 = 0b0010 (PB4-PB7)
   pulseEnable();
   delay_us(150);
   //printf("\r\nODR=0x%x", (unsigned int)GPIOB->ODR);
   load4BitBus(0x20); // Set D4-D7 = 0b0010 (PB4-PB7)
   pulseEnable();
   delay_us(150);
   //printf("\r\nODR=0x%x", (unsigned int)GPIOB->ODR);
   // The 4-bit interface should now be operative...
   //printf("\r\nODR=0x%x", GPIOB->ODR);
}

void loadLcdRegister(uint8_t regValue, uint8_t regType) {
   // Loads a 8-bit value into either a Instruction register or a Data register
   // depending on the regType.
   // regType 0 = Instruction register
   // regType 1 = Data register
   if (regType == 1) {
      GPIOB->ODR |= 0x1;
      printf("\r\nD=0x%x", (unsigned int)GPIOB->ODR);
   }
   else
      GPIOB->ODR &= ~0x1;

   load4BitBus(regValue);
   //printf("\r\nHO=0x%x", (regValue >> 4) & 0xf);
   pulseEnable();
   load4BitBus((regValue << 4) & 0xf0);
   //printf("\r\nLO=0x%x", regValue  & 0xf);
   pulseEnable();
}

int lcdConfig() {
   lcd4wireHwInit();
   loadLcdRegister(FUNCTION_SET | FUNCTION_4BIT_BUS | FUNCTION_2LINE_DISPLAY, 0);
   //printf("\r\nInst: 0x%02x", FUNCTION_SET | FUNCTION_4BIT_BUS | FUNCTION_2LINE_DISPLAY);

   loadLcdRegister(DISPLAY_SETUP | DISPLAY_SETUP_ON, 0);
   //loadLcdRegister(0x0c);
   //printf("\r\nInst: 0x%02x", DISPLAY_SETUP | DISPLAY_SETUP_ON);

   loadLcdRegister(ENTRY_MODE | ENTRY_MODE_INCR, 0);
   //loadLcdRegister(0x06);
   //printf("\r\nInst: 0x%02x", ENTRY_MODE | ENTRY_MODE_INCR);
   return 0;
}

void lcdInit() {
   //lcdInterfaceInit(); // the bus-interface is configured in the ioc gui.
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
