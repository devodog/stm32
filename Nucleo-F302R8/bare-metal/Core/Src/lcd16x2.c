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


   LCD pin 4 (RS) to PB0 On NucleoF302R8 CN7[pin34]
   LCD pin 5 (R/W) (grounded)
   LCD pin 6 (EN) to PB1 On NucleoF302R8 CN10[pin24]

   LCD pin 7 (D0) to not used
   LCD pin 8 (D1) to not used
   LCD pin 9 (D2) to not used
   LCD pin 10 (D3) to not used

   LCD pin 11 (D4) to PB4 On NucleoF302R8 CN10[pin27]
   LCD pin 12 (D5) to PB5 On NucleoF302R8 CN10[pin29]
   LCD pin 13 (D6) to PB6 On NucleoF302R8 CN10[pin17]
   LCD pin 14 (D7) to PB7 On NucleoF302R8 CN7[pin21]

   LCD pin 15 (LED+) to 5V
   LCD pin 16 (LED-) to GND = RED Backlight
 *
 */
#include <stdio.h>
//#include <stdint.h>
#include "main.h"
#include "lcd16x2.h"

extern void delay_us(uint16_t delay);

int lcdInterfaceInit() {
   // As indicated by the header, we'll need to configure 4 GPIO registers to
   // deploy the necessary I/O pins for output operation.
   // Setting the mode register
   // 01: General purpose output mode - for PB4 - PB7 & PB0 & PB1 the 32bit-map
   // will look like this:
   //                   D7D6D5D4    E SR
   //                 B8B7B6B5B4    B1B0
   // 0b00000000000000010101010100000101 = 0x15505
   #include "stm32f3xx_hal_gpio.h"

   GPIOB->MODER |= 0x15505U;

   // GPIO port output type: 0 = Output push-pull (reset state)
   GPIOB->OTYPER |= 0b0000000000000000;

   // GPIO port output speed
   // x0: Low speed
   // 01: Medium speed
   // 10: High speed
   //                   D7D6D5D4    E SR
   //                 B8B7B6B5B4    B1B0
   // 0b00000000000000011010101000001010 = 0x1AA0A
   GPIOB->OSPEEDR |= 0x1AA00U;

   // GPIO port pull-up/pull-down register
   // 00: No pull-up, pull-down
   GPIOB->PUPDR |= 0b0000000000000000;
  return 0;
}

void load4BitBus(uint8_t bits) {
   // B7 B6 B5 B4
   GPIOB->ODR &= ~0xf0; //
   GPIOB->ODR |= bits & 0xf0;
}

void pulseEnable() {
   GPIOB->ODR &= ~0x2; // Enable 0
   GPIOB->ODR |= 0x2; // Enable 1
   delay_us(10);
   GPIOB->ODR &= ~0x2; // Enable 0
   delay_us(100);
}

/*
 * lcd4wireHwInit() is to initialize the 4-wire instruction/data bus on the
 * HD44780 display controller.
 * It will load the instructions according to the recommended initialization
 * sequence for the HD44780 display controller's.
 *
 */
void lcd4wireHwInit() {
   HAL_Delay(50); // Wait 50 ms after power is applied - could reduce this time
   // Clear the bits in the GPIO Port B that is used for communication with the
   // LCD, alike Hitachi HD44780 display controller.
   GPIOB->ODR &= ~0xf3; // RS = 0 (PB6) & EN = 0 (PB7) & D4-D7 = 0 (PB0-PB3)

   load4BitBus(0x30); // Set D4-D7 = 0b0011xxxx (PB4-PB7)
   pulseEnable();
   delay_us(4500);

   load4BitBus(0x30); // Set D4-D7 = 0b0011xxxx (PB4-PB7)
   pulseEnable();
   delay_us(4500);

   load4BitBus(0x30); // Set D4-D7 = 0b0011xxxx (PB4-PB7)
   pulseEnable();
   delay_us(150);

   load4BitBus(0x20); // Set D4-D7 = 0b0010xxxx (PB4-PB7)
   pulseEnable();
   delay_us(150);

   // Not sure that this is really needed...
   /****
   load4BitBus(0x20); // Set D4-D7 = 0b0010 (PB4-PB7)
   pulseEnable();
   delay_us(150);
   ***/
}

void loadLcdRegister(uint8_t regValue, uint8_t regType) {
   // Loads a 8-bit value into either a Instruction register or a Data register
   // depending on the regType.
   // regType 0 = Instruction register
   // regType 1 = Data register
   if (regType == 1) {
      GPIOB->ODR |= 0x1;
      //printf("\r\nD=0x%x", (unsigned int)GPIOB->ODR);
   }
   else
      GPIOB->ODR &= ~0x1;

   load4BitBus(regValue);
   pulseEnable();
   load4BitBus((regValue << 4) & 0xf0);
   pulseEnable();
}

int lcdConfig() {
   lcd4wireHwInit();
   loadLcdRegister(FUNCTION_SET | FUNCTION_4BIT_BUS | FUNCTION_2LINE_DISPLAY, 0);
   //loadLcdRegister(0x28);

   loadLcdRegister(DISPLAY_SETUP | DISPLAY_SETUP_ON, 0);
   //loadLcdRegister(0x0c);

   loadLcdRegister(ENTRY_MODE | ENTRY_MODE_INCR, 0);
   //loadLcdRegister(0x06);
   return 0;
}

void lcdInit() {
   //lcdInterfaceInit(); // the bus-interface is configured in the ioc gui.
   lcdConfig();

   // Write a character to the display?
   uint8_t txt[] = {'N', 'U', 'C', 'L', 'E', 'O', ' ', 'b', 'a', 'r', 'e', '-'};
   uint8_t txt2[] = {'m', 'e', 't', 'a', 'l', ' ', '2', '0', '2', '4'};
   for (int i = 0; i < sizeof(txt); i++) {
      loadLcdRegister(txt[i], 1);
      HAL_Delay(1);
   }
   // In a 2-line display, the cursor moves to the second line when it passes
   // the 40th digit of the first line. Note that the first and second line
   // displays will shift at the same time.
   // ref. https://cdn-shop.adafruit.com/datasheets/HD44780.pdf
   //
   for (int i = 0; i < (40-sizeof(txt)); i++) {
      loadLcdRegister(0x20, 1);
      HAL_Delay(1);
   }
   // next line...
   for (int i = 0; i < sizeof(txt2); i++) {
      loadLcdRegister(txt2[i], 1);
      HAL_Delay(1);
   }
}

void lcdClear() {
   loadLcdRegister(LCD_CLEAR, 0);
   HAL_Delay(2);
}  // lcdClear

void lcdHome() {
   loadLcdRegister(LCD_HOME, 0);
   HAL_Delay(2);
}  // lcdHome 

void lcdCursorBack() {
   loadLcdRegister(LCD_CURSOR_BACK, 0);
   HAL_Delay(2);
}  // lcdCursorBack

void lcdClock(uint8_t twentyFour, uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t hundreds) {
   // NOT USABLE FOR MEASURING TIME!
   lcdClear();
   lcdHome();
   for (int i = 0; i < 4; i++) {
      loadLcdRegister(0x20, 1);
      //HAL_Delay(1);
   }
   char sbuf[16] = {0};
   if (twentyFour == 1) {
      sprintf(sbuf, "%02d:%02d:%02d    ", hours, minutes, seconds); // four spaces to center the LCD printout
   }
   else {
      sprintf(sbuf, "%02d:%02d.%02d    ", minutes, seconds, hundreds); // four spaces to center the LCD printout
   }

   for (int i = 0; i < sizeof(sbuf); i++) {
      loadLcdRegister(sbuf[i], 1);
      //HAL_Delay(1);
   }
   return;
}

int string2lcd(uint8_t* sbuf, uint8_t len) {
   lcdClear();
   lcdHome();
   if (len > MAX_LEN)
      len = MAX_LEN;
   for (int i = 0; i < len; i++) {
      loadLcdRegister(sbuf[i], 1);
      HAL_Delay(1);
   }

   return len;
}

int strings4lcd(uint8_t* line1, uint8_t len1, uint8_t* line2, uint8_t len2) {
   lcdClear();
   lcdHome();
   int spaceLen = 0;

   if (len1 > MAX_LEN)
      len1 = MAX_LEN;
   for (int i = 0; i < len1; i++) {
      loadLcdRegister(line1[i], 1);
      HAL_Delay(1);
   }
   spaceLen = 40 - len1;
   for (int i = 0; i < spaceLen; i++) {
         loadLcdRegister(0x20, 1);
         //HAL_Delay(1);
   }

   if (len2 > MAX_LEN)
      len2 = MAX_LEN;
   for (int i = 0; i < len2; i++) {
      loadLcdRegister(line2[i], 1);
      HAL_Delay(1);
   }

   return len1+len2;
}
