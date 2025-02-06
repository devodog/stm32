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

// Default device i2c address - can be changed in the DEVICE_CONFIG1 register
#define MCF8316A_ADDRESS 0x1 << 1

//  device I2C register address
#define GET_DATA_READY_STATUS 0x0202
#define READ_MEASURMENT 0x0300

#define COMMAND_PARAMS 10
#define COMMAND_PARAM_LENGTH 10
#define COMMAND_NAME_LENGTH 20
///////////////////////////////////////////////////////////////////////////////
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
uint16_t i2cAddress = MCF8316A_ADDRESS;

// The cmd-line Command structure
struct command {
  char name[COMMAND_NAME_LENGTH];  // the command name
  int params;     // room for params numbers of command-parameters
  int maxParamLength; // max param word length.
  char paramWords[COMMAND_PARAMS][COMMAND_PARAM_LENGTH];
  int paramValues[COMMAND_PARAMS];  // to preserve the operation state...
  void (*cmdFunction)(char*, struct command*); // the command support function...
};

enum binaryState {
   OFF,
   ON,
   STATE
};

///////////////////////////////////////////////////
// Define cmd-line Command support functions below.
void led(char* paramStr, struct command* pCmd) {
   if (strncmp(paramStr, pCmd->paramWords[ON], strlen(pCmd->paramWords[ON])) == 0) {
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		pCmd->paramValues[0] = ON;
      printf("\r\nled on");
	}
	else if (strncmp(paramStr, pCmd->paramWords[OFF], strlen(pCmd->paramWords[OFF])) == 0) {
	   HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	   pCmd->paramValues[0] = OFF;
      printf("\r\nled off");
	}
   else if (strncmp(paramStr, pCmd->paramWords[STATE], strlen(pCmd->paramWords[STATE])) == 0) {
      printf("\r\nled state = %s", pCmd->paramWords[pCmd->paramValues[0]]);
   }
	else {
		printf("\r\nUnknown led command");
	}
   //return led2;
}

void drv(char* paramStr, struct command* pCmd) {
   if (strncmp(paramStr, pCmd->paramWords[OFF], strlen(pCmd->paramWords[OFF])) == 0) {
      HAL_GPIO_WritePin(GPIOA, DRVOFF_Pin, GPIO_PIN_SET);
      pCmd->paramValues[0] = OFF;
   }
   else if (strncmp(paramStr, pCmd->paramWords[ON], strlen(pCmd->paramWords[ON])) == 0) {
      HAL_GPIO_WritePin(GPIOA, DRVOFF_Pin, GPIO_PIN_RESET);
      pCmd->paramValues[0] = ON;
   }
   else if (strncmp(paramStr, pCmd->paramWords[STATE], strlen(pCmd->paramWords[STATE])) == 0) {
         printf("\r\ndrv state = %s", pCmd->paramWords[pCmd->paramValues[0]]);
   }
   else {
      printf("\r\nUnknown drv command");
   }
}

void brk(char* paramStr, struct command* pCmd) {
   if (strncmp(paramStr, pCmd->paramWords[ON], strlen(pCmd->paramWords[ON])) == 0) {
         printf("\r\nBrake ON!");
         pCmd->paramValues[0] = ON;
         HAL_GPIO_WritePin(GPIOA, BRAKE_Pin, GPIO_PIN_SET);
      }
      else if (strncmp(paramStr, pCmd->paramWords[OFF], strlen(pCmd->paramWords[OFF])) == 0) {
         printf("\r\nBrake OFF!");
         pCmd->paramValues[0] = OFF;
         HAL_GPIO_WritePin(GPIOA, BRAKE_Pin, GPIO_PIN_RESET);
      }
      else if (strncmp(paramStr, pCmd->paramWords[STATE], strlen(pCmd->paramWords[STATE])) == 0) {
            printf("\r\nbrk state = %s", pCmd->paramWords[pCmd->paramValues[0]]);
      }
      else {
         printf("\r\nUnknown drv command");
      }
}

enum dirState {
   ABC,
   ACB
};

void dir(char* paramStr, struct command* pCmd) {
   if (strncmp(paramStr, pCmd->paramWords[ABC], strlen(pCmd->paramWords[ABC])) == 0) {
      printf("\r\nDIR pin high -> Phase sequence is A->B->C");
      pCmd->paramValues[0] = ABC;
      HAL_GPIO_WritePin(GPIOA, DIR_Pin, GPIO_PIN_SET);
   }
   else if (strncmp(paramStr, pCmd->paramWords[ACB], strlen(pCmd->paramWords[ACB])) == 0) {
      printf("\r\nDIR pin low -> Phase sequence is A->C->B");
      pCmd->paramValues[0] = ACB;
      HAL_GPIO_WritePin(GPIOA, DIR_Pin, GPIO_PIN_RESET);
   }
   else if (strncmp(paramStr, pCmd->paramWords[STATE], strlen(pCmd->paramWords[STATE])) == 0) {
         printf("\r\ndrv state = %s", pCmd->paramWords[pCmd->paramValues[0]]);
   }
   else {
      printf("\r\nUnknown dir command");
   }
}

void i2c(char* paramStr, struct command* pCmd) {
   if (strncmp(paramStr, "get", 3) == 0) {
      printf("\r\nI2C Address:0x%02x", i2cAddress);
   }
   else if (strncmp(paramStr, "set", 3) == 0) {
      // Auto-detect base -> if address is hex, start with 0x... if decimal just type the decimal.
      i2cAddress = ((uint16_t)strtol(&paramStr[3+1], NULL, 0))<<1;
      printf("\r\nI2C Address = 0x%02x", i2cAddress);
   }
   else {
      printf("\r\nUnknown i2c command");
      void promt();
      return;
   }
}

void sys(char* paramStr, struct command* pCmd){
   printf("\r\nBuild no.:%d", BUILD);
   printf("\r\nBuild date: %s", BUILD_DATE_AND_TIME);
   printf("\r\nVersion:%d.%d", MAJOR_VERSION, MINOR_VERSION);
}

void ramWrite(uint8_t lowPartRegAddr, uint32_t regValue) {
   // Only 3 ram registers can be written to...
   uint8_t i2cDataWord[8] = {0}; // I2C Data Word without CRC
   i2cDataWord[0] = (DATA_LENGTH_32 << DATA_LEN_POS);
   i2cDataWord[0] |= (WRITE_OPERATION << RW_OPERATION_BIT_POS);
   i2cDataWord[1] = 0; // we know that the most significant part of the register address is 0x00
   i2cDataWord[2] = lowPartRegAddr;
   memcpy(&i2cDataWord[3], &regValue, 4);

   if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 7, 1000) != HAL_OK) {
      printf("\r\nHAL_I2C_Master_Transmit for write operation FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
   }
   else {
      printf("\r\nHAL_I2C_Master_Transmit() OK!");
      printf("\r\n0x%02x%02x%02x%02x loaded at Addr.: 0x%02x%02x",
            i2cDataWord[6],i2cDataWord[5], i2cDataWord[4], i2cDataWord[3], i2cDataWord[1], i2cDataWord[2]);
   }
}
/**
void ALGO_CTRL1(char* paramStr, struct command* pCmd) {
   uint8_t paramStartPos = 0;

   if (strncmp(paramStr, "SET", 3) == 0) {
      paramStartPos = 4;
      // Now read the user input value for the specific register...
      if (strncmp(&paramStr[paramStartPos], "0x", 2) == 0) {
         int regdata = (int)strtol(&paramStr[paramStartPos+1], NULL, 0);
         printf("\r\nValid user input for register value: 0x%x", regdata);
         ramWrite(0xec, regdata);
      }
      else {
         printf("\r\nNo valid user input for register value (%s)", &paramStr[paramStartPos]);
         return;
      }
   }
   else {
      printf("\r\nUNKNOWN ALGO_CTRL1 COMMAND");
   }
}

void ALGO_CTRL2(char* paramStr, struct command* pCmd) {
   uint8_t paramStartPos = 0;

   if (strncmp(paramStr, "SET", 3) == 0) {
      paramStartPos = 4;
      // Now read the user input value for the specific register...
      if (strncmp(&paramStr[paramStartPos], "0x", 2) == 0) {
         int regdata = (int)strtol(&paramStr[paramStartPos+1], NULL, 0);
         printf("\r\nValid user input for register value: 0x%x", regdata);
         ramWrite(0xee, regdata);
      }
      else {
         printf("\r\nNo valid user input for register value (%s)", &paramStr[paramStartPos]);
         return;
      }
   }
   else {
      printf("\r\nUNKNOWN ALGO_CTRL2 COMMAND");
   }
}
**/
void EEPROM(char* paramStr, struct command* pCmd) {
   // IN PROGRESS...
   uint8_t paramStartPos = 0;
   uint8_t i2cDataWord[8] = {0}; // I2C Data Word without CRC
   i2cDataWord[0] = (DATA_LENGTH_32 << DATA_LEN_POS);

   if (strncmp(paramStr, "WRITE", 5) == 0) {
      i2cDataWord[0] |= (WRITE_OPERATION << RW_OPERATION_BIT_POS);
      paramStartPos = 6;
      // Check if the second parameter is a letter or not
      if ((paramStr[paramStartPos] >= 65 )&&(paramStr[paramStartPos] <= 90 )) {

         if (strncmp(&paramStr[paramStartPos], "DEFAULT1", 8) == 0) {
            // If the second parameter is the word 'DEFAULT' the EEPROM WRITE command
            // will load the predefined register (in eepromRegValues) values into the
            // shadow registers and then execute the 'store in EEPROM' command.
            for (int i = 0; i < sizeof(eepromAddr)/2; i++) {
               i2cDataWord[1] = (eepromAddr[i] >> 8)&(0xff);
               i2cDataWord[2] = (eepromAddr[i])&(0xff);
               memcpy(&i2cDataWord[3], &eepromRegValues[i], 4);

               if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 7, 1000) != HAL_OK) {
                  printf("\r\nHAL_I2C_Master_Transmit for write operation FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
               }
               else {
                  printf("\r\nHAL_I2C_Master_Transmit() OK!\r\n");
                  printf("\r\n0x%02x%02x%02x%02x loaded at Addr.: 0x%02x%02x [%s]",
                        i2cDataWord[6],i2cDataWord[5], i2cDataWord[4], i2cDataWord[3], i2cDataWord[1], i2cDataWord[2], regNames[i]);
               }
            }
         } /*** "DEFAULT1" END ***/
         else if (strncmp(&paramStr[paramStartPos], "DEFAULT2", 8) == 0) {
            // If the second parameter is the word 'DEFAULT' the EEPROM WRITE command
            // will load the predefined register (in eepromRegValues) values into the
            // shadow registers and then execute the 'store in EEPROM' command.
            for (int i = 0; i < sizeof(eepromAddr)/2; i++) {
               i2cDataWord[1] = (eepromAddr[i] >> 8)&(0xff);
               i2cDataWord[2] = (eepromAddr[i])&(0xff);
               memcpy(&i2cDataWord[3], &eepromRegValues2[i], 4);

               if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 7, 1000) != HAL_OK) {
                  printf("\r\nHAL_I2C_Master_Transmit for write operation FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
               }
               else {
                  printf("\r\nHAL_I2C_Master_Transmit() OK!\r\n");
                  printf("\r\n0x%02x%02x%02x%02x loaded at Addr.: 0x%02x%02x [%s]",
                        i2cDataWord[6],i2cDataWord[5], i2cDataWord[4], i2cDataWord[3], i2cDataWord[1], i2cDataWord[2], regNames[i]);
               }
            }
         } /*** "DEFAULT2" END ***/
         // write specific value to the eeprom...
         else {
            int nameIndex = 0;
            for (int i = 0; i < sizeof(eepromAddr)/2; i++) {
               if (strncmp((char*)&paramStr[paramStartPos], regNames[i], strlen(regNames[i])) == 0) {
                  printf("\r\nFound Register name: %s & index: %d", regNames[i], i);
                  i2cDataWord[1] = (eepromAddr[i] >> 8)&(0xff);
                  i2cDataWord[2] = (eepromAddr[i])&(0xff);
                  paramStartPos += strlen(regNames[i]);
                  nameIndex = i;
                  break;
               }
            }
            // Now read the user input value for the specific register...
            uint8_t userInputRegValue[4] = {0};
            if (strncmp(&paramStr[paramStartPos+1], "0x", 2) == 0) {

               int number = (int)strtol(&paramStr[paramStartPos+1], NULL, 0);
               printf("\r\nValid user input for register value: 0x%x", number);
               memcpy(&userInputRegValue[0], &number, 4);
            }
            else {
               printf("\r\nNo valid user input for register value (%s)", &paramStr[paramStartPos]);
               return;
            }
            memcpy(&i2cDataWord[3], &userInputRegValue[0], 4);

            if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 7, 1000) != HAL_OK) {
               printf("\r\nHAL_I2C_Master_Transmit for write operation FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
            }
            else {
               printf("\r\nHAL_I2C_Master_Transmit() OK!\r\n");
               printf("\r\n0x%02x%02x%02x%02x loaded at Addr.: 0x%02x%02x [%s]",
                     i2cDataWord[6],i2cDataWord[5], i2cDataWord[4], i2cDataWord[3], i2cDataWord[1], i2cDataWord[2], regNames[nameIndex]);
            }

         }
         // All loaded into shadow memory. Now write it into EEPROM
         // First construct the
         i2cDataWord[0] |= (WRITE_OPERATION << RW_OPERATION_BIT_POS);
         i2cDataWord[1] = (eeprom2MemAddr >> 8)&(0xff);
         i2cDataWord[2] = (eeprom2MemAddr)&(0xff);
         memcpy(&i2cDataWord[3], &eeprom2MemRW[WRITE_EEPROM], 4);
         /*** TEST
         printf("\r\nI2C Data Word: "); for (int i = 0; i < 7; i++) {printf("0x%02x ", i2cDataWord[i]);}
          ***/
         if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 7, 1000) != HAL_OK) {
            printf("\r\nHAL_I2C_Master_Transmit for EEPROM WRITE FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
            return;
         }
         else {
            printf("\r\nData stored in EEPROM OK!\r\n");
         }
      }
   }
   else if (strncmp(paramStr, "READ", 4) == 0) {
      /************************************************************************
      In MCF8316A, EEPROM read procedure is as follows,
      1. Write 0x40000000 into register 0x0000EA to read the EEPROM data into
         the shadow registers (0x000080-0x0000AE).
      2. Wait for 100ms for the EEPROM read operation to complete.
      3. Read the shadow register values,1 or 2 registers at a time, using the
         I2C read command as explained in Section 7.6.2. Shadow register
         addresses are in the range of 0x000080-0x0000AE. Register address
         increases in steps of 2 for 32-bit read operation (since each address
         is a 16-bit location).
      ************************************************************************/
      //#1___
      i2cDataWord[0] |= (WRITE_OPERATION << RW_OPERATION_BIT_POS);
      i2cDataWord[1] = (eeprom2MemAddr >> 8)&(0xff);
      i2cDataWord[2] = (eeprom2MemAddr)&(0xff);
      memcpy(&i2cDataWord[3], &eeprom2MemRW[READ_EEPROM], 4);

      if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 7, 1000) != HAL_OK) {
         printf("\r\nHAL_I2C FUNCTION FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
         return;
      }
      else {
         printf("\r\nEEPROM prepared for read OK!");
      }
      //#2___ ...the print statements will for sure take some time... - no HalDelay() used.
      //#3_1
      i2cDataWord[0] |= (READ_OPERATION << RW_OPERATION_BIT_POS);
      paramStartPos = 5; // indicating the start of the register-read argument.
      if ((paramStr[paramStartPos] >= 65 )&&(paramStr[paramStartPos] <= 90 )) {
         // List all eeprom locations...
         if (strncmp(&paramStr[paramStartPos], "ALL", 1) == 0) {
            // we'll only check for the first letter since no register name
            // starts with an 'A', and we'll need to divide size of the
            // eepromAddr array since it is a array of shorts...
            for (int i = 0; i < sizeof(eepromAddr)/2; i++) {
               i2cDataWord[1] = (eepromAddr[i] >> 8)&(0xff);
               i2cDataWord[2] = (eepromAddr[i])&(0xff);
               if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 3, 1000) != HAL_OK) {
                  printf("\r\nHAL_I2C_Master_Receive() FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
               }
               else {
                  if (HAL_I2C_Master_Receive(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[4], 4, 1000) != HAL_OK) {
                     printf("\r\nHAL_I2C_Master_Receive() FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
                  }
                  else {
                     printf("\r\nData read:0x%02x%02x%02x%02x at Addr.: 0x%02x%02x", i2cDataWord[7],i2cDataWord[6], i2cDataWord[5], i2cDataWord[4], i2cDataWord[1], i2cDataWord[2]);
                  }
               }
            }
            printf("\r\nDone!");
         }
         else {
            for (int i = 0; sizeof(eepromAddr)/2; i++) {
               if (strncmp(&paramStr[paramStartPos], regNames[i], strlen(regNames[i])) == 0) {
                  printf("\r\nFound Register name: %s & index: %d", regNames[i], i);
                  i2cDataWord[1] = (eepromAddr[i] >> 8)&(0xff);
                  i2cDataWord[2] = (eepromAddr[i])&(0xff);
                  paramStartPos += strlen(regNames[i]);
                  break;
               }
            }
            ///
            if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 3, 1000) != HAL_OK) {
               printf("\r\nHAL_I2C_Master_Receive() FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
            }
            else {
               if (HAL_I2C_Master_Receive(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[4], 4, 1000) != HAL_OK) {
                  printf("\r\nHAL_I2C_Master_Receive() FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
               }
               else {
                  //memcpy(data, &i2cDataWord[4],4);
                  //printf("\r\nData read:x0%x at Addr.: 0x%02x%02x\r\n", (unsigned int)*data, i2cDataWord[1], i2cDataWord[2]);
                  printf("\r\nData read:0x%02x%02x%02x%02x at Addr.: 0x%02x%02x", i2cDataWord[7],i2cDataWord[6], i2cDataWord[5], i2cDataWord[4], i2cDataWord[1], i2cDataWord[2]);
               }
            }
         }
      }
   }
   else {
      printf("\r\nUNKNOWN OR INCORRECT COMMAND FORMAT");
      void promt();
      return;
   }
}

void RAM(char* paramStr, struct command* pCmd){
   uint8_t paramStartPos = 0;
   uint8_t i2cDataWord[8] = {0}; // I2C Data Word without CRC
   i2cDataWord[0] = (DATA_LENGTH_32 << DATA_LEN_POS);

   if (strncmp(paramStr, "READ", 4) == 0) {
      i2cDataWord[0] |= (READ_OPERATION << RW_OPERATION_BIT_POS);
      paramStartPos = 5;

      if ((paramStr[paramStartPos] >= 65 )&&(paramStr[paramStartPos] <= 90 )) {
         // List all eeprom locations...
         if (strncmp(&paramStr[paramStartPos], "ALL", 3) == 0) {
            for (int i = 0; i < sizeof(ramAddr)/2; i++) {  // dividing ramAddr by 2 since the sizeof counts bytes and the ramAddr is 16 bit...
               i2cDataWord[1] = (ramAddr[i] >> 8)&(0xff);
               i2cDataWord[2] = (ramAddr[i])&(0xff);
               if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 3, 1000) != HAL_OK) {
                  printf("\r\nHAL_I2C_Master_Receive() FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
               }
               else {
                  if (HAL_I2C_Master_Receive(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[4], 4, 1000) != HAL_OK) {
                     printf("\r\nHAL_I2C_Master_Receive() FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
                  }
                  else {
                     printf("\r\nData read:0x%02x%02x%02x%02x at Addr.: 0x%02x%02x = %s",
                           i2cDataWord[7],i2cDataWord[6], i2cDataWord[5], i2cDataWord[4], i2cDataWord[1], i2cDataWord[2], volatileRegNames[i]);
                  }
               }
            }
            printf("\r\nDone!");
         }
         else if (strncmp(&paramStr[paramStartPos], "FAULT", 5) == 0) {
            paramStartPos += 6;
            uint8_t numOfFaultDescriptions = 0;
            uint32_t ControllerFaultStatus = 0;
            char** faultDescriptions;

            if (strncmp(&paramStr[paramStartPos], "GATE", 4) == 0) {
               i2cDataWord[2] = 0xe0;
               numOfFaultDescriptions = 31-numOfGDFdescriptions;
               faultDescriptions = &gateDriveFaultDescription[0];
               printf("\r\nGATE_DRIVER_FAULT_STATUS:");
            }
            else if (strncmp(&paramStr[paramStartPos], "CTRL", 4) == 0) {
               i2cDataWord[2] = 0xe4;
               numOfFaultDescriptions = 31-numOfCFdescriptions;
               faultDescriptions = &controllerFaultDescription[0];
               printf("\r\nCONTROLLER_FAULT_STATUS:");
            }
            else {
               printf("\r\nUnknown RAM READ FAULT command.\r\n");
               return;
            }

            if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 3, 1000) != HAL_OK) {
               printf("\r\nHAL_I2C_Master_Transmit() FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
            }
            else {
               if (HAL_I2C_Master_Receive(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[4], 4, 1000) != HAL_OK) {
                  printf("\r\nHAL_I2C_Master_Receive() FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
               }
               else {
                  memcpy(&ControllerFaultStatus, &i2cDataWord[4], 4);

                  for (int i = 31; i >= numOfFaultDescriptions; i--) {
                     if (((ControllerFaultStatus >> i) & 0x1) == 0) {
                        printf("\r\nNO %s", faultDescriptions[31-i]);
                     }
                     else {
                        printf("\r\n%s", faultDescriptions[31-i]);
                     }
                  }
                  printf("\r\nData read:0x%02x%02x%02x%02x at Addr.: 0x%02x%02x", i2cDataWord[7],i2cDataWord[6], i2cDataWord[5], i2cDataWord[4], i2cDataWord[1], i2cDataWord[2]);
               }
            }
         }
         else {
            int valideReg = 0;
            for (int i = 0; i < sizeof(ramAddr)/2; i++) {
               if (strncmp(&paramStr[paramStartPos], volatileRegNames[i], strlen(volatileRegNames[i])) == 0) {
                  printf("\r\nFound Register name: %s & index: %d", volatileRegNames[i], i);
                  i2cDataWord[1] = (ramAddr[i] >> 8)&(0xff);
                  i2cDataWord[2] = (ramAddr[i])&(0xff);
                  valideReg = strlen(volatileRegNames[i]);
                  //paramStartPos += strlen(volatileRegNames[i]);
                  break;
               }
            }
            if (valideReg != 0) {
               if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 3, HAL_MAX_DELAY) != HAL_OK) {
               //if (HAL_I2C_Master_Transmit(&hi2c1, 0x2, (uint8_t*)&i2cDataWord[0], 3, HAL_MAX_DELAY) != HAL_OK) {
                  printf("\r\nHAL_I2C_Master_Transmit() FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
               }
               else {
                  if (HAL_I2C_Master_Receive(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[4], 4, HAL_MAX_DELAY) != HAL_OK) {
                  //if (HAL_I2C_Master_Receive(&hi2c1, 0x2, (uint8_t*)&i2cDataWord[4], 4, HAL_MAX_DELAY) != HAL_OK) {
                     printf("\r\nHAL_I2C_Master_Receive() FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
                  }
                  else {
                     printf("\r\nData read:0x%02x%02x%02x%02x at Addr.: 0x%02x%02x", i2cDataWord[7],i2cDataWord[6], i2cDataWord[5], i2cDataWord[4], i2cDataWord[1], i2cDataWord[2]);
                  }
               }
            }
            else {
               printf("\r\nVolatile register %s not recognized.", &paramStr[paramStartPos]);
            }
         }
      }
   } // READ END
   else {
      printf("\r\nUNKNOWN OR INCORRECT COMMAND FORMAT");
      void promt();
      return;
   }
}

void clear(char* paramStr, struct command* pCmd){

   if (strncmp(paramStr, "flt", 3) == 0) {
      printf("\r\nAttempt to clear the fault indication!\r\n");
      ramWrite((uint8_t) ramAddr[DEV_CTRL], eeprom2MemRW[CLR_FLT]);
      //ramWrite(0xea, 0x20000000); // Clears all faults

      /****
      if (strncmp(&paramStr[paramStartPos], "CLEAR", 5) == 0) {
         paramStartPos = 6;
         if (strncmp(&paramStr[paramStartPos], "ALL", 3) == 0) {
            ramWrite(0xea, 0x20000000); // Clears all faults
         }
         else if (strncmp(&paramStr[paramStartPos], "COUNT", 5) == 0) {
            ramWrite(0xea, 0x10000000); // Clears fault retry count
         }
         else {
            printf("\r\nNothing to clear...");
         }
      }
      else
      ****/



/****
      i2cDataWord[0] |= (WRITE_OPERATION << RW_OPERATION_BIT_POS);
      i2cDataWord[1] = (eeprom2MemAddr >> 8)&(0xff);
      i2cDataWord[2] = (eeprom2MemAddr)&(0xff);
      memcpy(&i2cDataWord[3], &eeprom2MemRW[CLR_FLT], 4);
      //// TEST
      // printf("\r\nI2C Data Word: "); for (int i = 0; i < 7; i++) {printf("0x%02x ", i2cDataWord[i]);}

      if (HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, (uint8_t*)&i2cDataWord[0], 7, 1000) != HAL_OK) {
         printf("\r\nHAL_I2C_Master_Transmit for RAM WRITE FAILED! Error code: 0x%x\r\n", (unsigned int)hi2c1.ErrorCode);
         return;
      }
      else {
         printf("\r\nClearing fault OK!\r\n");
      }
*****/
   }
}
/**
#define COMMAND_PARAMS 10
#define COMMAND_PARAM_LENGTH 10
#define COMMAND_NAME_LENGTH 20

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
  {"led", 3, 6, {"off", "on", "state"}, {0}, &led},
  {"drv", 3, 6, {"off", "on", "state"}, {0}, &drv},
  {"dir", 3, 6, {"abc", "acb", "state"}, {0}, &dir},
  {"brk", 3, 6, {"off", "on", "state"}, {0}, &brk},
  {"i2c", 2, 4, {"get", "set"}, {0}, &i2c},
  {"sys", 0, 0, {"__"}, {0}, &sys},
  {"clear", 1, 6, {"flt", "all"}, {0, 0}, &clear},
//  {"ALGO_CTRL1", 1, 4, {"SET", "GET"}, {0, 0}, &ALGO_CTRL1},
//  {"ALGO_CTRL2", 1, 4, {"SET", "GET"}, {0, 0}, &ALGO_CTRL2},
  {"EEPROM", 3, 6, {"READ", "WRITE"}, {0, 0}, &EEPROM},
  {"RAM", 2, 6, {"READ", "WRITE"}, {0, 0}, &RAM}
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
 	       mcuCmds[i].cmdFunction((char*)&termInput[0], (struct command*) &mcuCmds[i]);
 	     }
 	     else {
 	       //mcuCmds[i].cmdFunction((char*)&termInput[strlen(mcuCmds[i].name)+1], (int*) &mcuCmds[i].paramValues);
 	       mcuCmds[i].cmdFunction((char*)&termInput[strlen(mcuCmds[i].name)+1], (struct command*) &mcuCmds[i]);
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
