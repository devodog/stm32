
/***
 * adafruit16x2LCD398.c
 * This is a driver for the Adafruit 16x2 LCD display
 * It is based on the Hitachi HD44780 display controller
 * It uses a 4-bit interface to the display
 * It is written for the stm32Nucleo-f302r8 microcontroller.
 * 
 * 
    Wiring:
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
    See more at
    https://
 * 
 * 
 */


// Command set for Hitachi 44780U LCD display controller
#define LCD_CLEAR 0x01    // It clears everythings 
#define LCD_HOME 0x02   // set the cursor to first line and first row
#define LCD_CURSOR_BACK 0x10 // moves curson one position back
#define LCD_CURSOR_FWD 0x14  //moves curson one position forward
#define LCD_PAN_LEFT 0x18       // used to scroll text left side to scroll text
#define LCD_PAN_RIGHT 0x1C   // used to scroll text right side to scroll text
#define LCD_CURSOR_OFF 0x0C // stops display curson on screen
#define LCD_CURSOR_ON 0x0E  // turns on cursor display
#define LCD_CURSOR_BLINK 0x0F  // curson keeps blinking
#define LCD_CURSOR_LINE2 0xC0   // move curson to scond line or second row

// display controller setup commands from page 46 of Hitachi datasheet
#define FUNCTION_SET 0x28 // 4 bit interface, 2 lines, 5x8 font
#define ENTRY_MODE 0x06 // increment mode
#define DISPLAY_SETUP 0x0C // display on, cursor off, blink off

// These #defines create the pin connections to the LCD in case they are changed on a future demo board
/*
#define LCD_PORT PORTD
#define LCD_PWR PORTDbits.RD7 // LCD power pin
#define LCD_EN PORTDbits.RD6 // LCD enable
#define LCD_RW PORTDbits.RD5 // LCD read/write line
#define LCD_RS PORTDbits.RD4 // LCD register select line
*/

void LCD_Initialize()
{
    LCD_PORT = 0; // clear latches before enabling TRIS bits
    TRISD = 0x00;  
    LCD_PWR = 1; // power up the LCD
    __delay_ms(LCD_Startup); // required by display controller to allow power to stabilize
    LCDPutCmd(0x32); // required by display initialization
    LCDPutCmd(FUNCTION_SET); // set 4-bit interface mode and 16x2 of lines and font
    LCDPutCmd(DISPLAY_SETUP); // turn on display and sets up cursor
    DisplayClr(); // clear the display
    LCDPutCmd(ENTRY_MODE); // set cursor movement direction to increment and display shift to off
}

void LCDPutCmd() // send command to display controller
{
    LCD_RS = 0; // set for command
    LCD_EN = 1; // take LCD enable line high
    LCD_PORT = cmd; // put command on output port
    LCD_EN = 0; // take LCD enable line low
    __delay_ms(1); // wait
}