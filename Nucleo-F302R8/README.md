! Be aware - this document is new and in the making... 
# Development notes for STM32NUCLEO-F302R8 development board
This repository is to contain small code fragments for the STM32NUCLEO-F302R8 development board to understand / get used to the STM32CubeIDE and to make use of some of the features in the STM32-F302R8 device.  
The study/investigation will start with making the LD2 blink. Thereafter make a bidirectional serial line communication that provides a simple command-line terminal user interface from a PC (like PUTTY or TeraTerm).  

This command-line interface should then be extended to provide more and more functions from the micro controller unit.

## Connecting the STM32NUCLEO-F302R8 development board to the software development environment.    
Should be quite simple, but if you have got your hands on a used device, you must ensure that the power (PWR) strap selector is set to U5V (USB 5V supply) in order for the ST-LINK to "see" the STM32-F302R8 chip. 

<img src="images/NUCLEO-F302R8-PWR-Settings.png" height="350">  
  

Make also sure that the strap is on the CN3 pin row, connecting the ST-LINK to the STM32-F302R8 chip SWD interface.

<img src="images/ST-LINK-selector.png" height="350">  


## Blinking the LD2
What to do...
1. Select the GPIO pion that is physical connected to the LD2.
2. Make the selected GPIO output an output pin.
3. To make the LD2 LED to blink, we have at least two options.  
a. Use the HAL (Hardware Abstraction Layer) functions: HAL_GPIO_TogglePin() + a HAL_Delay() placed in a never ending loop  
b. Use the HAL functions: HAL_GPIO_Write(1) + HAL_Delay() + HAL_GPIO_Write(0) + HAL_Delay() placed in a never ending loop  

This is fairly simple when using the hardware configurator in the STM32CubeIDE,

It is observed that the sw dev tool generates a lot of code and, the first thing noticed is the assembly start-up code startup_stm32f302r8tx.s 

## Pre-compile steps for time-stamping and numbering the latest build
It is usually informative to include some build data in the executable (binary) code, at least for some. For this reason, an include file, appver.h, has been added to the project to contain the last build date and time in addition to the build number. In the project properties and settings, two pre-compile steps are registered. The first is to run a simple python script that will update the   appver.h file with the new date and time immediately before building the executable, and next step tis to move/copy the update appver.h fil into the sourc-code path to be part of the build process.  

<img src="images/pre-build-actions.png" height="350">  

It should be noted that the python script, __prebuild.py__, and the __appver.h__ file has to be located in the debug folder, that is: bare-metal\Core\Debug\*.*  

## USART to RS232 communication
The STM32-F302R8 chip contains 3 USART blocks, USART1, USART2 and USART3.  
As USART1 is considered to be the first alternative for reprogram the flash memory, ref. RM0365 Rev 8 p61/1086, we'll use USART2 for MCU serial communication.  

Configuring the STM32-F302R8 to deploy USART2 for 9600 baud, 8-bit payload, 1 start bit and 1 stop bit to comply with the default settings of the PUTTY program on Win10. The Rx and Tx pins for USART2 is found at GPIO pin PC4(TX) and PC5(RX) seen in the pictures below.

<img src="images/UART-to-RS232.png" height="350"><img src="images/ExtensionConnectors.PNG" height="350">


The STM32 software developemt (kit) environment includes a Hardware Abstraction Layer (HAL) for all of its MCUs, which means that the HAL provides a set of functions for most of the hardware for initialization, read and write access and interrupt handling.  

### USART Transmission
For USART2, the __HAL_UART_Transmit()__ will transmit any number of bytes to the TX pin (PC4), but to make a character based serial line user interface, we'll have to send one character at a time in order to confirm to the user that each character has been received correctly. In addition to this, it will be further convenient to map the __HAL_UART_Transmit()__ to the c function __printf()__, which is commonly used i many console applications.

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
A traditional character based serial line user interface waits for single keyboard events and prints its value onto the output console.  
This means that the MCU must either do extensive polling to be able to catch the keyboard events, or have a interrupt service routine to be executed whenever a key on the keyboard is pressed.
We'll make use of a interrupt service routine for the particular USART device by including the ```AL_UART_RxCpltCallback```,  which is triggered by explicitly registering repeatedly RX interrupt ```HAL_UART_Receive_IT()```.  

In the callback routine we'll check if the received character is a carriage-RETURN, which indicates that the user command-line input is completed and is to be interpreted as a user command. Otherwise will the callback store the received characters one-by-one in a command-line buffer, which is parsed in command executioner function.  
The following code implements the above functionality.  

```
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

char termInputBuffer[80];
int bytesReceived = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (UART1_rxBuffer == 13) {
    if (bytesReceived > 0) {
	  // The user command is complete and is to be executed... - this should actually be a Deferred Procedure Call (DPC),
	  // in order to avoid holding the isr too long. But, for now this is ok, since there are only one isr for the moment.
      executeCmd(&termInputBuffer[0], bytesReceived); 
      bytesReceived = 0;
      memset(termInputBuffer, 0, 80);
    } else {
      promt(); // Output the command-line prompt to indicate that the user interface is ready for a new command.
    }
      // re-trigger the interrupt service callback...
	  HAL_UART_Receive_IT(&huart1, &UART1_rxBuffer, 1);
      return;
  }
  // Echo back the received printable character.
  HAL_UART_Transmit(&huart1, &UART1_rxBuffer, 1, 100);
  
  // Store the received character into the terminal input buffer.
  termInputBuffer[bytesReceived] = UART1_rxBuffer;
  bytesReceived++; // Update the buffer index...
  
  // re-trigger the interrupt service callback...
  HAL_UART_Receive_IT(&huart1, &UART1_rxBuffer, 1);
}
```
  
## Command-line interface
To Be Described  
Keywords  
Command parser and interpretor  
### User commands
Command-line contains a command word and 0 or parameters combined with 0 or more switches in the form of a dash (-) and a alphanumeric letter.  
The command-line parser will separate the command and parameters in order to interpret the user command and execute the appropriated operation.  

__How to design a command-line interface?__  
Bottom-UP: Identify a set of related operations that are to be accessible to the user.  

Example: Imagine that we want to control a LED and decide that it can be set in 3 states, ON, OFF and BLINKING.
This decision may lead us to define the command word to be LED and its parameter for the first two steps, should be either ON or OFF. But should this command set also support the command-line with no parameters? - user only entering the command word LED.  

For this command word, it would be natural to implement and show the LED status, if no parameters are entered with the command word.  
This means that we'll have to make one or more functions that can service the LED command-set.  

Is there a command-line implementation pattern? Not that many... We'll make a simple implementation ourself.  

Possible strategy is as follows:  
 - Make a command-structure for each command(-word)  
 - The command struct is to contain the command-word (string) and all the necessary parameters (list of strings)  
```
#define COMMAND_PARAMS 10
#define COMMAND_PARAM_LENGTH 10
#define COMMAND_NAME_LENGTH 20
#define NUMBERS_OF_MCU_COMMANDS 2

struct command {
  char name[COMMAND_NAME_LENGTH];  // the command name
  int params;     // room for params numbers of command-parameters
  int maxParamLength; // max param word length.
  char paramWords[COMMAND_PARAMS][COMMAND_PARAM_LENGTH];
  int paramValues[COMMAND_PARAMS];
  void (*cmdFunction)(char*); // the command support function...
};

void LED(char* paramStr){
	printf("%s", paramStr);
}

void dummy(char* paramStr){
	printf("DUMMY\r\n");
}


struct command mcuCmds [NUMBERS_OF_MCU_COMMANDS] = {
  {"LED", 3, 6, {"ON", "OFF", "BLINK"}, {0, 1, 500 }, &LED},
  {"DUMMY", 2, 6, {"TRUE", "FALSE"}, {0, 0}, &dummy}
};
```
is this the best way of handling commands entered in a command-line user interface ??
