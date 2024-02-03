/*
 * cmd.c
 *
 *  Created on: Jan 3, 2024
 *      Author: dagak
 *
 *  Minimalistic code for Sensirion CO2 sensor.
 *
 */


#include "scd30.h"

#define SENSIRION_ADDRESS 0x61 << 1
// Sensirion CO2 sensor device I2C register address
#define GET_DATA_READY_STATUS 0x0202
#define READ_MEASURMENT 0x0300

extern I2C_HandleTypeDef hi2c3;

void ReadFirmwareVersion() {
   uint8_t firmwareVersion[4] = {0xd1,0,0,0};
   uint16_t firmware = 0xD100;

   // Send a specific command to the Sensiron I2C slave... the command is a two byte register address...
   HAL_I2C_Master_Transmit(&hi2c3, SENSIRION_ADDRESS, firmwareVersion, 2, 1000);

   if (HAL_I2C_Mem_Read(&hi2c3, SENSIRION_ADDRESS, firmware, I2C_MEMADD_SIZE_16BIT, &firmwareVersion[0], 3, 1000) != HAL_OK) {
      printf("\r\nHAL_I2C_Mem_Read() FAILED!");
   }
   else {
      printf("\r\nSensiron SCD30 Ver.:0x%02x.0x%02x crc=0x%02x", firmwareVersion[0],firmwareVersion[1], firmwareVersion[2]);
   }
}

void ContinuousMeasurement(uint16_t AmbientPressureCompensation) {
   uint8_t cm[5] = {0, 0x10, 0, 0, 0x81};
   cm[2] = (AmbientPressureCompensation>>8) & 0xff;
   cm[3] = AmbientPressureCompensation & 0xff;

   if (HAL_I2C_Master_Transmit(&hi2c3, SENSIRION_ADDRESS, cm, 5, 1000)!= HAL_OK) {
      printf("\r\nStarting Continuous Measurement FAILED!");
   }
   else {
      printf("\r\nContinuous Measurement started.");
   }
}

void StopContinuousMeasurement() {
   uint8_t cm[2] = {0x01, 0x04};
   if (HAL_I2C_Master_Transmit(&hi2c3, SENSIRION_ADDRESS, cm, 2, 1000)!= HAL_OK) {
      printf("\r\nStopping Continuous Measurement FAILED!");
   }
   else {
      printf("\r\nContinuous Measurement stopped.");
   }
}

void SetMeasurementInterval(uint16_t interval) {
   uint8_t cm[5] = {0x46, 0x00, 0x00, 0x02, 0xE3};
   cm[2] = (interval>>8) & 0xff;
   cm[3] = interval & 0xff;

   if (HAL_I2C_Master_Transmit(&hi2c3, SENSIRION_ADDRESS, cm, 5, 1000)!= HAL_OK) {
      printf("\r\nSetting Measurement Interval FAILED!");
   }
   else {
      printf("\r\nMeasurement Interval is set.");
   }
}

int GetDataReadyStatus() {
   uint8_t cm[3] = {0x02, 0x02, 0}; //Get data ready status

   if (HAL_I2C_Master_Transmit(&hi2c3, SENSIRION_ADDRESS, cm, 2, 1000)!= HAL_OK) {
      printf("\r\nGet Data Ready Status FAILED!");
      return -1;
   }
   else {
      HAL_Delay(3);
      HAL_I2C_Mem_Read(&hi2c3, SENSIRION_ADDRESS, GET_DATA_READY_STATUS, I2C_MEMADD_SIZE_16BIT, cm, 3, 1000);
      printf("\r\nReady Status.:0x%02x|0x%02x|0x%02x", cm[0],cm[1], cm[2]);
      if ((cm[0]==0)&&(cm[1]==1))
         return 1;
      else
         return 0;
   }
}

int ReadMeasurement(uint8_t* data, uint8_t len) {
   uint8_t cm[2] = {0x03, 0x00};

   if (HAL_I2C_Master_Transmit(&hi2c3, SENSIRION_ADDRESS, cm, 2, 1000)!= HAL_OK) {
      printf("\r\nRead Measurement FAILED!");
      return -1;
   }
   else {
      HAL_Delay(5); // Should we use the GetDataReadyStatus() prior to this?
      HAL_I2C_Mem_Read(&hi2c3, SENSIRION_ADDRESS, GET_DATA_READY_STATUS, I2C_MEMADD_SIZE_16BIT, data, len, 1000);
      printf("\r\nMeasurement read.");
      return 1;
   }
}
//- (De-)Activate continuous calculation of reference value for automatic self-calibration (ASC)
//- Set external reference value for forced recalibration (FRC)
//- Set temperature offset for onboard RH/T sensor
//- Altitude compensation

void SoftReset() {

}
