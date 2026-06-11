/*
 * driver.h
 *
 *  Created on: 13. mai 2026
 *      Author: dagak
 */

#ifndef INC_DRIVER_H_
#define INC_DRIVER_H_

enum State{
   IDLE,
   STARTING_FORWARD,
   STARTING_REVERSE,
   RUNNING_FORWARD,
   RUNNING_REVERSE,
   TESTING
};


uint16_t readHallSensors(void);
int pwmChannel(int ch);
void phaseTest(int dutyCycle, int ph);
int start(int dutyCycle);
void stop(void);
void pwmUpdate(int dc);
void runTest(int dc);

#endif /* INC_DRIVER_H_ */
