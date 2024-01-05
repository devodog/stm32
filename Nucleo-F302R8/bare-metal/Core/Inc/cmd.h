/*
 * cmd.h
 *
 *  Created on: Jan 3, 2024
 *      Author: dagak
 */
#include "main.h"

#ifndef INC_CMD_H_
#define INC_CMD_H_

void promt();
uint8_t executeCmd(char *termInput, int cmdLength);
void noOperation();


#endif /* INC_CMD_H_ */
