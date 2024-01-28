/*
 * lcd16x2.h
 *
 *  Created on: Jan 27, 2024
 *      Author: dagak
 */

#ifndef INC_LCD16X2_H_
#define INC_LCD16X2_H_

// Command set for Hitachi 44780U LCD display controller
#define LCD_CLEAR 0x01    // It clears everythings
#define LCD_HOME 0x02   // set the cursor to first line and first row
#define LCD_PAN_LEFT 0x18       // used to scroll text left side to scroll text
#define LCD_PAN_RIGHT 0x1C   // used to scroll text right side to scroll text

#define LCD_CURSOR_BACK 0x10 // moves curson one position back
#define LCD_CURSOR_FWD 0x14  //moves curson one position forward
#define LCD_CURSOR_OFF 0x0C // stops display curson on screen
#define LCD_CURSOR_ON 0x0E  // turns on cursor display
#define LCD_CURSOR_BLINK 0x0F  // curson keeps blinking
#define LCD_CURSOR_LINE2 0xC0   // move curson to scond line or second row

// display controller setup commands from page 46 of Hitachi datasheet
#define FUNCTION_SET 0x20 //
#define FUNCTION_8BIT_BUS 0x10 //
#define FUNCTION_2LINE_DISPLAY 0x08 // 5x8 font
#define FUNCTION_CHAR_FONT 0x0x4 // 4 bit interface, 2 lines, 5x8 font

#define SET_DDRAM_ADDR 0x20

#define ENTRY_MODE 0x04 // display entry mode
#define ENTRY_MODE_INCR 0x02 // increment mode
#define ENTRY_MODE_SHIFT 0x01 // shift mode

#define DISPLAY_SETUP 0x08 // display setup indication
#define DISPLAY_SETUP_ON 0x04 // display on
#define DISPLAY_SETUP_CURSOR 0x02 // cursor on
#define DISPLAY_SETUP_BLINK 0x01 // blink on

int lcdInterfaceInit(void);
int lcdConfig(void);
void lcdInit(void);

void lcdRegLatch(uint8_t cmd);
void InstuctionRegWrite(uint8_t cmd);
void DataRegWrite(uint8_t data);

#endif /* INC_LCD16X2_H_ */
