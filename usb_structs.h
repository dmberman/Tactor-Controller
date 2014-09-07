/*
 * usb_structs.h
 *
 *  Created on: Mar 1, 2014
 *      Author: Dan
 */

#ifndef USB_STRUCTS_H_
#define USB_STRUCTS_H_



extern unsigned long USB_RxHandler(void *pvCBData, unsigned long ulEvent,
                               unsigned long ulMsgValue, void *pvMsgData);
extern unsigned long USB_TxHandler(void *pvlCBData, unsigned long ulEvent,
                               unsigned long ulMsgValue, void *pvMsgData);

extern const tUSBBuffer g_sTxBuffer;
extern const tUSBBuffer g_sRxBuffer;
extern const tUSBDCDCDevice g_sCDCDevice;
extern unsigned char g_pucUSBTxBuffer[];
extern unsigned char g_pucUSBRxBuffer[];


#endif /* USB_STRUCTS_H_ */
