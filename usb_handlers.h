/*
 * usb_handlers.h
 *
 *  Created on: Mar 1, 2014
 *      Author: Dan
 */

#ifndef USB_HANDLERS_H_
#define USB_HANDLERS_H_

//State Variables
#define FUNCTION 0
#define PARAMETER 1
#define COMMENT 2


#define USB_RX_LENGTH 50

#define REC_MODE_NORMAL 0
#define REC_MODE_STREAM 1

extern  volatile unsigned long g_ulUSBTxCount;
extern  volatile unsigned long g_ulUSBRxCount;
extern  volatile tBoolean g_bUSBConfigured;

void USBSetReceiveMode(unsigned long mode);

unsigned long
ControlHandler(void *pvCBData, unsigned long ulEvent,
               unsigned long ulMsgValue, void *pvMsgData);



unsigned long
TxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue,
          void *pvMsgData);


unsigned long
RxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue,
          void *pvMsgData);


#endif /* USB_HANDLERS_H_ */
