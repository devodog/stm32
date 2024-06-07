/*
 * cmd.c
 *
 *  Created on: July 3, 2024
 *      Author: dagak
 *
 *  Minimalistic code for command-line interface for BLDC driver configuration..
 *
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "main.h"
#include "cmd.h"
//#include "mcf8316a.h"
#include "appver.h"
// 

#define LED_OFF 0
#define LED_ON 1
#define LED_BLINK 2

#define MCF8316A_ADDRESS 0x1 << 1
//  device I2C register address
#define GET_DATA_READY_STATUS 0x0202
#define READ_MEASURMENT 0x0300

#define COMMAND_PARAMS 10
#define COMMAND_PARAM_LENGTH 10
#define COMMAND_NAME_LENGTH 20
#define NUMBERS_OF_MCU_COMMANDS 2
///////////////////////////////////////////////////////////////////////////////
#define MCF8316A_ADDRESS 0x1 << 1

#define DATA_LENGTH_16 0x0
#define DATA_LENGTH_32 0x1
#define DATA_LENGTH_64 0x2
#define DATA_LEN_POS 4
#define CRC_ENABLE 0x1
#define CRC_DISABLE 0x1

#define RW_OPERATION_BIT_POS 7
#define READ_OPERATION 1
#define WRITE_OPERATION 0
#define MEM_SEC_BIT_POS 0
#define MEM_PAGE_BIT_POS 6

#define READ_EEPROM_CMD 0x40000000  // to read the EEPROM data into the shadow registers (0x000080-0x0000AE).
#define WRITE_EEPROM_CMD 0x8A500000 // to write the shadow register(0x000080-0x0000AE) values into the EEPROM.
// DEVICE_CONTROL Registers
#define DEV_CTRL 0xEA

extern I2C_HandleTypeDef hi2c1;
extern uint8_t led2;
int msValue = 0; // milliseconds value
int lastError = 0;

// The cmd-line Command structure
struct command {
  char name[COMMAND_NAME_LENGTH];  // the command name
  int params;     // room for params numbers of command-parameters
  int maxParamLength; // max param word length.
  char paramWords[COMMAND_PARAMS][COMMAND_PARAM_LENGTH];
  int paramValues[COMMAND_PARAMS];  // to preserve the operation state...
  void (*cmdFunction)(char*, int*); // the command support function...
};

///////////////////////////////////////////////////
// Define cmd-line Command support functions below.

void LED(char* paramStr, int* paramValues) {
	//uint8_t led2 = paramValues[0];

   if (strncmp(paramStr, "ON", 2) == 0) {
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		paramValues[0] = 1;
      //led2 = ON;
      printf("\r\nLED ON");
	}
	else if (strncmp(paramStr, "OFF", 3) == 0) {
	   HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		paramValues[0] = 0;
      //led2 = OFF;
      printf("\r\nLED OFF");
	}
	else if (strncmp(paramStr, "BLINK", 5) == 0) {
      paramValues[0] = 2;

      if (strncmp(&paramStr[6], "0", 3) != 0) {
         // Blink interval to be used...
		   paramValues[1] = atoi(&paramStr[6]);
		   msValue = atoi(&paramStr[6]);
         //led2 = BLINKING;
		}
		printf("\r\nLED BLINK %d", msValue);
   }
	else {
		printf("\r\nUNKNOWN LED COMMAND");
	}
   //return led2;
}

void DRVOFF(char* paramStr, int* paramValues) {
   char cmdParameter;
   cmdParameter = toupper((unsigned char) *paramStr);

   if (cmdParameter == 'Y') {
      HAL_GPIO_WritePin(GPIOA, DRVOFF_Pin, GPIO_PIN_SET);
      paramValues[0] = 1;
   }
   else {
      HAL_GPIO_WritePin(GPIOA, DRVOFF_Pin, GPIO_PIN_RESET);
      paramValues[0] = 0;
   }
}

void BRAKE(char* paramStr, int* paramValues) {
   if (strncmp(paramStr, "ON", 2) == 0) {
         printf("\r\nBRAKE ON!");
         paramValues[0] = 1;
         HAL_GPIO_WritePin(GPIOA, BRAKE_Pin, GPIO_PIN_SET);
      }
      else if (strncmp(paramStr, "OFF", 3) == 0) {
         printf("\r\nBRAKE OFF!");
         paramValues[0] = 0;
         HAL_GPIO_WritePin(GPIOA, BRAKE_Pin, GPIO_PIN_RESET);
      }
      else {
         printf("\r\nUNKNOWN BRAKE COMMAND");
      }
}

void DIR(char* paramStr, int* paramValues) {
   if (strncmp(paramStr, "ABC", 3) == 0) {
            printf("\r\nDIR High -> Phase sequence is A->B->C");
            paramValues[0] = 1;
            HAL_GPIO_WritePin(GPIOA, DIR_Pin, GPIO_PIN_SET);
   }
   else if (strncmp(paramStr, "ACB", 3) == 0) {
      printf("\r\nDIR Low -> Phase sequence is A->C->B");
      paramValues[0] = 1;
      HAL_GPIO_WritePin(GPIOA, DIR_Pin, GPIO_PIN_RESET);
   }
   else {
      printf("\r\nUNKNOWN DIR COMMAND");
   }
}

void CONFIG(char* paramStr, int* paramValues) {
   uint8_t paramStartPos;
   uint16_t regAddr = 0;
   uint8_t regDataByte;
   uint8_t i2cDataWord[7] = {0};

   i2cDataWord[0] = (DATA_LENGTH_32 << DATA_LEN_POS);

   if (strncmp(paramStr, "WRITE", 5) == 0) {
      i2cDataWord[0] |= (WRITE_OPERATION << RW_OPERATION_BIT_POS);
      paramStartPos = 6;
   }
   else if (strncmp(paramStr, "READ", 4) == 0) {
      i2cDataWord[0] |= (READ_OPERATION << RW_OPERATION_BIT_POS);
      paramStartPos = 5;
   }
   else {
      printf("\r\nUNKNOWN OR INCORRECT COMMAND FORMAT");
      return;
   }
   // Get the register address and the data to be written to this register.
   // Will treat the command line input for this EEPROM WRITE command strict.
   // Only a single format are accepted.
   // Nucleo_M>WRITE 0x080 0x12345678
   // Register address is 12 bit while the data is 32 bit.

   // The first check that two hexadecimal values have been entered.
   if ((strncmp(&paramStr[paramStartPos], "0x", 2) != 0) || (strncmp(&paramStr[paramStartPos+6], "0x", 2) != 0) || (strlen(paramStr) != 22)) {
      printf("\r\nUNKNOWN OR INCORRECT COMMAND FORMAT");
      return;
   }

   // The second parameter in the EEPROM WRITE command is the register
   // address, 12 bit, which means that there shall be 3 ascii characters
   // representing the hexadecimal value to be converted to a binary value
   // starting at string position 8 (for WRITE command).
   uint8_t parameterStart = paramStartPos + 2;
   uint8_t parameterEnd = parameterStart + 3;
   for(uint8_t i = parameterStart; i < parameterEnd; i++) {
      if (paramStr[i] < 'A') {
         regAddr |= (paramStr[i] - 48);
      }
      else if ((paramStr[i] >= 'A') && (paramStr[i] < 'G')) {
         regAddr |= (paramStr[i] - 55); // 65 - 55 = 10
      }
      else if ((paramStr[i] >= 'a') && (paramStr[i] < 'g')) {
         regAddr |= (paramStr[i] - 87);
      }
      else {
         regAddr |= 0;
      }
      if (i < (parameterEnd-1)) {
         regAddr = regAddr << 4;
      }
      else
         break;
   }
   i2cDataWord[1] |= ((regAddr >> 8) & 0x0f);
   i2cDataWord[2] = regAddr & 0xff;
   uint8_t dwIndex = 7; // the last index of the data word byte buffer
   uint8_t mst = 1;
   // The third parameter to the EEPROM WRITE command is the data to be
   // written into the actual register, which must be a 32 bit hexadecimal
   // value. This parameter starts at the command string position 14.
   parameterStart = paramStartPos + 8;
   parameterEnd = parameterStart + 8;
   for(uint8_t i = parameterStart; i < parameterEnd; i++) {

      if (paramStr[i] < 'A') {
         regDataByte |= (paramStr[i] - 48);
      }
      else if ((paramStr[i] >= 'A') && (paramStr[i] < 'G')) {
         regDataByte |= paramStr[i] - 55;
      }
      else if ((paramStr[i] >= 'a') && (paramStr[i] < 'g')) {
         regDataByte |= paramStr[i] - 87;
      }
      else {
         regDataByte |= 0;
      }
      if (mst == 1) {
         regDataByte = regDataByte << 4;
         mst = 0;
      }
      else {
         // Make sure that the least significant byte is sent first on the i2c bus.
         // The first address values read are considered to be the most significant values...
         i2cDataWord[--dwIndex] = regDataByte;
         regDataByte = 0;
         mst = 1;
      }
   }

   // TEST!
   printf("\r\ni2c control word: ");
   for (int i = 0; i < 7; i++) {
      printf("0x%02x ", i2cDataWord[i]);
   }
   // Make sure that the motor is not spinning.
   // Pushing the data onto the "shadow register"
   //HAL_I2C_Master_Transmit(&hi2c3, MCF8316A_ADDRESS, (uint8_t*)&dataWord, 7, 1000);
}

void EEPROM(char* paramStr, int* paramValues) {
   // NOT COMPLETED...
   if (strncmp(paramStr, "WRITE", 5) == 0) {
      // Make a store-command to enter the value, written into the shadow
      // register, into the EEPROM.
      //dataWord.controlWord[2] = 0xEA;
      //dataWord.regData = 0x8A500000;
      //HAL_I2C_Master_Transmit(&hi2c3, MCF8316A_ADDRESS, (uint8_t*)&dataWord, 7, 1000);
   }
}

void SYS(char* paramStr, int* paramValues){
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


// Command array initialization

struct command mcuCmds [] = {
  {"LED", 2, 6, {"ON", "OFF", "BLINK", "HELP"}, {0, 500}, &LED},
  {"DRVOFF", 1, 4, {"YES", "NO"}, {1}, &DRVOFF},
  {"DIR", 1, 4, {"ABC", "ACB"}, {1}, &DIR},
  {"BRAKE", 1, 4, {"ON", "OFF"}, {1}, &BRAKE},
  {"CONFIG", 3, 6, {"READ", "WRITE"}, {0, 0}, &CONFIG},
  {"EEPROM", 3, 6, {"READ", "WRITE"}, {0, 0}, &EEPROM},
  {"SYS", 3, 4, {"BN", "BD", "VER"}, {0, 0, 0}, &SYS}
};

void promt() {
   printf("\r\nNucleo_M> ");
   fflush(stdout);
}

uint8_t executeCmd(char *termInput, int cmdLength) {
   int i = 0;
   size_t numberOfCommands = sizeof(mcuCmds) / sizeof(mcuCmds[0]);

   // Check if the entered command is part of the command-list for this application.
   for (; i < numberOfCommands; i++) {
 	  if (strncmp(mcuCmds[i].name, termInput, strlen(mcuCmds[i].name)) == 0) {
 	     if (strlen(mcuCmds[i].name) == strlen(termInput)) {
 	        // Single word command!

 	     }
 	     else {
 	       mcuCmds[i].cmdFunction((char*)&termInput[strlen(mcuCmds[i].name)+1], (int*) &mcuCmds[i].paramValues);
 	     }


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
