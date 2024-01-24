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
#include "scd30.h"
#include "appver.h"

#define SENSIRION_ADDRESS 0x61 << 1
// Sensirion CO2 sensor device I2C register address
#define GET_DATA_READY_STATUS 0x0202
#define READ_MEASURMENT 0x0300

#define COMMAND_PARAMS 10
#define COMMAND_PARAM_LENGTH 10
#define COMMAND_NAME_LENGTH 20
#define NUMBERS_OF_MCU_COMMANDS 2

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
extern I2C_HandleTypeDef hi2c1;

extern uint8_t led2;
extern uint8_t timMode;

int timRepeat = 1;
int timRepeatCount = 0;

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
      HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

      // Poll ADC1 Peripheral & TimeOut = 1mSec
      HAL_ADC_PollForConversion(&hadc1, 1);
      // Read The ADC Conversion Result - using 3300 + 400 offset to
      // calculate the analog value
      printf("\r\nAA Battery voltage: %ld mV", 3700*HAL_ADC_GetValue(&hadc1)/4096);
   }
   else if (strncmp(paramStr, "HELP", 2) == 0){
      printf("\r\nThe ADC command takes the following parameters\r\n");
      printf("RO = Read Once\r\n" \
            "AVRAGE (not implemented)\r\nPOLL (not implemented)\r\nHELP = this printout.");
   }
   else {
      printf("\r\nUNKNOWN ADC COMMAND");
   }
}

void TIM(char* paramStr, int* paramValues) {
   if (strncmp(paramStr, "OS", 2) == 0) {      
      __HAL_TIM_SET_AUTORELOAD(&htim2, atoi(&paramStr[3]));
      printf("\r\nOne Shot timer with period: %d", (int)htim2.Init.Period);
      // The timer2 struct is updated accordingly to the __HAL_TIM_SET_AUTORELOAD() function.
      
      HAL_TIM_Base_Start_IT(&htim2);
   }
   else if (strncmp(paramStr, "PERIOD", 6) == 0) {
      __HAL_TIM_SET_AUTORELOAD(&htim2, atoi(&paramStr[7]));
      printf("\r\nAuto-reload period: %d", (int)htim2.Init.Period);
   }
   else if (strncmp(paramStr, "REPEAT", 6) == 0) {
      timRepeat = atoi(&paramStr[7]);
      timRepeatCount = 0;
      HAL_TIM_Base_Start_IT(&htim2);
      timMode = REPEAT;
      printf("\r\nAuto-reload repeat % times", timRepeat);
   }
   else if (strncmp(paramStr, "HELP", 2) == 0){
      printf("\r\nSome help text for the Timer CountDown (TCD) command-set here...");
   }
   else {
      printf("\r\nUNKNOWN TCD COMMAND");
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

void CO2(char* paramStr, int* paramValues) {
   if (strncmp(paramStr, "START", 5) == 0) {
      ContinuousMeasurement(0);
   }
   else if (strncmp(paramStr, "STOP", 4) == 0) {
      StopContinuousMeasurement();
   }
   else if (strncmp(paramStr, "VERSION", 7) == 0) {
      ReadFirmwareVersion();
   }
   else if (strncmp(paramStr, "INTERVAL", sizeof("INTERVAL")) == 0){
      SetMeasurementInterval(2); // will probably only work with 2 sec interval since the crc is pre-calculated for this value.
   }
   else if (strncmp(paramStr, "READ", 4) == 0) {
      uint8_t data[20];

      if (1 == ReadMeasurement(data, sizeof(data))) {
         // CO2 concentration
         float co2Concentration;
         float temperature;
         unsigned int tempU32;
         // read data is in a buffer. In case of I2C CRCs have been removed
         // beforehand. Content of the buffer is the following
         /****************************
         unsigned char buffer[4];
         buffer[0] = 0x43; // MMSB CO2
         buffer[1] = 0xDB; // MLSB CO2
         buffer[2] = 0x8C; // LMSB CO2
         buffer[3] = 0x2E; // LLSB CO2
         *****************************/
         // cast 4 bytes to one unsigned 32 bit integer
         tempU32 = (unsigned int)((((unsigned int)data[0]) << 24) |
         (((unsigned int)data[1]) << 16) |
         (((unsigned int)data[3]) << 8) |
         ((unsigned int)data[4]));
         // cast unsigned 32 bit integer to 32 bit float
         co2Concentration = *(float*)&tempU32; // co2Concentration = 439.09f

         tempU32 = (unsigned int)((((unsigned int)data[6]) << 24) |
         (((unsigned int)data[7]) << 16) |
         (((unsigned int)data[9]) << 8) |
         ((unsigned int)data[10]));
         // cast unsigned 32 bit integer to 32 bit float
         temperature = *(float*)&tempU32; // co2Concentration = 439.09f

         //printf("\r\n");
         //for (int i = 0; i<18; i++) {
         //   printf("0x%02x", data[i]);
         //}
         printf("\r\nco2Concentration = %f", co2Concentration);
         printf("\r\ntemperature = %f", temperature);
      }
      else {
         printf("\r\nReading sensor-data failed!");
      }
   }
   else if (strncmp(paramStr, "HELP", 4) == 0){
      printf("\r\nSome help text for the CO2 Measurement command-set here...");
   }
   /********************
   else if (strncmp(paramStr, "VERSION", 6) == 0) {
      uint8_t firmwareVersion[4] = {0xd1,0,0,0};
      uint16_t firmware = 0xD100;

      // Send a specific command to the Sensiron I2C slave... the command is a two byte register address...
      HAL_I2C_Master_Transmit(&hi2c1, SENSIRION_ADDRESS, firmwareVersion, 2, 1000);

      if (HAL_I2C_Mem_Read(&hi2c1, SENSIRION_ADDRESS, firmware, I2C_MEMADD_SIZE_16BIT, &firmwareVersion[0], 3, 1000) != HAL_OK) {
         printf("\r\nHAL_I2C_Mem_Read() FAILED!");
      }
      else {
         printf("\r\nSensiron SCD30 Ver.:0x%02x.0x%02x crc=0x%02x", firmwareVersion[0],firmwareVersion[1], firmwareVersion[2]);
      }
   }
   *************/

   else {
      printf("\r\nUNKNOWN TCD COMMAND");
   }
}

// Command array initialization
struct command mcuCmds [] = {
  {"LED", 3, 6, {"ON", "OFF", "BLINK", "HELP"}, {0, 1, 500, 0}, &LED},
  {"ADC", 4, 7, {"RO", "AVRAGE", "POLL", "HELP"}, {0, 10, 500, 0}, &ADC},
  {"TCD", 4, 7, {"OS", "PERIOD", "REPEAT", "HELP"}, {0, 500, 10, 0}, &TIM},
  {"CO2", 4, 7, {"READ", "VERSION", "INTERVAL", "HELP"}, {0, 1000, 60, 0}, &CO2},
  {"SYS", 3, 4, {"BN", "BD", "VER"}, {0, 0, 0}, &SYS}
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
