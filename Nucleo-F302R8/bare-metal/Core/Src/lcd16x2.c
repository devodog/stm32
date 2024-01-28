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
   LCD pin 16 (LED-) to GND
 *
 *
 * See more at
 * https://
 *
 */
#include "main.h"
#include "lcd16x2.h"

int lcdInterfaceInit() {
   // As indicated bu the header, we'll need to configure 4 GPIO registers to
   // deploy the necessary I/O pins for output operation.
   // Setting the mode register
   // 01: General purpose output mode - for PB0 - PB3 & PB6 & PB7 the 32bit-map
   // will look like this: 0b00000000000000000000000000000000
   //                                        B7  B5B4------B0
   //                      0b00000000000000000101000001010101 = 0x0005055
   GPIOB->MODER |= 0x5055;
   //GPIOB_MODER |= 0x5055;
   // GPIO port output type: 0 = Output push-pull (reset state)
   //GPIOB_OTYPER |= 0b0000000000000000;

   // GPIO port output speed
   // 01: Medium speed
   GPIOB->OSPEEDR |= 0x5055;

   // GPIO port pull-up/pull-down register
   // 00: No pull-up, pull-down
   //GPIOB_PUPDR |= 0b0000000000000000;


   return 0;
}
void lcdRegLatch(uint8_t cmd) {
   // Check the timing sequence for the 4-bit data transfer!
   // The EN pin is the 'enable' line we use this to tell the LCD when data is ready for reading.
   // Clear the 4 least significant bits of the output (16-bit) B register.

   GPIOB->ODR &= ~0b0000000000001111; // Has to be done prior every tuple transfer
   // Set first most significant nibble on the 4-bit data-bus.
   GPIOB->ODR |= (cmd >> 4) & 0xf;
   // Set Enable (PB7) "high" to latch the first most significant bits into the instruction or data register.
   GPIOB->ODR |= 0b0000000010000000;
   // Delay for min. 1us.

   // set Enable (PB7) "low" -> ready the lower ordet nibble into instruction or data register.
   GPIOB->ODR &= ~0b0000000010000000;
   // delay?

   // Clear the 4 least significant bits of the output B register.
   GPIOB->ODR &= ~0b0000000000001111;

   // Load the lower order nibble on the 4-bit data-bus.
   GPIOB->ODR |= cmd & 0xf;
   // Set Enable (PB7) "high" to latch the least significant bits into the instruction or data register.
   GPIOB->ODR |= 0b0000000010000000;
   // Delay for min. 1us.

   // set Enable (PB7) "low" -> will latch inn the high order nibble into the command register.
   GPIOB->ODR &= ~0b0000000010000000;
   // We're done...
}

int lcdConfig() {
   /**
    *
    LCD_PORT = 0; // clear latches before enabling TRIS bits
    TRISD = 0x00;
    LCD_PWR = 1; // power up the LCD
    __delay_ms(LCD_Startup); // required by display controller to allow power to stabilize
    LCDPutCmd(0x32); // required by display initialization
    LCDPutCmd(FUNCTION_SET); // set 4-bit interface mode and 16x2 of lines and font
    LCDPutCmd(DISPLAY_SETUP); // turn on display and sets up cursor
    DisplayClr(); // clear the display
    LCDPutCmd(ENTRY_MODE); // set cursor movement direction to increment and display shift to off
    *
    */

   GPIOB->ODR &= ~(0b0000000010000000);
   // Power always on...
   lcdRegLatch(FUNCTION_SET | FUNCTION_8BIT_BUS); //required by display initialization??
   HAL_Delay(5);
   lcdRegLatch(FUNCTION_SET | FUNCTION_8BIT_BUS); //required by display initialization??
   HAL_Delay(1);

   lcdRegLatch(FUNCTION_SET | FUNCTION_2LINE_DISPLAY);
   lcdRegLatch(DISPLAY_SETUP | DISPLAY_SETUP_ON);
   lcdRegLatch(ENTRY_MODE | ENTRY_MODE_INCR);

   return 0;
}

void lcdInit() {
   lcdInterfaceInit();
   lcdConfig();
   // Write a character to the display?
   uint8_t txt[] = {'N', 'U', 'C', 'L', 'E', 'O'};
   for (int i = 0; i < sizeof(txt); i++) {
      DataRegWrite('N');
      HAL_Delay(1);
   }
}

void InstuctionRegWrite(uint8_t cmd) {
   // RS = 0 (PB6)
   lcdRegLatch(cmd);
}

void DataRegWrite(uint8_t data) {
   // RS = 1 (PB6) to enable data register load...
   GPIOB->ODR |= 0b0000000001000000;
   //
   lcdRegLatch(data);
   //
   GPIOB->ODR &= ~0b0000000001000000;
}
