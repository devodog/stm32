/*
 * scd30.h
 *
 *  Created on: Jan 24, 2024
 *      Author: dagak
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "main.h"

#ifndef INC_SCD30_H_
#define INC_SCD30_H_

void ReadFirmwareVersion(void);
void ContinuousMeasurement(uint16_t);
void StopContinuousMeasurement(void);
void SetMeasurementInterval(uint16_t);
int GetDataReadyStatus(void);
int ReadMeasurement(uint8_t*, uint8_t);

#endif /* INC_SCD30_H_ */
