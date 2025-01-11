# Configurator for MCF8316A UiA custom development board 
Using STMicroelectrionic Nucleo-F302R8 development board for providing serial 
command line interface for accessing the MCF8316A chip configuration and 
driver registers through I2C interface. 

## Commands available
Nucleo_M> led <on|off|state>; Setting the Nucleo dev kit LED on or of and retrieving its state as well.

Nucleo_M> drv <on|off|state>
 
Nucleo_M> brk <on|off|state>
 
Nucleo_M> dir <on|off|state>
 
Nucleo_M> i2c <get|set<i2c-addr[dec]>>
 
Nucleo_M> sys <>

Nucleo_M> ALGO_CTRL1 <SET|GET>

Nucleo_M> ALGO_CTRL2 <SET|GET>

Nucleo_M> FAULT <>

Nucleo_M> EEPROM <READ <ALL|reg-addr[hex]> | WRITE <DEFAULT1|DEFAULT2|?reg-addr[hex]>>

Nucleo_M> RAM <READ <ALL|reg-addr[hex]> | WRITE ??>

// Command array initialization
struct command mcuCmds [] = {
  {"ALGO_CTRL1", 1, 4, {"SET", "GET"}, {0, 0}, &ALGO_CTRL1},
  {"ALGO_CTRL2", 1, 4, {"SET", "GET"}, {0, 0}, &ALGO_CTRL2},
  {"FAULT", 2, 5, {"GDFS", "CFS"}, {0, 0}, &FAULT},
  {"EEPROM", 3, 6, {"READ", "WRITE"}, {0, 0}, &EEPROM},
  {"RAM", 2, 6, {"READ", "WRITE"}, {0, 0}, &RAM},
};