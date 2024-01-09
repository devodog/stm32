! Be aware - this document is new and in the making... 
# Development notes for STM32NUCLEO-F302R8 development board
This repository is to contain small code fragments for the STM32NUCLEO-F302R8 development board to understand / get used to the STM32CubeIDE and to make use of some of the features in the STM32-F302R8 device.  
The study/invetigation will start with making the LD2 blink. Thereafter make a bidirectional serial line communication that provides a simple command-line terminal user interface from a PC (like PUTTY or TeraTerm).  

This command-line interface should then be extended to provide more and more functions from the micro controller unit.

## Connecting the STM32NUCLEO-F302R8 development board to the softare development environment.  
Should be quite simple, but if you have got your hands on a used device, you must ensure that the power (PWR) strap selector is set to U5V (USB 5V supply) in order for the ST-LINK to "see" the STM32-F302R8 chip. 

![NUCLEO-F302R8-PWR-Settings](images/NUCLEO-F302R8-PWR-Settings.png "Text to show on mouseover")  

## Blinking the LD2
What to do...
1. Select the GPIO pion that is physical connected to the LD2.
2. Make the selected GPIO output an output pin.
3. To make the LD2 LED to blink, we have at least two options.  
a. Use the HAL (Hardware Abstraction Layer) functions: HAL_GPIO_TogglePin() + a HAL_Delay() placed in a never ending loop  
b. Use the HAL functions: HAL_GPIO_Write(1) + HAL_Delay() + HAL_GPIO_Write(0) + HAL_Delay() placed in a never ending loop  

This is fairly simple when useing the hardware configurator in the STM32CubeIDE,

It is observed that the sw dev tool generates a lot of code and, the first thing noticed is the assembly start-up code startup_stm32f302r8tx.s 

## Pre-compile steps for time-stamping and numbering the latest build
It is usually informative to include some build data in the executable (binary) code, at least for some. For this reason, an include file, appver.h, has been added to the project to contain the last build date and time in addition to the build number. In the project properties and settings, two pre-compile steps are registered. The first is to run a simple python script that will update the   appver.h file with the new date and time immediatly before bulding the executable, and next step tis to move/cpoy the update appver.h fil into the sourc-code path to be part of the build process.  

<img src="images/pre-build-actions.png" height="400">  

It should be noted that the python script, __prebuild.py__, and the __appver.h__ file has to be located in the debug folder, that is: bare-metal\Core\Debug\*.*  

## USART to RS232 communication
The STM32-F302R8 chip contains 3 USART blocks, USART1, USART2 and USART3.  
As USART1 is considered to be the first alternative for reprogram the flash memory, ref. RM0365 Rev 8 p61/1086, we'll use USART2 for MCU serial communication.  

Configuring the STM32-F302R8 to deploy USART2 for 9600 baud, 8-bit payload, 1 start bit and 1 stop bit to comply with the defalt settings of the PUTTY program on Win10. The Rx and Tx pins for USART2 is found at GPIO pin PC4(TX) and PC5(RX) seen in the pictures below.

<img src="images/UART-to-RS232.png" height="400">
<img src="images/ExtensionConnectors.PNG" height="400">


The STM32 software developemt (kit) environment includes a Hardware Abstraction Layer (HAL) for all of its MCUs, which means that the HAL provides a set of functions for most of the hardware for initialization, read and write access and interrupt handling.  

### USART Transmission
For USART2, the __HAL_UART_Transmit()__ will transmitt any number of bytes to the TX pin (PC4), but to make a character based serial line user interface, we'll have to send one character at a time in order to confirm to the user that each character has been received correctly. In addition to this, it will be further convinient to map the __HAL_UART_Transmit()__ to the c function __printf()__, which is commonly used i many console applications.

```
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
int _write(int fd, char *ptr, int len) {
	HAL_UART_Transmit(&huart1, (uint8_t*) ptr, len, 1000);
	return len;
}
/* USER CODE END PFP */
```

The code above will map a particular implementation (single byte/character implementation) of the ```HAL_UART_Transmit()``` to the ```printf()``` function.  

```__io_putchar()``` could also be an alternative to ```_write()```.

### USART Reception
A traditional character based serial line user interface waits for single keyboard events and prints this in the output console.  
This means that the MCU must either do extencive polling to be able to chatch the keyboard events, or have a interrupt service routine to be executed whenver a key on the keyboard is pressed.
  
## Command-line interface
TBD  
