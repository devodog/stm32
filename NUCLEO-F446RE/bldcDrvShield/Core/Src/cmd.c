/*
 * cmd.c
 *
 *  Created on: Apr 27, 2026
 *      Author: dagak
 *
 * This file is to handle user input from the a serial line (UART) communication port.
 * Predefined and valid user input will be interpreted to act, in this project, on motor parameters.
 *
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#include "cmd.h"
#include "appver.h"


#define COMMAND_PARAMS 10
#define COMMAND_PARAM_LENGTH 10
#define COMMAND_NAME_LENGTH 20
#define NUMBERS_OF_MCU_COMMANDS 2

extern uint8_t led2;
extern ADC_HandleTypeDef hadc1;

int timRepeat = 1;
int timRepeatCount = 0;

int msValue = 0; // milliseconds value
int lastError = 0;
int sensorReadings = 0;

// The cmd-line Command structure
struct command {
  char name[COMMAND_NAME_LENGTH];  // the command name
  int params;     // room for params numbers of command-parameters
  int maxParamLength; // max param word length.
  char paramWords[COMMAND_PARAMS][COMMAND_PARAM_LENGTH];
  int paramValues[COMMAND_PARAMS];
  void (*cmdFunction)(char*, int*); // the command support function...
};
enum LED_STATE {
	OFF,
	ON,
	BLINK
};

// Define cmd-line Command support functions below.
void led(char* paramStr, int* paramValues) {
    if (strncmp(paramStr, "on", 2) == 0) {
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
       //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
       printf("\r\nled on");
       paramValues[0] = ON;
    }
    else if (strncmp(paramStr, "off", 3) == 0) {
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
       //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
       printf("\r\nled off");
       paramValues[0] = OFF;
    }
    else if (strncmp(paramStr, "blink", 5) == 0) {
      // Turn on LED blinking with interval in milliseconds.
      // On and Off time is equal gives the blinking frequency to be 1/(2*blinkInterval)
      // The LED toggle operation is located in the main-loop.
      //HAL_GPIO_TogglePin(GPIO_PIN_8);
        if (strncmp(&paramStr[6], "0", 3) != 0) {
           paramValues[2] = atoi(&paramStr[6]);
           msValue = atoi(&paramStr[6]); // msValue is a global parameter...
           paramValues[0] = ON;
           paramValues[1] = msValue;
        }
        else {
            paramValues[1] = 0;
        }
        printf("\r\nled blink on %d", msValue);
   }
    else {
        printf("\r\nUNKNOWN LED COMMAND");
    }
}

void adc(char* paramStr, int* paramValues) {
   uint16_t adcValue;
   adcValue = HAL_ADC_GetValue(&hadc1);
   printf("\r\n\r\nReading ADC value to be: %d", adcValue);
}

void speed(char* paramStr, int* paramValues) {
   /*
   // List the options for this command...
   int numOfOptions = mcuCmds[1].params;


   for (int i = 0; i < numOfOptions; i++) {
      printf("\r\n%s", mcuCmds[1].paramWords[i]);
   }
   */
   printf("\r\nSpeed control not implemented.");

}

void sys(char* paramStr, int* paramValues){
   if (strncmp(paramStr, "BN", 2) == 0) {
      printf("\r\nBuild no.:%d", BUILD);
   }
   else if (strncmp(paramStr, "BD", 2) == 0) {
      printf("\r\nBuild date: %s", BUILD_DATE_AND_TIME);
   }
   else if (strncmp(paramStr, "VER", 2) == 0) {
      printf("\r\nVersion:%d.%d", MAJOR_VERSION, MINOR_VERSION);
   }

}

// Commands defined. (Command array initialization)
// All commands should primerarly be expressed in lower case.
struct command mcuCmds [] = {
//	cmd, number_of_parameters, max_parameter_len,
  {"led", 3, 6, {"on", "off", "blink", "help"}, {0, 500}, &led},
  {"adc", 3, 6, {"on", "off", "blink", "help"}, {0, 500}, &adc},
  {"speed", 4, 7, {"forward", "revers", "stop", "help"}, {0, 500, 10, 0}, &speed},
  {"sys", 3, 4, {"bn", "bd", "ver"}, {0, 0, 0}, &sys}
};

void promt() {
   printf("\r\nNUCLEO> ");
   fflush(stdout);
}

uint8_t executeCmd(char *termInput, int cmdLength) {
   int i = 0;
   // How many commands are predefined for this system...
   size_t numberOfCommands = sizeof(mcuCmds) / sizeof(mcuCmds[0]);

   // Check if the entered command is part of the command-list for this application.
   for (; i < numberOfCommands; i++) {
 	  if (strncmp(mcuCmds[i].name, termInput, strlen(mcuCmds[i].name)) == 0) {
 	     // The entered command is found it the command-list and the command is executed.
 		 mcuCmds[i].cmdFunction((char*)&termInput[strlen(mcuCmds[i].name)+1], (int*) &mcuCmds[i].paramValues);


 		 /*** for test only...
 		 printf("\r\nparamValues[0]: %d, paramValues[1]: %d, paramValues[2]: %d",
               mcuCmds[i].paramValues[0],mcuCmds[i].paramValues[1],mcuCmds[i].paramValues[2]);
         ***/


 		 promt();
     	 return 0;
      }
   }

   // Execute the command if part of the command-list.
   if (i >= numberOfCommands) {
      printf("\r\nThe command: %s[%d], is not recognized", termInput, numberOfCommands);
      promt();
      return -1;
   }
   else {
	  return -2;
   }
}



