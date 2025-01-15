# Configurator for MCF8316A UiA custom development board 
Using STMicroelectrionic Nucleo-F302R8 development board for providing serial 
command line interface for accessing the MCF8316A chip configuration and 
driver registers through I2C interface. 

## Commands available
```
led <on|off|state>
- Setting the Nucleo dev kit LED on or of and retrieving its state as well.

drv <on|off|state>
- Setting the MCF8316A Coast (Hi-Z) all six MOSFETs when drv = off (DRVOFF is high).
 
brk <on|off|state>
- Setting motor brake condition.
 
dir <on|off|state>
- Setting the motor rotation condition.
 
i2c <get|set<i2c-addr[dec]>>
- Setting the i2c slave address of the MCF8316A
 
sys
- Listing the version and build information.


EEPROM <READ <ALL|reg-addr[hex]> | WRITE <DEFAULT1|DEFAULT2|reg.name <reg-value[hex]>>
- Reading and writing data from/to the non-volatile EEPROM MCF8316A memory.

RAM <READ <ALL|FAULT <GATE|CTRL>|volatile reg. name>>
- Reading data from the volatile MCF8316A memory.

## Algorithm control registers for debug - see data sheet for details.  
```
### ALGO_CTRL1 <SET|(GET)>
```
|bit|  |r/w|   |Func. Name|  
===========================
[31] 	[W] 	OVERRIDE   
	- Use to control the SPD_CTRL bits. If OVERRIDE = 1b, speed command can be written by the user through serial interface.  
	0 = SPEED_CMD using Analog/PWM/Freq mode  
	1 = SPEED_CMD using SPD_CTRL[14:0]  

[30-16] [W] 	DIGITAL_SPEED_CTRL, bit30-16 = SPD_CTRL[14:0] [W]   
	- Digital speed control If OVERRIDE = 1b, then SPEED_CMD is control using DIGITAL_SPEED_CTRL
	
[15] 	[W] 	CLOSED_LOOP_DIS  
	- Use to disable closed loop  
	0h = Enable Closed Loop  
	1h = Disable Closed loop, motor commutation in open loop  

[14] 	[W] 	FORCE_ALIGN_EN  
	- Force align state enable  
	0h = Disable Force Align state, device comes out of align state if MTR_STARTUP is selected as ALIGN or DOUBLE ALIGN  
	1h = Enable Force Align state, device stays in align state if MTR_STARTUP is selected as ALIGN or DOUBLE ALIGN  

[13] 	[W] 	FORCE_SLOW_FIRST_CYCLE_EN  
	- Force slow first cycle enable  
	0h = Disable Force Slow First Cycle state, device comes out of slow first cycle state if MTR_STARTUP is selected as SLOW FIRST CYCLE  
	1h = Enable Force Slow First Cycle state, device stays in slow first cycle state if MTR_STARTUP is selected as SLOW FIRST CYCLE  

[12]	[W] 	FORCE_IPD_EN   
	- Force IPD enable  
	0h = Disable Force IPD state, device comes out of IPD state if MTR_STARTUP is selected as IPD  
	1h = Enable Force IPD state, device stays in IPD state if MTR_STARTUP is selected as IPD  
	
[11]	[W] 	FORCE_ISD_EN   
	- Force ISD enable  
	0h = Disable Force ISD state, device comes out of ISD state if ISD_EN is set  
	1h = Enable Force ISD state, device stays in ISD state if ISD_EN is set  
	
[10]	[W] 	FORCE_ALIGN_ANGLE_SRC_SEL  
	- Force align angle state source select  
	0h = Force Align Angle defined by ALIGN_ANGLE  
	1h = Force Align Angle defined by FORCED_ALIGN_ANGLE  
	 
[9-0]	[W] 	FORCE_IQ_REF_SPEED_LOOP_DIS  
	- Sets Iq_ref when speed loop is disabled If SPEED_LOOP_DIS = 1b,   
	  then Iq_ref is set using IQ_REF_SPEED_LOOP_DIS Iq_ref = (FORCE_IQ_REF_SPEED_LOOP_DIS / 500) * 10,  
	  if FORCE_IQ_REF_SPEED_LOOP_DIS < 500 - (FORCE_IQ_REF_SPEED_LOOP_DIS - 512) / 500 * 10,   
	  if FORCE_IQ_REF_SPEED_LOOP_DIS > 512 Valid values are 0 to 500 and 512 to 1000  

-----------------------------------------------------------------  
 3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0  
 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  
|1|0|0|0|0|0|0|0|0|1|0|0|0|0|0|0|1|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|  

OVERRIDE = 1  
DIGITAL_SPEED_CTRL = 0x40 = 64  
CLOSED_LOOP_DIS = 1  
FORCE_ALIGN_EN = ?  
FORCE_SLOW_FIRST_CYCLE_EN  
FORCE_IPD_EN  
FORCE_ISD_EN  
FORCE_ALIGN_ANGLE_SRC_SEL  
FORCE_IQ_REF_SPEED_LOOP_DIS  
-----------------------------------------------------------------
```  

### ALGO_CTRL2 <SET|GET>

```
|bit|  |r/w|   |Func. Name|  
===========================

[31-27] RESERVED

[26] CURRENT_LOOP_DIS 
	- Use to control the FORCE_VD_CURRENT_LOOP_DIS and FORCE_VQ_CURRENT_LOOP_DIS. If CURRENT_LOOP_DIS = 1b, current loop and speed loop are disabled
	0h = Enable Current Loop
	1h = Disable Current Loop
	
[25-16] FORCE_VD_CURRENT_LOOP_DIS
	- Sets Vd_ref when current loop and speed loop are disabled If CURRENT_LOOP_DIS = 1b, 
	then Vd is controlled using FORCE_VD_CURRENT_LOOP_DIS Vd_ref = (FORCE_VD_CURRENT_LOOP_DIS / 500) 
	if FORCE_VD_CURRENT_LOOP_DIS < 500 - (FORCE_VD_CURRENT_LOOP_DIS - 512) / 500 
	if FORCE_VD_CURRENT_LOOP_DIS > 512 Valid values: 0 to 500 and 512 to 1000
	
[15-6] FORCE_VQ_CURRENT_LOOP_DIS
	- Sets Vq_ref when current loop speed loop are disabled If CURRENT_LOOP_DIS = 1b, 
	then Vq is controlled using FORCE_VQ_CURRENT_LOOP_DIS Vq_ref = (FORCE_VQ_CURRENT_LOOP_DIS / 500) 
	if FORCE_VQ_CURRENT_LOOP_DIS < 500 - (FORCE_VQ_CURRENT_LOOP_DIS - 512) / 500 
	if FORCE_VQ_CURRENT_LOOP_DIS > 512 Valid values: 0 to 500 and 512 to 1000
	
[5] MPET_CMD
	- Initiates motor parameter measurement routine when set to 1b
	
[4] MPET_R
	- Enables motor resistance measurement during motor parameter measurement routine
	0h = Disable Motor Resistance measurement during motor parameter measurement routine
	1h = Enable Motor Resistance measurement during motor parameter measurement routine

[3] MPET_L
	- Enables motor inductance measurement during motor parameter measurement routine
	0h = Disable Motor Inductance measurement during motor parameter measurement routine
	1h = Enable Motor Inductance measurement during motor parameter measurement routine
	
[2] MPET_KE
	- Enables motor BEMF constant measurement during motor parameter measurement routine
	0h = Disables Motor BEMF constant measurement during motor parameter measurement routine
	1h = Enable Motor BEMF constant measurement during motor parameter measurement routine
	
[1] MPET_MECH
	- Enables motor mechanical parameter measurement during motor parameter measurement routine
	0h = Disable Motor mechanical parameter measurement during motor parameter measurement routine
	1h = Enable Motor mechanical parameter measurement during motor parameter measurement routine

[0] MPET_WRITE_SHADOW
	- Write measured parameters to shadow register when set to 1b

```