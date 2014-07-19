/*
 * main.h
 *
 *  Created on: Mar 3, 2014
 *      Author: Dan
 */

#ifndef MAIN_H_
#define MAIN_H_

#define PI 3.1415926
#define PI_2 6.28318

#define RTP_MODE 50
#define PWM_MODE 0

#define N_SINE_CMPS 3 //number of RTP sine frequencies




//PORTING

#define CHIP_SELECT_BASE
#define CHIP_SELECT_PIN

#define MUX1_BASE
#define MUX1_PIN
#define MUX2_BASE GPIO_PORTA_BASE
#define MUX2_PIN GPIO_PIN_3
#define MUX3_BASE GPIO_PORTA_BASE
#define MUX3_PIN GPIO_PIN_4
#define MUX4_BASE GPIO_PORTA_BASE
#define MUX4_PIN GPIO_PIN_6

#define DRV2605_SLAVE_ADDRESS       0x5A

#define I2C_BASE I2C3_MASTER_BASE

//*****************************************************************************
//
// USB Parsing Functions and Definitions
//
//*****************************************************************************

void parseUSB(unsigned char *func_name,unsigned char *params[],unsigned long nparams);
unsigned char USBSendPacket(unsigned char data[], unsigned char length);



#endif /* MAIN_H_ */
