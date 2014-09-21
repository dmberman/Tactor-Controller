/*
 * usb_handlers.c
 *
 *  Created on: Mar 1, 2014
 *      Author: Dan
 */

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_uart.h"
#include "inc/hw_sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/usb.h"
#include "driverlib/rom.h"
#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "usb_structs.h"
#include "usb_handlers.h"
#include "main.h"
#include "String.h"

#include "proto_defs.h"

#include "stdio.h"
#include "stdlib.h"

//*****************************************************************************
//
// Handles CDC driver notifications related to control and setup of the device.
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to perform control-related
// operations on behalf of the USB host.  These functions include setting
// and querying the serial communication parameters, setting handshake line
// states and sending break conditions.
//
// \return The return value is event-specific.
//
//*****************************************************************************

//*****************************************************************************
//
// Global variables
//
//*****************************************************************************

unsigned char g_ucReadMode = FUNCTION;
unsigned long g_ulCharIndex = 0;
unsigned long g_ulParamIndex = 0;
unsigned long g_ulReceiveMode = REC_MODE_NORMAL;
unsigned char *g_strFuncName;
unsigned char *g_strParams[N_PARAMS];
unsigned char g_strAssembler[MAX_STR_LEN];

void (*g_ptrStreamFcn)(unsigned char); //Pointer to stream function

void GetLineCoding(tLineCoding *psLineCoding)
{
	psLineCoding->ulRate = 9600;
	psLineCoding->ucDatabits = 8;
	psLineCoding->ucParity = USB_CDC_PARITY_NONE;
	psLineCoding->ucStop = USB_CDC_STOP_BITS_1;
}


unsigned long
USB_ControlHandler(void *pvCBData, unsigned long ulEvent,
               unsigned long ulMsgValue, void *pvMsgData)
{


    //
    // Which event are we being asked to process?
    //
    switch(ulEvent)
    {
        //
        // We are connected to a host and communication is now possible.
        //
        case USB_EVENT_CONNECTED:
            g_bUSBConfigured = true;

            // Flush our buffers.
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);

            UARTprintf("USB Connected\n");
            break;

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
            g_bUSBConfigured = false;
            UARTprintf("USB Disconnected\n");
            break;

        //
        // Return the current serial communication parameters.
        //
        case USBD_CDC_EVENT_GET_LINE_CODING:
        	GetLineCoding(pvMsgData);
        	break;
        case USBD_CDC_EVENT_SET_LINE_CODING:
        	break;
        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
        	break;
        case USBD_CDC_EVENT_SEND_BREAK:
        	break;
        case USBD_CDC_EVENT_CLEAR_BREAK:
        	break;
        case USB_EVENT_SUSPEND:
        	break;
        case USB_EVENT_RESUME:
        	break;
        default:
#ifdef DEBUG
#else
            break;
#endif

    }

    return(0);
}



//*****************************************************************************
//
// Handles CDC driver notifications related to the transmit channel (data to
// the USB host).
//
// \param ulCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
unsigned long
USB_TxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue,
          void *pvMsgData)
{
    //
    // Which event have we been sent?
    //
    switch(ulEvent)
    {
        case USB_EVENT_TX_COMPLETE:
            //
            // Increment Packet Counter
        	g_ulUSBTxCount ++;
            break;

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif

    }
    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the receive channel (data from
// the USB host).
//
// \param ulCBData is the client-supplied callback data value for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
unsigned long
USB_RxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue,
          void *pvMsgData)
{
    unsigned long ulCount;
    unsigned char ucCharArr[USB_RX_LENGTH];
    unsigned char ucBytesRead;
    unsigned char nextByte;
    //
    // Which event are we being sent?
    //
    switch(ulEvent)
    {
        //
        // A new packet has been received.
        //
        case USB_EVENT_RX_AVAILABLE:
        {
        	//Increment Packet Counter
        	g_ulUSBRxCount++;

        	if(g_ulReceiveMode == REC_MODE_STREAM){
        		//Buffer reading handled in stream interrupt
        	}

        	else{
        		//allocate packet array
        		ucBytesRead = USBBufferRead((tUSBBuffer *)&g_sRxBuffer, ucCharArr, ulMsgValue);
				for(ulCount=0;ulCount<ucBytesRead;ulCount++){
					nextByte = ucCharArr[ulCount];

/*					//Ignore non-printing and whitespace characters
					if(nextByte <= ' '){
						break;
					}

					//Ignore comments
					if(nextByte == '#'){
						g_ucReadMode = COMMENT;
						break;
					}

					if(g_ucReadMode == COMMENT){
						if(nextByte == '#'){
							g_ucReadMode = FUNCTION;
							g_ulCharIndex = 0;
							g_ulParamIndex = 0;
						}
						break;
					}
*/
					if(nextByte == ')'){ //End of input
						g_strAssembler[g_ulCharIndex] = NULL;
						g_strParams[g_ulParamIndex] = malloc(strlen((char *)g_strAssembler) + 1); //allocate memory for parameter value
						strcpy((char *)g_strParams[g_ulParamIndex],(char *)g_strAssembler);

						if(g_ulParamIndex > 0){
							g_ulParamIndex ++;
						}

						//Echo Command
					    /*
						ucCharArr[0] = '\n';
						USBBufferWrite((tUSBBuffer *)g_sTxBuffer,(*c'\n',1);
						USBBufferWrite((tUSBBuffer *)g_sTxBuffer,g_strFuncName,strlen((char *)g_strFuncName));
						ucCharArr[0] = "(";
						USBBufferWrite((tUSBBuffer *)g_sTxBuffer,ucCharArr,1);
						for(ulCount2=0;ulCount2<g_ulParamIndex+1;ulCount2++){
							USBBufferWrite((tUSBBuffer *)g_sTxBuffer,g_strParams[ulCount2],strlen((char *)g_strParams[ulCount2]));
							ucCharArr[0] = ",";
							USBBufferWrite((tUSBBuffer *)g_sTxBuffer,ucCharArr,1);
						}
						ucCharArr = ")\n";
						USBBufferWrite((tUSBBuffer *)g_sTxBuffer,ucCharArr,2);
*/
						parseUSB(g_strFuncName,g_strParams,g_ulParamIndex);
						g_ucReadMode = FUNCTION;
						g_ulCharIndex = 0;
						g_ulParamIndex = 0;
						break;
					}
					else{ //not end of input
						if(g_ucReadMode == FUNCTION){ //If currently reading function
							if(nextByte == '('){
								g_strAssembler[g_ulCharIndex] = NULL; //Terminate string
								g_strFuncName = malloc(strlen((char *)g_strAssembler) + 1); //allocate memory for function name
								strcpy((char *)g_strFuncName,(char *)g_strAssembler); //Copy function name
								g_ulCharIndex = 0;
								g_ucReadMode = PARAMETER;
							}
							else{
								g_strAssembler[g_ulCharIndex] = nextByte;
								g_ulCharIndex ++;
								if(g_ulCharIndex > MAX_FUNC_LENGTH){ //Error Checking
									g_ucReadMode = FUNCTION;
									g_ulCharIndex = 0;
									g_ulParamIndex = 0;
									break;
								}
							}
						}
						else{ 						  //Currently reading parameters
							if(nextByte == ' ' || nextByte == ','){ //next parameter
								g_strAssembler[g_ulCharIndex] = NULL;
								g_strParams[g_ulParamIndex] = malloc(strlen((char *)g_strAssembler) + 1); //allocate memory for parameter value
								strcpy((char *)g_strParams[g_ulParamIndex],(char *)g_strAssembler);
								g_ulParamIndex ++;
								g_ulCharIndex = 0;
							}
							else{
								g_strAssembler[g_ulCharIndex] = nextByte;
								g_ulCharIndex ++;
								if(g_ulCharIndex > MAX_PARAM_LENGTH){ //Error Checking
									g_ucReadMode = FUNCTION;
									g_ulCharIndex = 0;
									g_ulParamIndex = 0;
									break;
								}
							}
						}
					}
				}
            break;
        	}

        }
        //
        // We are being asked how much unprocessed data we have still to
        // process. We return 0 if the UART is currently idle or 1 if it is
        // in the process of transmitting something. The actual number of
        // bytes in the UART FIFO is not important here, merely whether or
        // not everything previously sent to us has been transmitted.
        //
        case USB_EVENT_DATA_REMAINING:
        {
            //
            // For now just return 0
            //
        	ulCount = 0;
            return(ulCount);
        }

        //
        // We are being asked to provide a buffer into which the next packet
        // can be read. We do not support this mode of receiving data so let
        // the driver know by returning 0. The CDC driver should not be sending
        // this message but this is included just for illustration and
        // completeness.
        //
        case USB_EVENT_REQUEST_BUFFER:
        {
            return(0);
        }

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
        	return(0);
    }

    return(0);
}


void USBSetReceiveMode(unsigned long mode){
	g_ulReceiveMode = mode;
}
