# March 2024 challenge.
## Legacy 7-Segment display with CMOS (15V) hardware driver
Reverse engineering of an Two digit 7-Segment display element without any documentation.  
The hardware will need 12V to illuminate the display segments. See the relevant data sheet for LA4141R-82.  
<img src="pictures/DSC_0088.JPG" height="400">  

Each 7-Segment display element is controlled by 8-bit serial to paralell shift register. The 7 bits of the 8-bit paralell output from this register is connected to a High-Current Darlington Transistor Array (ULN2003) that powers the display segments through a 220 ohm resistor array from 4100 Bourns.  

Since the two digit display has two individual display elements, there are two 8-bit serial to paralell shift register connected in cascade. The first shift register is to illuminate the highest value digit, where as the second cascade connected shift register is to illuminate the lowest value digit. This will require that the serial data for both of these display elements are sent in one 2-byte sequence, where the low value is entered first, as the first byte of the sequence, followed by the high value 7-segment data. 

The serial data received by the first shift register will output the serial data on the serial output pin, which is connected to the second shift registers data input pin. Both clock and strobe are shared between the shift registeres. 

The first shift register's clock, data and strobe lines are connected to the output CMOS Hex Schmitt-Trigger Inverters. The input of these invertes are connected directly to the display element's 9-pin D-Sub female connector in the following order.  

- D-Sub pin 3 = strobe
- D-Sub pin 4 = data
- D-Sub pin 5 = clock

Using a mcu to provide proper and sufficient serial data for display illumination, a TTL to CMOS 12V levelconverter is needed, since the display element's logic is CMOS 12V based.

One simple solution is to use one npn transistor for each control line, making a 3.3V level to a 12V logical "high".

THE DESIGN TO BE SHOWN HERE.


## The software driver
The driver is written in c and deployed on the STM32 MCU platform for a specific NucleoF302R8 development board.
The integrated development environment is the STM32CubeIDE from ST Electronics and utilizing the Hardware Abstraction Layer library.

The following development boards GPIOs are used to interface the double 7-Segment display unit:  
- D-Sub pin 3 = strobe = STM32 Nucleo F302R8 board GPIO PC2  
- D-Sub pin 4 = data = STM32 Nucleo F302R8 board GPIO PC1  
- D-Sub pin 5 = clock = STM32 Nucleo F302R8 board GPIO PC0  
12V and GND is connected to pins....   

#### 9-pin D-Sub cable
- Red    = pin 1
- Yellow = pin 2
- Blue   = pin 3
- White  = pin 4   
- Black  = pin 5
- Orange = pin 6
- Pink   = pin 7
- Brown  = pin 8
- Green  = pin 9


### Transmission speed
Driver to provide clock signal, serial data (paralell to serial) and enable output parallel data.  
What would be de minimum frequency for the Clock signal? This will surly depend on the what kind of functionality this display is to provide.
For instans, using it for time measurments as a stopwatch, the time resolution will set the transmission speed requirements.  

Lets assume it is sufficient to update the display after 1 ms, or a few.  
If we have 4 7-segment display elements or digits, it will need at least 4 x 8 = 32 clock cycles, which will give a clock signal period equal to
1 ms / 32 = 31.25 µs => 32 kHz.  

Driver implemented on STM32 Nucleo F302R8 board.  
Slow timing used. Will need to check this and eventually optimize.  
```
for (int i = 0; i < 8; i++) {
    // msb (most significant bit) on the line first.
    // The bits enter into the least significant bit of the shift register and  
    // will be shifted towards the most significant bit.

    sLine = (ssCode[digit] >> (7-i)) & 0x1;
    // Data on sData_Pin
    HAL_GPIO_WritePin(GPIOC, sData_Pin, sLine); //PC1 <=> D-SUB#4 = Orange&White = DATA
    HAL_Delay(delay);

    // Clock goes HIGH latching the data Neg. Logic
    HAL_GPIO_WritePin(GPIOC, sClk_Pin, GPIO_PIN_SET); //PC0 <=> D-SUB#5 = Green = CLK
    HAL_Delay(delay);
    //
    HAL_GPIO_WritePin(GPIOC, sClk_Pin, GPIO_PIN_RESET);
    HAL_Delay(delay);
}
```
We will probably need a custom made delay to make a the transmission sufficienty fast.  

### Stop watch functionality
- Start measurement
- Stop Measurement
- Reset  
All of these events must trigger an interrupt service routine.  
Can use one for both start and stop, and a second one for reset.  

### Running display during time measurment
Update every 0.1 sec?






 