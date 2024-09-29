/*
 * mcf8316a.h
 *
 *  Created on: Sep 12, 2024
 *      Author: dagak
 */

#ifndef INC_MCF8316A_H_
#define INC_MCF8316A_H_
uint16_t eeprom2MemAddr = 0x000000EA;
//                        READ EEPROM  WRITE EEPROM
uint32_t eeprom2MemRW[] = {0x40000000, 0x8A500000};
enum EEPROM_RW {
   READ_EEPROM = 0,
   WRITE_EEPROM
};

// Register addresses
uint16_t eepromAddr[] = {
      0x00000080,
      0x00000082,
      0x00000084,
      0x00000086,
      0x00000088,
      0x0000008A,
      0x0000008C,
      0x0000008E,
      0x00000094,
      0x00000096,
      0x00000098,
      0x0000009A,
      0x0000009C,
      0x0000009E,
      0x00000090,
      0x00000092,
      0x000000A4,
      0x000000A6,
      0x000000A8,
      0x000000AA,
      0x000000AC,
      0x000000AE,
      0x000000A0,
      0x000000A2
};

// Recommended Default Register Values
uint32_t eepromRegValues[] = {
      0x64738C20,
      0x28200000,
      0x0B6807D0,
      0x2306600C,
      0x0D3201B5,
      0x1BAD0000,
      0x00000000,
      0x00000000,
      0x00000000,
      0x00000000,
      0x00000000,
      0x000D0000,
      0x00000000,
      0x00000000,
      0x3EC80106,
      0x70D00888,
      0x00000000,
      0x00101462,
      0x4000F00F,
      0x41C01F00,
      0x1C450100,
      0x00200000,
      0x2433407D,
      0x000001A7
};

enum registerIndex {
   ISD_CONFIG = 0,
   REV_DRIVE_CONFIG,
   MOTOR_STARTUP1,
   MOTOR_STARTUP2,
   CLOSED_LOOP1,
   CLOSED_LOOP2,
   CLOSED_LOOP3,
   CLOSED_LOOP4,
   SPEED_PROFILES1,
   SPEED_PROFILES2,
   SPEED_PROFILES3,
   SPEED_PROFILES4,
   SPEED_PROFILES5,
   SPEED_PROFILES6,
   FAULT_CONFIG1,
   FAULT_CONFIG2,
   PIN_CONFIG,
   DEVICE_CONFIG1,
   DEVICE_CONFIG2,
   PERI_CONFIG1,
   GD_CONFIG1,
   GD_CONFIG2,
   INT_ALGO_1,
   INT_ALGO_2
};

char* regNames[] = {
   "ISD_CONFIG",
   "REV_DRIVE_CONFIG",
   "MOTOR_STARTUP1",
   "MOTOR_STARTUP2",
   "CLOSED_LOOP1",
   "CLOSED_LOOP2",
   "CLOSED_LOOP3",
   "CLOSED_LOOP4",
   "SPEED_PROFILES1",
   "SPEED_PROFILES2",
   "SPEED_PROFILES3",
   "SPEED_PROFILES4",
   "SPEED_PROFILES5",
   "SPEED_PROFILES6",
   "FAULT_CONFIG1",
   "FAULT_CONFIG2",
   "PIN_CONFIG",
   "DEVICE_CONFIG1",
   "DEVICE_CONFIG2",
   "PERI_CONFIG1",
   "GD_CONFIG1",
   "GD_CONFIG2",
   "INT_ALGO_1",
   "INT_ALGO_2"
};


#endif /* INC_MCF8316A_H_ */
