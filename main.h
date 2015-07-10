/*
 * main.h
 *
 *  Created on: Mar 3, 2014
 *      Author: Dan
 */

#ifndef MAIN_H_
#define MAIN_H_

//DEBUG MODES
//General Debug Mode
#ifndef __DEBUG__
#define __DEBUG__
#endif

//Include traceback info in debug line (file, line #)
/*
#ifndef __DEBUG_TRACEBACK__
#define __DEBUG_TRACEBACK__
#endif
*/

//Print heartbeat message to debug
/*
#ifndef __DEBUG_HEARTBEAT__
#define __DEBUG_HEARTBEAT__
#endif
*/

//Printf sine_timer heartbeat to debug
/*
#ifndef __DEBUG_SINE_HEARTBEAT__
#define __DEBUG_SINE_HEARTBEAT__
#endif
*/

//Verbose usb debug mode
/*
#ifndef __DEBUG_USB_EVERYCHAR__
#define __DEBUG_USB_EVERYCHAR__
#endif
*/

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



#define PI 3.1415926
#define PI_2 6.28318

#define USB_SEND_SUCCESS 0x01
#define USB_SEND_FAIL 0x00
#define GPIO_INVALID_BASE 0x02
#define GPIO_INVALID_PIN 0x03


#define PCA_9685_DEFAULT_FREQ 200


//PORTING
#define LED_BASE GPIO_PORTF_BASE
#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

#define I2C_BASE I2C3_MASTER_BASE

//*****************************************************************************
//
// Function Prototypes
//
//*****************************************************************************

void parseUSB(unsigned char *func_name,unsigned long params[],unsigned long nparams);
unsigned long USBSendPacket(unsigned char data[], unsigned char length);
void flashLED(unsigned long color);


#endif /* MAIN_H_ */
