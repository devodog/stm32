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
#include "mcf8316a.h"
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

void I2C(char* paramStr, int* paramValues) {
   uint8_t i2cDataWord[7] = {0}; // I2C Data Word without CRC
   i2cDataWord[0] = (DATA_LENGTH_32 << DATA_LEN_POS);
   i2cDataWord[0] |= (WRITE_OPERATION << RW_OPERATION_BIT_POS);
   i2cDataWord[1] = (eepromAddr[ISD_CONFIG] >> 8)&(0xff);
   i2cDataWord[2] = (eepromAddr[ISD_CONFIG])&(0xff);
   memcpy(&i2cDataWord[3], &eepromRegValues[ISD_CONFIG], 4);

   if ((strncmp(paramStr, "TEST", 4) == 0) || (strncmp(paramStr, "test", 4)) == 0) {


      if (HAL_I2C_Master_Transmit(&hi2c1, 0x2, (uint8_t*)&i2cDataWord[0], 7, 1000) != HAL_OK) {
         printf("\r\nHAL_I2C FUNCTION FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
      }
      else {
         printf("\r\nHAL_I2C_Mem_Write() OK!");
      }


      /****
      if (HAL_I2C_IsDeviceReady(&hi2c1, 0x2, 1, 10) != HAL_OK) {
         printf("\r\nHAL_I2C_IsDeviceReady failed with error code 0x%02x\r\n", hi2c1.ErrorCode);
      }
      ****/

      printf("\r\nScanning all addresses...\r\n");
      for (uint16_t i = 2; i < 128; i++) {
         if (HAL_I2C_Master_Transmit(&hi2c1, i<<1, (uint8_t*)&i2cDataWord[0], 7, 1000) == HAL_OK) {
            printf("\r\nI2C device addr. 0x%02x responded!", i<<1);
            return;
         }
         printf(".");
      }
      printf("\r\nNO I2C Device responded!");
   }
   else {
      printf("\r\nUNKNOWN I2C COMMAND");
   }
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

/*
 * EEPROM READ ONLY - ALL ADDRESSES
 */
/*
typedef struct cw {
   uint8_t OP_RW : 1;
   uint8_t CRC_EN : 1;
   uint8_t DLEN : 2;
};
*/
void EEPROM(char* paramStr, int* paramValues) {
   // NOT COMPLETED...
   uint8_t paramStartPos;
   //uint16_t regAddr = 0;
   //uint8_t regDataByte;
   //uint8_t rxData[7] = {0};
   uint8_t i2cDataWord[7] = {0}; // I2C Data Word without CRC
   i2cDataWord[0] = (DATA_LENGTH_32 << DATA_LEN_POS);

   if (strncmp(paramStr, "WRITE", 5) == 0) {
      i2cDataWord[0] |= (WRITE_OPERATION << RW_OPERATION_BIT_POS);
      paramStartPos = 6;
      // For initial test we'll attempt to write predefined data into one
      // of the shadow registers.
      // Make up the DataWord
      i2cDataWord[1] = (eepromAddr[ISD_CONFIG] >> 8)&(0xff);
      i2cDataWord[2] = (eepromAddr[ISD_CONFIG])&(0xff);
      //memcpy(&i2cDataWord[1], &eepromAddr[ISD_CONFIG], 2);
      memcpy(&i2cDataWord[3], &eepromRegValues[ISD_CONFIG], 4);
/***
      // TEST!
      printf("\r\nI2C Data Word: ");
      for (int i = 0; i < 7; i++) {
         printf("0x%02x ", i2cDataWord[i]);
      }
***/
      // Load Control Word and data into the device's registers
      //if (HAL_I2C_Mem_Write(&hi2c1, 0x02, 0x00, 7, (uint8_t*)&i2cDataWord[0], 7, 1) != HAL_OK) {
      if (HAL_I2C_Master_Transmit(&hi2c1, 0x2, (uint8_t*)&i2cDataWord[0], 7, 1000) != HAL_OK) {
         printf("\r\nHAL_I2C FUNCTION FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
      }
      else {
         printf("\r\nHAL_I2C_Master_Transmit() OK!\r\n");
         HAL_Delay(100);
         i2cDataWord[0] |= READ_OPERATION;
         //memcpy(&i2cDataWord[3], 0, 4); // Clearing the data part of the Data Word...
         if (HAL_I2C_Master_Receive(&hi2c1, 0x2, (uint8_t*)&i2cDataWord[0], 7, 1000) != HAL_OK) {
            printf("\r\nHAL_I2C_Master_Receive() FAILED!");
         }
         else {
            printf("\r\nData read:0x%02x%02x%02x%02x at Addr.: 0x%02x%02x", i2cDataWord[3],i2cDataWord[4], i2cDataWord[5], i2cDataWord[6], i2cDataWord[1], i2cDataWord[2]);
         }
         /***
            if (HAL_I2C_Master_Receive(&hi2c3, MCF8316A_ADDRESS, (uint8_t*)&rxData[0], 4, 1000) != HAL_OK) {
               printf("\r\nHAL_I2C_Master_Receive() FAILED!");
            }
            else {
               printf("\r\nData read:0x%02x%02x%02x%02x at Addr.: 0x%02x%02x", rxData[0],rxData[1], rxData[2], rxData[3], i2cDataWord[1], i2cDataWord[2]);
            }
            ***/
            /***
            if (HAL_I2C_Mem_Read(&hi2c3, MCF8316A_ADDRESS, memAddr, I2C_MEMADD_SIZE_16BIT, &dataRead[0], 2, 1000) != HAL_OK) {
               printf("\r\nHAL_I2C_Mem_Read() FAILED!");
            }
            else {
               printf("\r\nData read:0x%02x%02x at Addr.: 0x%x", dataRead[0],dataRead[1], memAddr);
            }
            ***/
         }
      void promt();
      return;


      }


   else if (strncmp(paramStr, "READ", 4) == 0) {
      i2cDataWord[0] |= (READ_OPERATION << RW_OPERATION_BIT_POS);
      paramStartPos = 5;


   }
   else {
      printf("\r\nUNKNOWN OR INCORRECT COMMAND FORMAT");
      return;
   }
   // If we want to real all device registers...
   if ((strncmp(&paramStr[paramStartPos], "ALL", 3) == 0) && (paramStartPos == 5)) {
      /************************************************************************
      In MCF8316A, EEPROM read procedure is as follows,
      1. Write 0x40000000 into register 0x0000EA to read the EEPROM data into
      the shadow registers (0x000080-0x0000AE).
      2. Wait for 100ms for the EEPROM read operation to complete.
      3. Read the shadow register values,1 or 2 registers at a time, using the
      I2C read command as explained in Section 7.6.2. Shadow register addresses
      are in the range of 0x000080-0x0000AE. Register address increases in
      steps of 2 for 32-bit read operation
      (since each address is a 16-bit location).
      ************************************************************************/




      //HAL_I2C_Mem_Write(&hi2c3, MCF8316A_ADDRESS, (uint8_t*)&i2cDataWord, 7, 1000);


      /*
      for (int i = 0; i < 24; i++) {

         // I2C Read data from address = eepromAddr[i]
         printf("DATA? in Reg. %s @addr. 0x%x\r\n", regNames[i], (unsigned int)eepromAddr[i]);
      }
      */
      void promt();
      return;
   }


   // Get the register address and the data to be written to this register.
   // Will treat the command line input for this EEPROM WRITE command strict.
   // Only a single format are accepted.
   // Nucleo_M>EEPROM WRITE 0x080 0x12345678
   // Register address is 12 bit while the data is 32 bit.

   // The first check that two hexadecimal values have been entered.
   if ((strncmp(&paramStr[paramStartPos], "0x", 2) != 0) || (strncmp(&paramStr[paramStartPos+6], "0x", 2) != 0) || (strlen(paramStr) != 22)) {
      printf("\r\nUNKNOWN OR INCORRECT COMMAND FORMAT");
      return;
   }

      // Make a store-command to enter the value, written into the shadow
      // register, into the EEPROM.
      //dataWord.controlWord[2] = 0xEA;
      //dataWord.regData = 0x8A500000;
      //HAL_I2C_Master_Transmit(&hi2c3, MCF8316A_ADDRESS, (uint8_t*)&dataWord, 7, 1000);
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

/**
// The cmd-line Command structure
struct command {
  char name[COMMAND_NAME_LENGTH];  // the command name
  int params;     // room for params numbers of command-parameters
  int maxParamLength; // max param word length.
  char paramWords[COMMAND_PARAMS][COMMAND_PARAM_LENGTH];
  int paramValues[COMMAND_PARAMS];  // to preserve the operation state...
  void (*cmdFunction)(char*, int*); // the command support function...
};
*/


// Command array initialization
struct command mcuCmds [] = {
  {"LED", 2, 6, {"ON", "OFF", "BLINK", "HELP"}, {0, 500}, &LED},
  {"I2C", 1, 5, {"test", "TEST"}, {1}, &I2C},
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
