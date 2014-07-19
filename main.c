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
#include "I2C_Stellaris_API.h"

#include "str2num.h"
#include "proto_defs.h"

#include <stdbool.h>
#include <math.h>

//Porting:
#define LED_BASE GPIO_PORTF_BASE
#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

//*****************************************************************************
//
// The system tick rate expressed both as ticks per second and a millisecond
// period.
//
//*****************************************************************************
#define SYSTICKS_PER_SECOND 80
#define SYSTICK_PERIOD_MS (1000 / SYSTICKS_PER_SECOND)


//*****************************************************************************
//
// Global system tick counter
//
//*****************************************************************************
volatile unsigned long g_ulSysTickCount = 0;

//*****************************************************************************
//
// Global USB packet counters
//
//*****************************************************************************
volatile unsigned long g_ulUSBTxCount = 0;
volatile unsigned long g_ulUSBRxCount = 0;

//*****************************************************************************
//
// Global flag indicating that a USB configuration has been set.
//
//*****************************************************************************
volatile tBoolean g_bUSBConfigured = false;

//*****************************************************************************
//
// Global Sine variables
//
//*****************************************************************************

volatile unsigned long g_ulSineMode = PWM_MODE;
volatile unsigned long g_ulSineFreq[N_SINE_CMPS];
volatile unsigned long g_ulSineAmplitude[N_SINE_CMPS];
volatile double g_dblSinePhase[N_SINE_CMPS];
volatile double g_dblSineSamplePeriod;
volatile double g_dblSineTime = 0;

//*****************************************************************************
//
// Global PWM variables
//
//*****************************************************************************
volatile unsigned long g_ulPWMPeriod;



//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************

#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
    while(1)
    {
    }
}
#endif


volatile unsigned long dutyCycle;

void sineTimerInit(unsigned long sineFreq){
    //TIMER INT OUTPUT
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_7);
    ROM_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_7,0x00); //Set pin low

    g_dblSineSamplePeriod = 1/(double)sineFreq; //Sample period

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	ROM_TimerDisable(TIMER1_BASE,TIMER_A);
    ROM_TimerConfigure(TIMER1_BASE,TIMER_CFG_A_PERIODIC | TIMER_CFG_SPLIT_PAIR);
    ROM_TimerIntClear(TIMER1_BASE,TIMER_TIMA_TIMEOUT);
    ROM_TimerLoadSet(TIMER1_BASE,TIMER_A,(ROM_SysCtlClockGet()/sineFreq)/2);
    ROM_TimerIntEnable(TIMER1_BASE,TIMER_TIMA_TIMEOUT);
    ROM_IntEnable(INT_TIMER1A);
    ROM_TimerEnable(TIMER1_BASE,TIMER_A);
}

void sineTimerStop(void){
    ROM_TimerDisable(TIMER1_BASE,TIMER_A);
}

int initUSB(void){
    // Initialize USB

    // Not configured initially.
    g_bUSBConfigured = false;

    // Configure the required pins for USB operation.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_5 | GPIO_PIN_4);

	// Initialize the transmit and receive buffers.
    USBSetReceiveMode(REC_MODE_NORMAL);
	USBBufferInit((tUSBBuffer *)&g_sTxBuffer);
	USBBufferInit((tUSBBuffer *)&g_sRxBuffer);

	// Set the USB stack mode to Device mode with VBUS monitoring.
	USBStackModeSet(0, USB_MODE_DEVICE, 0);

	// Pass our device information to the USB library and place the device
	// on the bus.
	USBDCDCInit(0, (tUSBDCDCDevice *)&g_sCDCDevice);
	return 0;
}

int initLED(void){
	// Initialize LED
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	ROM_GPIOPinTypeGPIOOutput(LED_BASE, LED_RED|LED_GREEN|LED_BLUE);
	ROM_GPIOPinWrite(LED_BASE, LED_RED|LED_GREEN|LED_BLUE, 0x00); //Turn off LED
	return 0;
}

int initSW(void){
    // Initialize Button
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

    ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);
    ROM_GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
    return 0;
}

int initUART(void){
    // Initialize the UART
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioInit(0);
	return 0;
}

int initSysTick(void){
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / SYSTICKS_PER_SECOND);
    ROM_SysTickIntEnable();
    ROM_SysTickEnable();
    ROM_IntMasterEnable();
}

int initI2C(void){
	I2CSetup(I2C3_MASTER_BASE,true);
}

int init(void){
	    ROM_FPULazyStackingEnable();
	    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	    ROM_SysCtlDelay(10000);

	    initLED();
	    initSW();
	    initUSB();

	    // Enable the system tick. (used for RTP)
		return 0;
}

int main(void) {
    unsigned long ulTxCount;
    unsigned long ulRxCount;
    unsigned long ulSysTickCount;
    unsigned long oldLEDState;
    unsigned long ulLoop;

    ulTxCount = 0;
    ulRxCount = 0;


	init();
	UARTprintf("Initialized\n");

	for(ulLoop = 0; ulLoop < 30000; ulLoop++);
	while(1){

		for(ulLoop = 0; ulLoop < 1000000; ulLoop++);

		if(ulTxCount != g_ulUSBTxCount){ //data has been sent
				oldLEDState = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x0E); //Turn on WHITE LED
				for(ulLoop = 0; ulLoop < 750000; ulLoop++);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, oldLEDState); //Return to previous LED state
				ulTxCount = g_ulUSBTxCount;
		}

		if(ulRxCount != g_ulUSBRxCount){ //data has been received
				oldLEDState = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x0C); //Turn on YELLOW LED
				for(ulLoop = 0; ulLoop < 750000; ulLoop++);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, oldLEDState); //Return to previous LED state
				ulRxCount = g_ulUSBRxCount;
		}

		if(ulSysTickCount != g_ulSysTickCount){ //flash heartbeat
				oldLEDState = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
				if(g_bUSBConfigured)
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x08); //Turn on GREEN LED
				else
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x02); //Turn on RED LED
				for(ulLoop = 0; ulLoop < 100000; ulLoop++);
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, oldLEDState); //Return to previous LED state
				ulSysTickCount = g_ulSysTickCount;
		}

	}
}


unsigned char USBSendPacket(unsigned char data[], unsigned char length){
	unsigned char packet[10];
	unsigned char bytesWritten;
	unsigned char status;
	unsigned long ulCount;
	unsigned long temp;

	packet[0] = START_OF_PACKET;
	for(ulCount = 0;ulCount<length;ulCount++){
		temp=ulCount+1;
		packet[temp] = data[ulCount];
	}
	ulCount = ulCount + 1;
	packet[ulCount] = END_OF_PACKET;

	length = length + 2;

	bytesWritten = USBBufferWrite((tUSBBuffer *)&g_sTxBuffer,packet, length);
	if(bytesWritten == length){
			status = STATUS_SUCCESS;
	}
	else{
			status = STATUS_FAIL;
	}
	return status;
}

unsigned long GPIOPortNumber(unsigned char port){
	unsigned long ulPort;
	switch(port){
	case 0:
		ulPort = GPIO_PORTA_BASE;
		break;
	case 1:
		ulPort = GPIO_PORTB_BASE;
		break;
	case 2:
		ulPort = GPIO_PORTC_BASE;
		break;
	case 3:
		ulPort = GPIO_PORTD_BASE;
		break;
	case 4:
		ulPort = GPIO_PORTE_BASE;
		break;
	case 5:
		ulPort = GPIO_PORTF_BASE;
		break;
	default:
		ulPort = GPIO_INVALID_BASE;
	}
	return ulPort;
}

unsigned long GPIOPinNumber(unsigned char port){
	unsigned long ulPort;
	switch(port){
	case 0:
		ulPort = GPIO_PIN_0;
		break;
	case 1:
		ulPort = GPIO_PIN_1;
		break;
	case 2:
		ulPort = GPIO_PIN_2;
		break;
	case 3:
		ulPort = GPIO_PIN_3;
		break;
	case 4:
		ulPort = GPIO_PIN_4;
		break;
	case 5:
		ulPort = GPIO_PIN_5;
		break;
	case 6:
		ulPort = GPIO_PIN_6;
		break;
	case 7:
		ulPort = GPIO_PIN_7;
		break;
	default:
		ulPort = GPIO_INVALID_PIN;
	}
	return ulPort;
}


void parseUSB(unsigned char *func_name,unsigned char *params[],unsigned long nparams){
	unsigned char status;
	unsigned long ulFuncNum;

	ulFuncNum = str2ul(func_name);

	switch(ulFuncNum){
		case DRV2605_WRITE_REG:
			DRV2605_setReg(str2ul(params[0]),str2ul(params[1]));
			break;
//		case DRV2605_READ_REG:
//			DRV2605_setReg((unsigned char)params[0], (unsigned char)params[1]);
//			break;
		case DRV2605_SET_MUX:
			DRV2605_setMUX(str2ul(params[0]));
			break;
		case DRV2605_ENABLE:
			DRV2605_enable(str2ul(params[0]));
			break;
		case SINE_INIT:
			sineTimerInit(str2ul(params[0]));
			break;
		case SINE_STOP:
			sineTimerStop();
			pwmSetDuty(0);
			break;
		case SINE_SET_AMPLITUDE:
			g_ulSineAmplitude[str2ul(params[0])] = str2ul(params[1]);
			break;
		case SINE_SET_PHASE:
			g_dblSinePhase[str2ul(params[0])] = (str2ul(params[1])/1000);
			break;
		case SINE_SET_FREQ:
			g_ulSineFreq[str2ul(params[0])] = str2ul(params[1]);
			break;
		case SINE_SET_MODE:
			if(str2ul(params[0]) == 0){
				g_ulSineMode = PWM_MODE;
			}
			else{
				g_ulSineMode = RTP_MODE;
			}
			break;
		case PWM_INIT:
			pwmInit(str2ul(params[0]));
			break;
		case PWM_SET_DUTY:
			pwmSetDuty(str2ul(params[0]));
			break;
		default:
			status = STATUS_FAIL;
	}
}


//*****************************************************************************
//
// Interrupt handler for the system tick counter.
//
//*****************************************************************************
void
SysTickIntHandler(void){
}

void SineTimerIntHandler(void){
}

void I2CIntHandler(void){
}
