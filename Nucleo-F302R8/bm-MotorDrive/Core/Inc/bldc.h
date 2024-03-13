/*
 * bldc.h
 *
 * Created on: Mar 9, 2024
 * Author: dagak
 *
 * Software Device Driver for DRV8300DIPWR
 * MCU interface is 6 individual gate control signals. These control signals to
 * be connected to the MCU's 6 GPIO output ports, which is then to provide Six-
 * step commutation for a 3-phase inverter giving power to a BLDC Motor.
 *
 */

#ifndef INC_BLDC_H_
#define INC_BLDC_H_

/**
 * Commutation table = controls the half-bridge 3-phase inverter switches that
 * constitutes commutator.
 * Each value in the table will be a 6 bit-value indicating which inverter
 * switch to open and which to keep closed.
 *
 * Half-bridge Switch sequence...
 * Phase A Switch 1 & Swtch 2 ==> S1PA & S2PA = Phase A
 * S1PB, S2PB, S1PC, S2PC
 *
 * PORTA will use PA0, PA1, PA2, PA3, PA4 and PA5 as output.
 *
 * Commutation states
 *                  Q#2  Q#1  Q#4  Q#3  Q#6  Q#5
 *                  PA1  PA0  PA3  PA2  PA5  PA4
 *   H3  H2  H1  |  Q1L  Q1H  Q2L  Q2H  Q3L  Q3H
 *    1   0   1  |   0    1    1    0    0    0
 *    0   0   1  |   0    1    0    0    1    0
 *    0   1   1  |   0    0    0    1    1    0
 *    0   1   0  |   1    0    0    1    0    0
 *    1   1   0  |   1    0    0    0    0    1
 *    1   0   0  |   0    0    1    0    0    1
 *
 *
 * LOW SIDE INVERTED IN DRV8300DIPWR & ALIGNED WITH THE INVERTER SCHEMATICS
 *
 *                  Q#1  Q#2  Q#3  Q#4  Q#5  Q#5
 *                  PA0  PA1  PA2  PA3  PA4  PA5
 *   H3  H2  H1  |  Q1H  Q1L  Q2H  Q2L  Q3H  Q3L
 *    1   0   1  |   1    1    0    0    0    1 = 0x23 [5]
 *    0   0   1  |   1    1    0    1    0    0 = 0x0b [1]
 *    0   1   1  |   0    1    1    1    0    0 = 0x0e [3]
 *    0   1   0  |   0    0    1    1    0    1 = 0x2c [2]
 *    1   1   0  |   0    0    0    1    1    1 = 0x38 [6]
 *    1   0   0  |   0    1    0    0    1    1 = 0x32 [4]
 *
 *    pwm mask = 0x15
 *
 *    Switching frequency is experimentally chosen to be 1/800 µs => giving a total ARR value of 57599
 *    For a 50% Duty Cycle the ON- and OFF-Time will be 400 µs => 28799
 *    Making 8 µs ON the smallest forward drive value => DC = 1% - not sure if this will cause any movement.
 *
 */
uint8_t invSwitchState[6] = {0x0b, 0x2c, 0x0e, 0x32, 0x23, 0x38};
uint8_t pwmPinOff[6] = {~0x01, ~0x04, ~0x04, ~0x10, ~0x01, ~0x10};

// On:
// GPIOC->ODR = invSwitchState[6]

// Off:
// GPIOC->ODR = invSwitchState[hallState] & pwmPinOff[hallState]

#endif /* INC_BLDC_H_ */
