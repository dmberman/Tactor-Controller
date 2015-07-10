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

unsigned char g_ucReadMode = WAITING;
unsigned long g_ulCharIndex = 0;
unsigned long g_ulParamIndex = 0;
unsigned long g_ulReceiveMode = REC_MODE_NORMAL;
unsigned char *g_strFuncName;
unsigned long g_ulParams[N_PARAMS];
unsigned char g_strAssembler[MAX_STR_LEN];

void (*g_ptrStreamFcn)(unsigned char); //Pointer to stream function

void USB_resetPacketAssembler(void){
	g_ucReadMode = WAITING;
	g_ulCharIndex = 0;
	g_ulParamIndex = 0;
	g_ulReceiveMode = REC_MODE_NORMAL;

#ifdef __DEBUG__
		UARTprintf("\nTx Buffer Space Available = %d",USBBufferSpaceAvailable((tUSBBuffer *)&g_sTxBuffer));
		UARTprintf("\nRx Buffer Space Available = %d",USBBufferSpaceAvailable((tUSBBuffer *)&g_sRxBuffer));
		UARTprintf("\nCLEARING USB BUFFERS...");
#endif

	USBBufferFlush((tUSBBuffer *)&g_sTxBuffer);
	USBBufferFlush((tUSBBuffer *)&g_sRxBuffer);

#ifdef __DEBUG__
	UARTprintf("\nTx Buffer Space Available = %d",USBBufferSpaceAvailable((tUSBBuffer *)&g_sTxBuffer));
	UARTprintf("\nRx Buffer Space Available = %d",USBBufferSpaceAvailable((tUSBBuffer *)&g_sRxBuffer));
#endif

}

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
#ifdef __DEBUG__
	UARTprintf("\nUSB Connected");
#ifdef __DEBUG_TRACEBACK__
			UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
#endif
#endif
            break;

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
            g_bUSBConfigured = false;
#ifdef __DEBUG__
	UARTprintf("\nUSB Disconnected");
#ifdef __DEBUG_TRACEBACK__
			UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
#endif
#endif
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
			#ifdef __DEBUG__
				UARTprintf("\nUSB Tx Complete");
			#ifdef __DEBUG_TRACEBACK__
						UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
			#endif
			#endif
            break;

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
        	break;
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
				#ifdef __DEBUG__
					UARTprintf("\nUSB Stream Mode Rx Event");
				#ifdef __DEBUG_TRACEBACK__
							UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
				#endif
				#endif
        	}

        	else{
        		//allocate packet array
        		ucBytesRead = USBBufferRead((tUSBBuffer *)&g_sRxBuffer, ucCharArr, USB_RX_LENGTH-1);
        		ucCharArr[ucBytesRead] = NULL; //Null terminate string
				#ifdef __DEBUG__
					UARTprintf("\nUSB Buffer Dump (length = %d):\n%s\n",ucBytesRead, ucCharArr);
				#endif
				for(ulCount=0;ulCount<ucBytesRead;ulCount++){
					nextByte = ucCharArr[ulCount];
					#ifdef __DEBUG__
					#ifdef __DEBUG_USB_EVERYCHAR__
						UARTprintf("\nUSB nextByte = %c, ",nextByte);
					#ifdef __DEBUG_TRACEBACK__
						UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
					#endif
					#endif
					#endif

					//Ignore non-printing and whitespace characters
					if(nextByte <= ' '){
						#ifdef __DEBUG__
							UARTprintf("\nUSB Ignored Character = %c, ",nextByte);
						#ifdef __DEBUG_TRACEBACK__
							UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
						#endif
						#endif
						continue;
					}


					if(nextByte == '#'){
						if(g_ucReadMode == COMMENT){
							g_ucReadMode = WAITING;
							g_ulCharIndex = 0;
							g_ulParamIndex = 0;
							#ifdef __DEBUG__
								UARTprintf("\ng_ucReadMode = FUNCTION");
								#ifdef __DEBUG_TRACEBACK__
									UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
								#endif
							#endif
							continue;
						}
						else{
							g_ucReadMode = COMMENT;
							#ifdef __DEBUG__
								UARTprintf("\ng_ucReadMode = COMMENT");
								#ifdef __DEBUG_TRACEBACK__
									UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
								#endif
							#endif
							continue;
						}
					}

					switch(g_ucReadMode){
					case WAITING:
							if(nextByte < 'A' || nextByte > 'Z'){
								continue;
							}
							else{
								g_ucReadMode = FUNCTION;
							}
							//DON'T PUT A BREAK HERE!
					case FUNCTION:
						if(nextByte == '('){ //end of Function section
							g_strAssembler[g_ulCharIndex] = NULL; //Terminate string
							g_strFuncName = malloc(strlen((char *)g_strAssembler) + 1); //allocate memory for function name
							strcpy((char *)g_strFuncName,(char *)g_strAssembler); //Copy function name
							g_ulCharIndex = 0;
							g_ucReadMode = PARAMETER;
							#ifdef __DEBUG__
							#ifdef __DEBUG_USB_EVERYCHAR__
								UARTprintf("\ng_ucReadMode = PARAMETER");
								#ifdef __DEBUG_TRACEBACK__
									UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
								#endif
							#endif
							#endif
							continue;
						}
						else{
							g_strAssembler[g_ulCharIndex] = nextByte;
							g_ulCharIndex ++;
							if(g_ulCharIndex > MAX_FUNC_LENGTH){ //Error Checking
								g_ucReadMode = WAITING;
								g_ulCharIndex = 0;
								g_ulParamIndex = 0;
								#ifdef __DEBUG__
									UARTprintf("\nFUNCTION LENGTH EXCEEDED, g_ucReadMode = WAITING");
									#ifdef __DEBUG_TRACEBACK__
										UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
									#endif
								#endif
								continue;
							}
						}
						break;
					case PARAMETER:
						if(nextByte == ')'){ //end of input
							g_strAssembler[g_ulCharIndex] = NULL;
							g_ulParams[g_ulParamIndex] = str2ul(g_strAssembler);

							if(g_ulParamIndex > 0){
								g_ulParamIndex ++;
							}

							parseUSB(g_strFuncName,g_ulParams,g_ulParamIndex);
							g_ucReadMode = WAITING;
							g_ulCharIndex = 0;
							g_ulParamIndex = 0;
							free(g_strFuncName);
							#ifdef __DEBUG__
							#ifdef __DEBUG_USB_EVERYCHAR__
								UARTprintf("\ng_ucReadMode = WAITING");
								#ifdef __DEBUG_TRACEBACK__
									UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
								#endif
							#endif
							#endif
							continue;
						}
						else{
							if(nextByte == ' ' || nextByte == ','){ //next parameter
							g_strAssembler[g_ulCharIndex] = NULL;
							g_ulParams[g_ulParamIndex] = str2ul(g_strAssembler);
							g_ulParamIndex ++;
							g_ulCharIndex = 0;
							continue;
							}
							else{
								g_strAssembler[g_ulCharIndex] = nextByte;
								g_ulCharIndex ++;
								if(g_ulCharIndex > MAX_PARAM_LENGTH){ //Error Checking
									g_ucReadMode = WAITING;
									g_ulCharIndex = 0;
									g_ulParamIndex = 0;
									#ifdef __DEBUG__
									#ifdef __DEBUG_USB_EVERYCHAR__
										UARTprintf("\nPARAMEtER LENGTH EXCEEDED, g_ucReadMode = WAITING");
										#ifdef __DEBUG_TRACEBACK__
											UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
										#endif
									#endif
									#endif
									continue;
								}
							}
						}

						break;
					default:
						break;
					}
				}
			}
        }
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
			#ifdef __DEBUG__
				UARTprintf("\nUSB_EVENT_DATA_REMAINING");
				#ifdef __DEBUG_TRACEBACK__
					UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
				#endif
			#endif
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
			#ifdef __DEBUG__
				UARTprintf("\nUSB_EVENT_REQUEST_BUFFER");
				#ifdef __DEBUG_TRACEBACK__
					UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
				#endif
			#endif
            return(0);
        }

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
        	return(0);
    }
}


void USBSetReceiveMode(unsigned long mode){
	g_ulReceiveMode = mode;
}
