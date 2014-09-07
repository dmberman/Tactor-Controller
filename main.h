/*
 * main.h
 *
 *  Created on: Mar 3, 2014
 *      Author: Dan
 */

#ifndef MAIN_H_
#define MAIN_H_


#include "I2C_Stellaris_API.h"

#include "str2num.h"
#include "proto_defs.h"
#include "PCA9685.h"

#include <stdbool.h>
#include <math.h>

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#define SINE_TIMER_BASE TIMER0_BASE
#define SINE_TIMER_SIDE TIMER_A
#define SINE_TIMER_INT TIMER_TIMA_TIMEOUT
#define SINE_TIMER_PERHIP SYSCTL_PERIPH_TIMER0
#define SINE_TIMER_IE INT_TIMER0A

#define PI 3.1415926
#define PI_2 6.28318

#define USB_SEND_SUCCESS 0x01
#define USB_SEND_FAIL 0x00
#define GPIO_INVALID_BASE 0x02
#define GPIO_INVALID_PIN 0x03


#define PCA_9685_DEFAULT_FREQ 200


//PORTING



#define I2C_BASE I2C3_MASTER_BASE

//*****************************************************************************
//
// USB Parsing Functions and Definitions
//
//*****************************************************************************

void parseUSB(unsigned char *func_name,unsigned char *params[],unsigned long nparams);
unsigned long USBSendPacket(unsigned char data[], unsigned char length);
void initSineTimer(unsigned long sineFreq);
void flashLED(unsigned long color);


#endif /* MAIN_H_ */
