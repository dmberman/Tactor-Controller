/*
 * PCA9685.h
 *
 *  Created on: Jul 15, 2014
 *      Author: Dan
 */

#ifndef PCA9685_H_
#define PCA9685_H_

#define PCA9685_DEFAULT_FREQUENCY 200

int PCA9685_init(unsigned long freq);
int PCA9685_setDutyCycle(unsigned long channel, unsigned long duty);
int PCA9685_reset(void);


unsigned long PCA9685_writeREG(unsigned char ucReg, unsigned char ucValue);
unsigned long PCA9685_writeDATA(unsigned char ucReg, char* cSendData, unsigned int uiSize);
unsigned long PCA9685_readREG(unsigned char ucReg);
unsigned long PCA9685_readDATA(unsigned char ucReg, char* cReadData, unsigned int uiSize);



#endif /* PCA9685_H_ */
