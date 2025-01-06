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
   0x00101462, // 31-28 = 0, 27-24 = 0, 23-20 = 1 => i2c ADDRESS = 0x1
   0x4000F00F,
   0x41C01F00,
   0x1C450100,
   0x00200000,
   0x2433407D,
   0x000001A7
};

uint32_t eepromRegValues2[] = {
   0x44638C20,
   0x283AF064,
   0x0B6807D0,
   0x23066000,
   0x0C3181B0,
   0x1AAD0000,
   0x00000000,
   0x0000012C,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x5FE80206,
   0x74000000,
   0x00000000,
   0x00000000, // ADDRESS 0X0
   0x0000B000,
   0x40000000,
   0x00000100,
   0x00200000,
   0x00000000,
   0x00B3407D,
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
   DEVICE_CONFIG1, // 20 - 26 i2c ADDRESS
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
uint16_t ramAddr[] = {
   0x00E0,
   0x00E2,
   0x00E4,
   0x00E6,
   0x00E8,
   0x00EA,
   0x00EC,
   0x00EE,
   0x00F0,
   0x00F2,
   0x0210,
   0x0216,
   0x0410,
   0x043E,
   0x0440,
   0x0442,
   0x0466,
   0x0476,
   0x0478,
   0x047E,
   0x0480,
   0x0482,
   0x04BA,
   0x04BC,
   0x04D4,
   0x04D6,
   0x04D8,
   0x04DA,
   0x04E4,
   0x04E6,
   0x04E8,
   0x04EA,
   0x0524,
   0x053A,
   0x0548,
   0x05CC,
   0x05FC,
   0x05FE,
   0x067A,
   0x0684,
   0x06B8,
   0x06FC,
   0x0742,
   0x0744,
   0x0752,
   0x0756
};
char* volatileRegNames[] = {
   "GATE_DRIVER_FAULT_STATUS",   // Fault Status Register GATE_DRIVER_FAULT_STATUS Register(Address = E0h) [Reset = 00000000h]
   "CONTROLLER_FAULT_STATUS",    // Fault Status Register CONTROLLER_FAULT_STATUS Register(Address = E2h) [Reset = 00000000h]
   "ALGO_STATUS",                // System Status Register ALGO_STATUS Register (Address = E4h)[Reset = 00000000h]
   "MTR_PARAMS",                 // System Status Register MTR_PARAMS Register (Address = E6h)[Reset = 00000000h]
   "ALGO_STATUS_MPET",           // System Status Register ALGO_STATUS_MPET Register (Address =E8h) [Reset = 00000000h]
   "DEV_CTRL DEV_CTRL",          // Register (Address = EAh) [Reset = 00000000h]
   "ALGO_CTRL1",                 // Algorithm Control Register ALGO_CTRL1 Register (Address = ECh)[Reset = 00000000h]
   "ALGO_CTRL2",                 // Algorithm Control Register ALGO_CTRL2 Register (Address = EEh)[Reset = 00000000h]
   "CURRENT_PI",                 // Current PI Controller Register CURRENT_PI Register (Address = F0h)[Reset = 00000000h]
   "SPEED_PI",                   // Speed PI Controller Register SPEED_PI Register (Address = F2h) [Reset = 00000000h]
   "ALGORITHM_STATE",            // Current Algorithm State Register ALGORITHM_STATE Register (Address = 210h) [Reset = 00000000h]
   "FG_SPEED_FDBK",              // FG Speed Feedback Register FG_SPEED_FDBK Register (Address = 216h) [Reset = 00000000h]
   "BUS_CURRENT",                // Calculated DC Bus Current Register BUS_CURRENT Register (Address = 410h) [Reset = 00000000h]
   "PHASE_CURRENT_A",            // Measured Current on Phase A Register PHASE_CURRENT_A Register (Address = 43Eh) [Reset = 00000000h]
   "PHASE_CURRENT_B",            // Measured Current on Phase B Register PHASE_CURRENT_B Register (Address = 440h) [Reset = 00000000h]
   "PHASE_CURRENT_C",            // Measured Current on Phase C Register PHASE_CURRENT_C Register (Address = 442h) [Reset = 00000000h]
   "CSA_GAIN_FEEDBACK",          // CSA Gain Register CSA_GAIN_FEEDBACK Register (Address = 466h) [Reset = 00000000h]
   "VOLTAGE_GAIN_FEEDBACK",      // Voltage Gain Register VOLTAGE_GAIN_FEEDBACK Register (Address = 476h) [Reset = 00000000h]
   "VM_VOLTAGE",                 // VM Voltage Register VM_VOLTAGE Register (Address = 478h) [Reset = 00000000h]
   "PHASE_VOLTAGE_VA",           // Phase Voltage Register PHASE_VOLTAGE_VA Register (Address = 47Eh) [Reset = 00000000h]
   "PHASE_VOLTAGE_VB",           // Phase Voltage Register PHASE_VOLTAGE_VB Register (Address = 480h) [Reset = 00000000h]
   "PHASE_VOLTAGE_VC",           // Phase Voltage Register PHASE_VOLTAGE_VC Register (Address = 482h) [Reset = 00000000h]
   "SIN_COMMUTATION_ANGLE",      // Sine of Commutation Angle SIN_COMMUTATION_ANGLE Register (Address = 4BAh) [Reset = 00000000h]
   "COS_COMMUTATION_ANGLE",      // Cosine of Commutation Angle COS_COMMUTATION_ANGLE Register (Address = 4BCh) [Reset = 00000000h]
   "IALPHA",                     // IALPHA Current Register IALPHA Register (Address = 4D4h) [Reset = 00000000h]
   "IBETA",                      // IBETA Current Register IBETA Register (Address = 4D6h) [Reset = 00000000h]
   "VALPHA",                     // VALPHA Voltage Register VALPHA Register (Address = 4D8h) [Reset = 00000000h]
   "VBETA",                      // VBETA Voltage Register VBETA Register (Address = 4DAh) [Reset = 00000000h]
   "ID",                         // Measured d-axis Current Register ID Register (Address = 4E4h) [Reset = 00000000h]
   "IQ",                         // Measured q-axis Current Register IQ Register (Address = 4E6h) [Reset = 00000000h]
   "VD",                         // VD Voltage Register VD Register (Address = 4E8h) [Reset = 00000000h]
   "VQ",                         // VQ Voltage Register VQ Register (Address = 4EAh) [Reset = 00000000h]
   "IQ_REF_ROTOR_ALIGN",         // Align Current Reference IQ_REF_ROTOR_ALIGN Register (Address = 524h) [Reset = 00000000h]
   "SPEED_REF_OPEN_LOOP",        // Open Loop Speed Register SPEED_REF_OPEN_LOOP Register (Address = 53Ah) [Reset = 00000000h]
   "IQ_REF_OPEN_LOOP",           // Open Loop Current Reference IQ_REF_OPEN_LOOP Register (Address = 548h) [Reset = 00000000h]
   "SPEED_REF_CLOSED_LOOP",      // Speed Reference Register SPEED_REF_CLOSED_LOOP Register (Address = 5CCh) [Reset = 00000000h]
   "ID_REF_CLOSED_LOOP",         // Reference for Current Loop Register ID_REF_CLOSED_LOOP Register (Address = 5FCh) [Reset = 00000000h]
   "IQ_REF_CLOSED_LOOP",         // Reference for Current Loop Register IQ_REF_CLOSED_LOOP Register (Address = 5FEh) [Reset = 00000000h]
   "ISD_STATE",                  // ISD State Register ISD_STATE Register (Address = 67Ah) [Reset = 00000000h]
   "ISD_SPEED",                  // ISD Speed Register ISD_SPEED Register (Address = 684h) [Reset = 00000000h]
   "IPD_STATE",                  // IPD State Register IPD_STATE Register (Address = 6B8h) [Reset = 00000000h]
   "IPD_ANGLE",                  // Calculated IPD Angle Register IPD_ANGLE Register (Address = 6FCh) [Reset = 00000000h]
   "ED",                         // Estimated BEMF EQ Register ED Register (Address = 742h) [Reset = 00000000h]
   "EQ",                         // Estimated BEMF ED Register EQ Register (Address = 744h) [Reset = 00000000h]
   "SPEED_FDBK",                 // Speed Feedback Register SPEED_FDBK Register (Address = 752h) [Reset = 00000000h]
   "THETA_EST"                   // Estimated Motor Position Register THETA_EST Register (Address = 756h) [Reset = 00000000h]
};
/******************************************************************************
reg.addr. 0xe0    GATE_DRIVER_FAULT_STATUS Fault Status Register
31 DRIVER_FAULT   R 0h Logic OR of driver fault registers
30 BK_FLT         R 0h Buck fault, 0h = No buck regulator fault condition is detected, 1h = Buck regulator fault condition is detected
29 RESERVED       R 0h Reserved
28 OCP            R 0h Overcurrent protection status, 0h = No overcurrent condition is detected, 1h = Overcurrent condition is detected
27 NPOR           R 0h Supply power on reset, 0h = Power on reset condition is detected on VM, 1h = No power-on-reset condition is detected on VM
26 OVP            R 0h Supply overvoltage protection status, 0h = No overvoltage condition is detected on VM, 1h = Overvoltage condition is detected on VM
25 OT             R 0h Overtemperature fault status, 0h = No overtemperature warning / shutdown is detected, 1h = Overtemperature warning / shutdown is detected
24 RESERVED       R 0h Reserved
23 OTW            R 0h Overtemperature warning status, 0h = No overtemperature warning is detected, 1h = Overtemperature warning is detected
22 TSD            R 0h Overtemperature shutdown status, 0h = No overtemperature shutdown is detected, 1h = Overtemperature shutdown is detected
21 OCP_HC         R 0h Overcurrent status on high-side switch of OUTC, 0h = No overcurrent detected on high-side switch of OUTC, 1h = Overcurrent detected on high-side switch of OUTC
20 OCP_LC         R 0h Overcurrent status on low-side switch of OUTC, 0h = No overcurrent detected on low-side switch of OUTC, 1h = Overcurrent detected on low-side switch of OUTC
19 OCP_HB         R 0h Overcurrent status on high-side switch of OUTB, 0h = No overcurrent detected on high-side switch of OUTB, 1h = Overcurrent detected on high-side switch of OUTB
18 OCP_LB         R 0h Overcurrent status on low-side switch of OUTB, 0h = No overcurrent detected on low-side switch of OUTB, 1h = Overcurrent detected on low-side switch of OUTB
17 OCP_HA         R 0h Overcurrent status on high-side switch of OUTA, 0h = No overcurrent detected on high-side switch of OUTA, 1h = Overcurrent detected on high-side switch of OUTA
16 OCP_LA         R 0h Overcurrent status on low-side switch of OUTA, 0h = No overcurrent detected on low-side switch of OUTA, 1h = Overcurrent detected on low-side switch of OUTA
15 RESERVED       R 0h Reserved
14 OTP_ERR        R 0h One-time programmable (OTP) error, 0h = No OTP error is detected, 1h = OTP Error is detected
13 BUCK_OCP       R 0h Buck regulator overcurrent status, 0h = No buck regulator overcurrent is detected, 1h = Buck regulator overcurrent is detected
12 BUCK_UV        R 0h Buck regulator undervoltage status, 0h = No buck regulator undervoltage is detected, 1h = Buck regulator undervoltage is detected
11 VCP_UV         R 0h Charge pump undervoltage status, 0h = No charge pump undervoltage is detected, 1h = Charge pump undervoltage is detected
10-0 RESERVED     R 0h Reserved
******************************************************************************/
uint8_t numOfGDFdescriptions = 31 - 11;
char* gateDriveFaultDescription[] = {
/*bit31*/   "Gate Driver fault condition is detected",
/*bit30*/   "Buck regulator fault condition is detected",
/*bit29*/   "Reserved info",
/*bit28*/   "Overcurrent condition is detected",
/*bit27*/   "No power-on-reset condition is detected on VM",
/*bit26*/   "Overvoltage condition is detected on VM",
/*bit25*/   "Overtemperature warning / shutdown is detected",
/*bit24*/   "Reserved info",
/*bit23*/   "Overtemperature warning is detected",
/*bit22*/   "Overtemperature shutdown is detected",
/*bit21*/   "Overcurrent detected on high-side switch of OUTC",
/*bit20*/   "Overcurrent detected on low-side switch of OUTC",
/*bit19*/   "Overcurrent detected on high-side switch of OUTB",
/*bit18*/   "Overcurrent detected on low-side switch of OUTB",
/*bit17*/   "Overcurrent detected on high-side switch of OUTA",
/*bit16*/   "Overcurrent detected on low-side switch of OUTA",
/*bit15*/   "Reserved info",
/*bit14*/   "OTP Error is detected",
/*bit13*/   "Buck regulator overcurrent is detected",
/*bit12*/   "Buck regulator undervoltage is detected",
/*bit11*/   "Charge pump undervoltage is detected"
/*bit10*/   //"Reserved info"
};

/******************************************************************************
reg.addr. 0xe2 CONTROLLER_FAULT_STATUS Fault Status Register
31 CONTROLLER_FAULT  R 0h Logic OR of controller fault status registers, 0h = No controller fault condition is detected, 1h = Controller fault condition is detected
30 RESERVED          R 0h Reserved
29 IPD_FREQ_FAULT    R 0h Indicates IPD frequency fault, 0h = No IPD frequency fault detected, 1h = IPD frequency fault detected
28 IPD_T1_FAULT      R 0h Indicates IPD T1 fault, 0h = No IPD T1 fault detected, 1h = IPD T1 fault detected
27 IPD_T2_FAULT      R 0h Indicates IPD T2 fault, 0h = No IPD T2 fault detected, 1h = IPD T2 fault detected
26 BUS_CURRENT_LIMIT_STATUS R 0h Indicates status of bus current limit, 0h = No bus current limit fault detected, 1h = Bus current limit fault detected
25 MPET_IPD_FAULT    R 0h Indicates error during resistance and inductance measurement, 0h = No MPET IPD fault detected, 1h = MPET IPD fault detected
24 MPET_BEMF_FAULT   R 0h Indicates error during BEMF constant measurement, 0h = No MPET BEMF fault detected, 1h = MPET BEMF fault detected
23 ABN_SPEED         R 0h Indicates abnormal speed motor lock condition, 0h = No abnormal speed fault detected, 1h = Abnormal speed fault detected
22 ABN_BEMF          R 0h Indicates abnormal BEMF motor lock condition, 0h = No abnormal BEMF fault detected, 1h = Abnormal BEMF fault detected
21 NO_MTR            R 0h Indicates no motor fault, 0h = No motor fault not detected, 1h = No motor fault detected
20 MTR_LCK           R 0h Indicates when one of the motor lock is triggered, 0h = Motor lock fault not detected, 1h = Motor lock fault detected
19 LOCK_ILIMIT       R 0h Indicates lock Ilimit fault, 0h = No lock current limit fault detected, 1h = Lock current limit fault detected
18 HW_LOCK_ILIMIT    R 0h Indicates hardware lock Ilimit fault, 0h = No hardware lock current limit fault detected, 1h = Hardware lock current limit fault detected
17 MTR_UNDER_VOLTAGE R 0h Indicates motor undervoltage fault, 0h = No motor undervoltage detected, 1h = Motor undervoltage detected
16 MTR_OVER_VOLTAGE  R 0h Indicates motor overvoltage fault, 0h = No motor overvoltage detected, 1h = Motor overvoltage detected
15 SPEED_LOOP_SATURATION R 0h Indicates speed loop saturation, 0h = No speed loop saturation detected, 1h = Speed loop saturation detected
14 CURRENT_LOOP_SATURATION R 0h Indicates current loop saturation, 0h = No current loop saturation detected, 1h = Current loop saturation detected
13-3 RESERVED R 0h Reserved
2 RESERVED R 0h Reserved
1 RESERVED R 0h Reserved
0 RESERVED R 0h Reserved
******************************************************************************/
uint8_t numOfCFdescriptions = 31-14;
char* controllerFaultDescription[] = {
/*bit31*/   "Controller fault condition is detected",
/*bit30*/   "Bit is reserved",
/*bit29*/   "IPD frequency fault detected",
/*bit28*/   "IPD T1 fault detected",
/*bit27*/   "IPD T2 fault detected",
/*bit26*/   "Bus current limit fault detected",
/*bit25*/   "MPET IPD fault detected",
/*bit24*/   "MPET BEMF fault detected",
/*bit23*/   "Abnormal speed fault detected",
/*bit22*/   "Abnormal BEMF fault detected",
/*bit21*/   "Missing motor fault detected",
/*bit20*/   "Motor lock fault detected",
/*bit19*/   "Lock current limit fault detected",
/*bit18*/   "Hardware lock current limit fault detected",
/*bit17*/   "Motor undervoltage detected",
/*bit16*/   "Motor overvoltage detected",
/*bit15*/   "Speed loop saturation detected",
/*bit14*/   "Current loop saturation detected"
};


#endif /* INC_MCF8316A_H_ */
