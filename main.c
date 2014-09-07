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
#include "waveform.h"

#include "str2num.h"
#include "proto_defs.h"
#include "PCA9685.h"

#include <stdbool.h>
#include <math.h>

//Porting:
#define LED_BASE GPIO_PORTF_BASE
#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

//*****************************************************************************
//
//Flash Memory Variables
//
//*****************************************************************************
 volatile unsigned long g_unsavedWaveform = FALSE;

//*****************************************************************************
//
//Sine Timer Variables
//
//*****************************************************************************

volatile unsigned long g_ulSampleFreq = 0;
volatile unsigned long g_sineTimerVal = 0;
volatile unsigned long g_sineTimerMS = 0;
volatile unsigned long g_sineIntCount = 0;
unsigned long g_sysClk = 0;
unsigned long g_timerLoadVal = 0;

//*****************************************************************************
//
// The system tick rate expressed both as ticks per second and a millisecond
// period.
//
//*****************************************************************************
#define SYSTICKS_PER_SECOND 1
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

void sineTimerLoad(unsigned long sampleFreq){
    g_ulSampleFreq = sampleFreq; //Sample period
    g_sysClk = ROM_SysCtlClockGet();
    g_timerLoadVal = g_sysClk/sampleFreq;
    ROM_TimerLoadSet(SINE_TIMER_BASE,SINE_TIMER_SIDE,g_timerLoadVal);
}

void initSineTimer(unsigned long sampleFreq){
	ROM_SysCtlPeripheralEnable(SINE_TIMER_PERHIP);
    flashLED(LED_GREEN);
	ROM_TimerConfigure(SINE_TIMER_BASE,TIMER_CFG_PERIODIC);
	ROM_TimerControlStall(SINE_TIMER_BASE,SINE_TIMER_SIDE,1);
	sineTimerLoad(sampleFreq);
    ROM_TimerIntClear(SINE_TIMER_BASE,SINE_TIMER_INT);
    ROM_TimerEnable(SINE_TIMER_BASE,SINE_TIMER_SIDE);
    ROM_TimerIntEnable(SINE_TIMER_BASE,SINE_TIMER_INT);
    ROM_IntEnable(SINE_TIMER_IE);
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
    return 0;
}

int initI2C(void){
	I2CSetup(I2C_BASE,true);
	return 0;
}

int init(void){
	    ROM_FPULazyStackingEnable();
	    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	    ROM_SysCtlDelay(10000);

	    initLED();
	    initSW();
	    initUSB();

	    initI2C();

	    PCA9685_init(PCA9685_DEFAULT_FREQUENCY);

	    initSysTick();
	    initUART();
	    initWaveform();
	    initSineTimer(50);
		return 0;
}

void flashLED(unsigned long color){
	unsigned long oldLEDState, ulLoop;
	oldLEDState = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, color); //Turn on LED
	for(ulLoop = 0; ulLoop < 100000; ulLoop++);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, oldLEDState); //Return to previous LED state
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
	while(1){
		if(g_unsavedWaveform == TRUE){
			waveformFlashSave();
		}

		if(ulTxCount != g_ulUSBTxCount){ //data has been sent
				flashLED(0x0E); //Flash White LED
				ulTxCount = g_ulUSBTxCount;
		}

		if(ulRxCount != g_ulUSBRxCount){ //data has been received
				flashLED(0x0C); //Flash Yellow LED
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


unsigned long USBSendPacket(unsigned char data[], unsigned char length){
	unsigned char packet[10];
	unsigned char bytesWritten;
	unsigned long ulCount;
	unsigned long status;

	for(ulCount = 0;ulCount<length;ulCount++){
		packet[ulCount] = data[ulCount];
	};
	packet[ulCount] = END_OF_PACKET;
	length ++;

	bytesWritten = USBBufferWrite((tUSBBuffer *)&g_sTxBuffer,packet, length);
	if(bytesWritten == length){
			status = USB_SEND_SUCCESS;
	}
	else{
			status = USB_SEND_FAIL;
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
	unsigned long i;
	unsigned char packet[Tx_MAX_PACKET_LENGTH];
	unsigned long sinFreq[N_SINE_COMPONENTS], sinAmplitude[N_SINE_COMPONENTS];
	switch(func_name[0]){
		case LED:
			ROM_GPIOPinWrite(LED_BASE,LED_RED, str2ul(params[0]));
			packet[0] = ACK;
			break;
		case MOTOR_RUN:
			 runMotor(str2ul(params[0]), //Channel
					 str2ul(params[1]),  //Waveform
					 str2ul(params[2]),  //Amplitude Stretch
					 str2ul(params[3])); //Frequency Stretch
			packet[0] = ACK;
			break;
		case ALL_RUN:
			for(i=0;i<N_MOTORS;i++){
				runMotor(i, 			   //Channel
						str2ul(params[0]), //Waveform
						str2ul(params[1]), //Amplitude Stretch
						str2ul(params[2])); //Frequency Stretch

			}
		case MOTOR_STOP:
			stopMotor(str2ul(params[0])); //Channel
			packet[0] = ACK;
			break;
		case ALL_STOP:
			for(i=0;i<N_MOTORS;i++)
				stopMotor(i);
			packet[0] = ACK;
			break;
		case COM_CHECK:
			packet[0] = ACK;
			break;
		case RESET:
			PCA9685_reset();
			PCA9685_init(PCA_9685_DEFAULT_FREQ);
			packet[0] = ACK;
			break;
		case SET_WAVEFORM:
			for(i=0;i<N_SINE_COMPONENTS;i++){
				sinFreq[i] = str2ul(params[3+(i*2)]);
				sinAmplitude[i] = str2ul(params[4+(i*2)]);
			}
			setWaveform(str2ul(params[0]), //Slope
					str2ul(params[1]), //Envelope Shape
					str2ul(params[2]), //Envelope Duty Cycle
					sinFreq,
					sinAmplitude);	   //Sine Component Amplitude
			packet[0] = ACK;
			break;
		case SET_SAMPLE_RATE:
			sineTimerLoad(str2ul(params[0])); //sample frequency
			packet[0] = ACK;
			break;
		case GET_WAVEFORM:
			packet[0] = NACK;
			break;
		case GET_SAMPLE_RATE:
			packet[0] = NACK;
			break;
		default:
			packet[0] = NACK;
	}

	USBSendPacket(packet,1);

}


//*****************************************************************************
//
// Interrupt handler for the system tick counter.
//
//*****************************************************************************
void SysTickIntHandler(void){
	//flashLED(LED_GREEN);
}

void SineTimerIntHandler(void){
	ROM_TimerIntClear(SINE_TIMER_BASE, SINE_TIMER_INT);
	g_sineIntCount ++;
	if(g_sineIntCount > 100){ //Toggle LED every 100 interrupts
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1,
				~GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1));
		g_sineIntCount = 0;
	}
	g_sineTimerVal = (g_timerLoadVal - TimerValueGet(SINE_TIMER_BASE,SINE_TIMER_SIDE)); //Add ISR Access Time
	g_sineTimerMS += ((g_timerLoadVal+g_sineTimerVal)*1000)/g_sysClk;
	stepWaveform(g_sineTimerMS);
}

void I2CIntHandler(void){
}
