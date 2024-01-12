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

///////////////////////////////////////////////////////////////////

int lastError = 0;
int ms = 0;
char *cmdList[] = {"ld2", "dummy"};
uint8_t numberOfCommands = 2;
extern uint8_t led2;
int msValue = 0;
enum commands {
   LD2,
   DUMMY
} COMMANDS;

char *argList[] = {"on", "off", "blink"};
//


void promt() {
   printf("\r\nNUCLEO> ");
   fflush(stdout);
}

uint8_t executeCmd(char *termInput, int cmdLength) {
   int i = 0;

   // Check if the entered command is part of the command-list for this application.
   for (; i < numberOfCommands; i++) {

	  // TEST OF THE NEW COMMAND IMPLEMENTATION...
	  ///////////////////////////////////////////////////////////////////
 	  if (strncmp(mcuCmds[i].name, termInput, strlen(mcuCmds[i].name)) == 0) {
 		 // The command entered is implemented partly or in full...
 		 // Has the command 1 or more parameters?
 		 // Should the command-structure have an extra element that holds the pointer
 		 // to a subroutine that handles the complete user command?
 		 // ...what about the following?
 		 mcuCmds[i].cmdFunction((char*)&termInput[strlen(mcuCmds[i].name)+1]);

 		 printf("\r\n");
 		 for(int p=0; p<mcuCmds[i].params; p++){
     		 printf("%s = %d\r\n", mcuCmds[i].paramWords[p], mcuCmds[i].paramValues[p]);
     	 }
 		 promt();
     	 return 0;
      }
 	  ///////////////////////////////////////////////////////////////////
      if (strncmp(cmdList[i], termInput, strlen(cmdList[i])) == 0) {
    	 // The command entered is found in the command-list
         break;
      }
   }

   // Execute the command if part of the command-list.
   if (i >= sizeof(cmdList)) {
      printf("\r\n%s is not recognized\r\n", termInput);
      promt();
      return -1;
   }
   else {
      switch (i) {
         case LD2: { // Parsing parameters
		   char param1[5] = {0};
		   char param2[5] = {0};
		   int j = 0;
		   // The first parameter
		   for (; j<5; j++){
			  if (termInput[4+j] != ' ') {
				  param1[j] = termInput[4+j];
			  }
			  else
			  break;
		   }

		   if ((j > 0)&&(param1[1]=='l')) {
			   int k = 0;
			   // read the next argument/parameter
			   for (; k<5; k++){
				  if (termInput[5+j+k] != ' ') {
					 param2[k] = termInput[5+j+k];
				  }
				  else
					 break;
			   }
			   msValue = atoi(&param2[0]);
			   printf("\r\nBlinking LD2 every %d ms\r\n", msValue);
			   led2 = BLINKING;
		   }
		   else if ((j > 0)&&(param1[1]=='n')){
			   printf("\r\nSetting LD2 ON\r\n");
			   led2 = ON;
		   }
		   else{
			   printf("\r\nSetting LD2 OFF\r\n");
			   led2 = OFF;
		   }
		}
		break;

	 case DUMMY:
		printf("\r\ndummy command....\r\n");
	 default:
		printf("\nNOP[i=%d]", i);
      }
   }
   promt();
   return 0;
}
