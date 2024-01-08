# Development notes for STM32NUCLEO-F302R8 development board
This repository is to contain small code fragments for the STM32NUCLEO-F302R8 development board to understand / get used to the STM32CubeIDE and to make use of some of the features in the STM32-F302R8 device.  
The study/invetigation will start with making the LD2 blink. Thereafter make a bidirectional serial line communication that provides a simple command-line terminal user interface from a PC (like PUTTY or TeraTerm).  

This command-line interface should then be extended to provide more and more functions from the micro controller unit.

## Connecting the STM32NUCLEO-F302R8 development board to the softare development environment.  
Should be quite simple, but if you have got your hands on a used device, you must ensure that the power (PWR) strap selector is set to U5V (USB 5V supply) in order for the ST-LINK to "see" the STM32-F302R8 chip. 

![NUCLEO-F302R8-PWR-Settings](images\NUCLEO-F302R8-PWR-Settings.png "Text to show on mouseover")  

## Blinking the LD2
What to do...
1. Select the GPIO pion that is physical connected to the LD2.
2. Make the selected GPIO output an output pin.
3. To make the LD2 LED to blink, we have at least two options.  
a. Use the HAL (Hardware Abstraction Layer) functions: HAL_GPIO_TogglePin() + a HAL_Delay() placed in a never ending loop  
b. Use the HAL functions: HAL_GPIO_Write(1) + HAL_Delay() + HAL_GPIO_Write(0) + HAL_Delay() placed in a never ending loop  

This is fairly simple when useing the hardware configurator in the STM32CubeIDE,

It is observed that the sw dev tool generates a lot of code and, the first thing noticed is the assembly start-up code startup_stm32f302r8tx.s 
