/*
 * cmd.h
 *
 *  Created on: Apr 27, 2026
 *      Author: dagak
 *
 */
#include "main.h"

#ifndef INC_CMD_H_
#define INC_CMD_H_

void promt();
void noOperation();

uint8_t executeCmd(char *termInput, int cmdLength);

#endif /* INC_CMD_H_ */
