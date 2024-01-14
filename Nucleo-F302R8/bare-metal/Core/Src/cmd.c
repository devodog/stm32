/*
 * cmd.c
 *
 *  Created on: Jan 3, 2024
 *      Author: dagak
 *
 *  Minimalistic code for command-line interface.
 *
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "cmd.h"

#define COMMAND_PARAMS 10
#define COMMAND_PARAM_LENGTH 10
#define COMMAND_NAME_LENGTH 20
#define NUMBERS_OF_MCU_COMMANDS 2

extern ADC_HandleTypeDef hadc1;
extern uint8_t led2;

int msValue = 0; // milliseconds value
int lastError = 0;

// The cmd-line Command structure
struct command {
  char name[COMMAND_NAME_LENGTH];  // the command name
  int params;     // room for params numbers of command-parameters
  int maxParamLength; // max param word length.
  char paramWords[COMMAND_PARAMS][COMMAND_PARAM_LENGTH];
  int paramValues[COMMAND_PARAMS];
  void (*cmdFunction)(char*, int*); // the command support function...
};
///////////////////////////////////////////////////
// Define cmd-line Command support functions below.
void LED(char* paramStr, int* paramValues) {
	if (strncmp(paramStr, "ON", 2) == 0) {
		printf("\r\nLED ON");
		paramValues[0] = 1;
		paramValues[1] = 0;
		paramValues[2] = 0;
      led2 = ON;
	}
	else if (strncmp(paramStr, "OFF", 3) == 0) {
		printf("\r\nLED OFF");
		paramValues[0] = 0;
		paramValues[1] = 1;
		paramValues[2] = 0;
      led2 = OFF;
	}
	else if (strncmp(paramStr, "BLINK", 5) == 0) {
		if (strncmp(&paramStr[6], "0", 3) != 0) {
		   paramValues[2] = atoi(&paramStr[6]);
		   msValue = atoi(&paramStr[6]);
         paramValues[0] = 0;
         paramValues[1] = 0;
         led2 = BLINKING;
		}
		else {
			paramValues[2] = 0;
		}
		printf("\r\nLED BLINK %d", msValue);
   }
	else {
		printf("\r\nUNKNOWN LED COMMAND");
	}
}

void ADC(char* paramStr, int* paramValues){
   if (strncmp(paramStr, "RO", 2) == 0) {
      printf("\r\nADC READ ONCE");
      // Start ADC Conversion
      HAL_ADC_Start(&hadc1);
      // Poll ADC1 Peripheral & TimeOut = 1mSec
      HAL_ADC_PollForConversion(&hadc1, 1);
      // Read The ADC Conversion Result - using 3300 + 400 offset to
      // calculate the analog value
      printf("\r\nAA Battery voltage: %ld mV", 3700*HAL_ADC_GetValue(&hadc1)/4096);
   }
   else if (strncmp(paramStr, "HELP", 2) == 0){
      printf("\r\nSome help text her...");
   }
   else {
      printf("\r\nUNKNOWN ADC COMMAND");
   }
}

void dummy(char* paramStr, int* paramValues){
	printf("DUMMY\r\n");
}

// Command array initialization
struct command mcuCmds [] = {
  {"LED", 3, 6, {"ON", "OFF", "BLINK"}, {0, 1, 500 }, &LED},
  {"ADC", 4, 6, {"RO", "AVRAGE", "POLL", "HELP"}, {0, 10, 500, 0}, &ADC},
  {"DUMMY", 2, 6, {"TRUE", "FALSE"}, {0, 0}, &dummy}
};

void promt() {
   printf("\r\nNUCLEO> ");
   fflush(stdout);
}

uint8_t executeCmd(char *termInput, int cmdLength) {
   int i = 0;
   size_t numberOfCommands = sizeof(mcuCmds) / sizeof(mcuCmds[0]);

   // Check if the entered command is part of the command-list for this application.
   for (; i < numberOfCommands; i++) {
 	  if (strncmp(mcuCmds[i].name, termInput, strlen(mcuCmds[i].name)) == 0) {
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
