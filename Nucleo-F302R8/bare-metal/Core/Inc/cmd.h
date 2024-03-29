/*
 * cmd.h
 *
 *  Created on: Jan 3, 2024
 *      Author: dagak
 */
#include "main.h"

#ifndef INC_CMD_H_
#define INC_CMD_H_
enum TIM_MODE {
   ONE_SHOT,
   REPEAT
};

void promt();
uint8_t executeCmd(char *termInput, int cmdLength);
void noOperation();
void CO2(char* paramStr, int* paramValues);

#endif /* INC_CMD_H_ */
