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
