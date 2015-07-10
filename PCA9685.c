/*
 * PCA9685.c
 *
 *  Created on: Jul 15, 2014
 *      Author: Dan
 */
#include "PCA9685.h"
#include "I2C_Stellaris_API.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_uart.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_i2c.h"

#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/i2c.h"
#include "driverlib/usb.h"
#include "driverlib/rom.h"
#include "driverlib/uart.h"
#include "driverlib/pwm.h"


#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "usb_handlers.h"
#include "usb_structs.h"

#include "main.h"

//I2C Addresses
#define PCA9685_ADDRESS 0x7C
#define PCA9685_RESET 0x06

//Register Addresses
#define PCA9685_CNTRL_REG_BASE 0x06
#define PCA9685_PRESCALE_REG 0xFE
#define PCA9685_MODE1_REG 0x00
#define PCA9685_MODE2_REG 0x01

//Config Register Values
#define PCA9685_MODE1_VAL 0x80
#define PCA9685_MODE2_VAL 0x04

const long PCA9685_DELAY[16] = {1,1,767,1023,1279,1535,1791,2047,2303,2559,2815,3071,3327,3583,3839,4095};

//Communication Functions
unsigned long PCA9685_writeREG(unsigned char ucReg, unsigned char ucValue){
	unsigned long result;
	result = I2CRegWrite(I2C_BASE, PCA9685_ADDRESS, ucReg, ucValue);
	return result;
}

unsigned long PCA9685_writeDATA(unsigned char ucReg, char* cSendData, unsigned int uiSize){
	unsigned long result;
	result = I2CWriteData(I2C_BASE,PCA9685_ADDRESS,ucReg,cSendData,uiSize);
	return result;
}

unsigned long PCA9685_readREG(unsigned char ucReg){
	unsigned long result;
	result = I2CRegRead(I2C_BASE,PCA9685_ADDRESS,ucReg);
	return result;
}

unsigned long PCA9685_readDATA(unsigned char ucReg, char* cReadData, unsigned int uiSize){
	unsigned long result;
	result = I2CReadData(I2C_BASE, PCA9685_ADDRESS,ucReg,cReadData,uiSize);
	return result;
}

int PCA9685_reset(void){
	PCA9685_writeREG(PCA9685_MODE1_REG,0x00);
	return 0;
}

int PCA9685_init(unsigned long freq){
	unsigned long prescaleValue = (25000000/(4096*freq))-1;
	if(prescaleValue > 255){
		prescaleValue = 255;
	}
	else{
		if(prescaleValue < 3){
			prescaleValue = 3;
		}
	}

	PCA9685_writeREG(PCA9685_MODE1_REG,0x01); //put device to sleep
	PCA9685_writeREG(PCA9685_PRESCALE_REG,(unsigned char)prescaleValue); //write prescale value
	PCA9685_writeREG(PCA9685_MODE1_REG,PCA9685_MODE2_VAL); //put device to normal mode
	PCA9685_writeREG(PCA9685_MODE2_REG,PCA9685_MODE2_VAL);
	return 0;
}

int PCA9685_setDutyCycle(unsigned long channel, unsigned long duty){
	//Register addresses
	if(duty > 4095){
		duty = 4095;
	}
	if(duty < 1){
		duty = 1;
	}
	unsigned char OnRegLow = PCA9685_CNTRL_REG_BASE + (4*channel);
	unsigned char OnRegHigh = OnRegLow + 1;
	unsigned char OffRegLow = OnRegHigh + 1;
	unsigned char OffRegHigh = OffRegLow + 1;

	unsigned int OnTime = PCA9685_DELAY[channel] - 1;
	unsigned int OffTime = OnTime + duty - 1;

	if(OffTime > 4095){
		OffTime = OffTime - 4096;
	}

	PCA9685_writeREG(OnRegLow,(unsigned char)(OnTime & 0xFF));
	PCA9685_writeREG(OnRegHigh,(unsigned char)(OnTime >> 8));
	PCA9685_writeREG(OffRegLow,(unsigned char)(OffTime & 0xFF));
	PCA9685_writeREG(OffRegHigh,(unsigned char)(OffTime >> 8));
	return 0;
}



