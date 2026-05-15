/*
 * driver.h
 *
 *  Created on: 13. mai 2026
 *      Author: dagak
 */

#ifndef INC_DRIVER_H_
#define INC_DRIVER_H_

uint16_t readHallSensors(void);
int pwmChannel(int ch);
void phaseTest(int dutyCycle, int ph);
int start(int dutyCycle);
void stop(void);
void run(int dc);

#endif /* INC_DRIVER_H_ */
