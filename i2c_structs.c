/*
 * i2c_structs.c
 *
 *  Created on: Sep 7, 2014
 *      Author: Dan
 *      Contents: Ring Buffers for handling i2c data streams
 */
/*

#define i2c_BUFFER_SIZE 512

//*****************************************************************************
//
// Receive buffer
//
//*****************************************************************************


unsigned char g_pci2cRxBuffer[USB_BUFFER_SIZE];
unsigned char g_pi2cRxBufferWorkspace[USB_BUFFER_WORKSPACE_SIZE];
const tUSBRingBufObject g_i2cRxBuffer =
{
    i2c_BUFFER_SIZE,                          // This is a receive buffer.
    i2c_RxHandler,                  // pfnCallback
    (void *)&g_sCDCDevice,          // Callback data is our device pointer.
    USBDCDCPacketRead,              // pfnTransfer
    USBDCDCRxPacketAvailable,       // pfnAvailable
    (void *)&g_sCDCDevice,          // pvHandle
    g_pcUSBRxBuffer,                // pcBuffer
    i2c_BUFFER_SIZE,               // ulBufferSize
    g_pucRxBufferWorkspace          // pvWorkspace
};

//*****************************************************************************
//
// Transmit buffer
//
//*****************************************************************************
unsigned char g_pcUSBTxBuffer[USB_BUFFER_SIZE];
unsigned char g_pucTxBufferWorkspace[USB_BUFFER_WORKSPACE_SIZE];
const tUSBBuffer g_sTxBuffer =
{
    true,                           // This is a transmit buffer.
    USB_TxHandler,                      // pfnCallback
    (void *)&g_sCDCDevice,          // Callback data is our device pointer.
    USBDCDCPacketWrite,             // pfnTransfer
    USBDCDCTxPacketAvailable,       // pfnAvailable
    (void *)&g_sCDCDevice,          // pvHandle
    g_pcUSBTxBuffer,                // pcBuffer
    USB_BUFFER_SIZE,               // ulBufferSize
    g_pucTxBufferWorkspace          // pvWorkspace
};
*/
