/*
 * cmd.c
 *
 *  Created on: Jan 3, 2024
 *      Author: dagak
 *
 *  Minimalistic and only Proof of Concept (PoC) code for command-line interface.
 *
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "cmd.h"

// New way of handling command-line user interface.
///////////////////////////////////////////////////////////////////
#define COMMAND_PARAMS 10
#define COMMAND_PARAM_LENGTH 10
#define COMMAND_NAME_LENGTH 20
#define NUMBERS_OF_MCU_COMMANDS 2

extern uint8_t led2;
int msValue = 0; // milliseconds value
int lastError = 0;

struct command {
  char name[COMMAND_NAME_LENGTH];  // the command name
  int params;     // room for params numbers of command-parameters
  int maxParamLength; // max param word length.
  char paramWords[COMMAND_PARAMS][COMMAND_PARAM_LENGTH];
  int paramValues[COMMAND_PARAMS];
  void (*cmdFunction)(char*, int*); // the command support function...
};


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

void dummy(char* paramStr, int* paramValues){
	printf("DUMMY\r\n");
}

struct command mcuCmds [NUMBERS_OF_MCU_COMMANDS] = {
  {"LED", 3, 6, {"ON", "OFF", "BLINK"}, {0, 1, 500 }, &LED},
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
 		 promt();
     	 return 0;
      }
   }

   // Execute the command if part of the command-list.
   if (i >= numberOfCommands) {
      printf("\r\nThe command: %s, is not recognized", termInput);
      promt();
      return -1;
   }
   else {
	  return -2;
   }
}
