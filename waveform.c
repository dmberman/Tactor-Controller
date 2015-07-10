#include "waveform.h"
#include "main.h"
#include "PCA9685.h"
#include "inc/hw_types.h"
#include "inc/hw_flash.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/flash.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"

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



extern unsigned long g_unsavedWaveform;
const unsigned char flashBlock[2048]__attribute__ ((aligned(2048))) = { 0x00, 0x00, 0x00, 0x00 };

volatile tWaveform waveforms[WAVEFORM_LIBRARY_LENGTH];
volatile tMotor motors[N_MOTORS];
volatile unsigned long dutyCycles[N_MOTORS];

void initSineTimer(unsigned long sampleFreq){
	ROM_SysCtlPeripheralEnable(SINE_TIMER_PERHIP);
	ROM_TimerConfigure(SINE_TIMER_BASE,TIMER_CFG_PERIODIC);
	ROM_TimerControlStall(SINE_TIMER_BASE,SINE_TIMER_SIDE,1);
    g_ulSampleFreq = sampleFreq; //Sample period
    g_sysClk = ROM_SysCtlClockGet();
    g_timerLoadVal = g_sysClk/sampleFreq;
    ROM_TimerLoadSet(SINE_TIMER_BASE,SINE_TIMER_SIDE,g_timerLoadVal);
    ROM_TimerIntClear(SINE_TIMER_BASE,SINE_TIMER_INT);
    ROM_TimerEnable(SINE_TIMER_BASE,SINE_TIMER_SIDE);
    ROM_TimerIntEnable(SINE_TIMER_BASE,SINE_TIMER_INT);
    ROM_IntEnable(SINE_TIMER_IE);
}

unsigned long waveformFlashSave(void){
    FlashErase((unsigned long) flashBlock);
    FlashProgram((unsigned long*) &waveforms, (unsigned long) flashBlock, sizeof(waveforms));
    g_unsavedWaveform = FALSE;
    return 0;
}

unsigned long waveformFlashLoad(void){
    memcpy(&waveforms, flashBlock, sizeof(waveforms));
    return 0;
}

unsigned long initWaveform(void){
	unsigned long i;
	waveformFlashLoad();
	for(i = 0;i<N_MOTORS;i++){
		stopMotor(i);
	}
    initSineTimer(50);
	return 0;
}

// Motor Functions
unsigned long runMotor(unsigned long motor, unsigned long waveform, unsigned long amplitudeStretch, unsigned long frequencyStretch){
	motors[motor].waveform = waveform;
	motors[motor].amplitudeStretch = amplitudeStretch;
	motors[motor].frequencyStretch = frequencyStretch;
	motors[motor].active = MOTOR_ACTIVE;
	return 0;
}

unsigned long stopMotor(unsigned long motor){
	motors[motor].active = MOTOR_INACTIVE;
	PCA9685_setDutyCycle(motor, OFF_DUTY_CYCLE);
	return 0;
}

// Waveform Functions

/* Checks if given waveform slot is currently in use
 * Inputs: slot - slot # to check
 * Outputs: 0: Slot is available for new use
 * 			1: Slot is currently filled
 */
unsigned long slotFull(unsigned long slot){
	return (waveforms[slot].envelopeShape > WAVEFORM_UNDEFINED);
}


/* Sets given waveform slot to prescribed waveform */
unsigned long setWaveform(unsigned long slot, unsigned long envelopeShape, unsigned long envelopeDutyCycle,
		unsigned long sinFreq[], unsigned long sinAmpl[]){
	unsigned long i;
	waveforms[slot].envelopeShape = envelopeShape;
	waveforms[slot].envelopeDutyCycle = envelopeDutyCycle;
	for(i = 0;i<N_SINE_COMPONENTS;i++){
		waveforms[slot].sinw[i] = (sinFreq[i]/10.0)*PI_2;
		waveforms[slot].sinAmplitude[i] = sinAmpl[i];
	}
	g_unsavedWaveform = TRUE;
	return 0;
}


/* Checks if given waveform slot is currently in use
 * Inputs: slot - slot # to check
 * 		   *waveform - pointer to waveform space to return waveform parameters
 * Outputs: 0: Slot is empty  (waveform invalid)
 * 			1: Slot is filled (waveform valid)
 */
unsigned long getWaveform(unsigned long slot, tWaveform *waveform){
	*waveform = waveforms[slot];
	return slotFull(slot);
}

/*
 * Calculates the envelope amplitude (0-1) based on envelope type, duty cycle (0-100), and time (0-1000)
 */
float calcEnvelope(unsigned long envelopeShape, unsigned long envelopeDutyCycle, unsigned long t){
	float onTime = envelopeDutyCycle*10;
	switch(envelopeShape){
	case SQUARE:
		if(t < onTime){
			return 1;
		}
		else{
			return 0;
		}

	case TRIANGLE:
		if(t <= (onTime/2)){
			return ((2*t)/onTime);
		}
		else{
			if(t < onTime){
				return 2-((2*t)/onTime);
			}
			else{
				return 0;
			}
		}

	case SINE:
		if(t < onTime){
			return (sin(t/onTime * PI_2)+1)/2;
		}
		break;
	}
	return 0;
}

unsigned long getCurrentMS(void){
	return g_sineTimerMS;
}

void SineTimerIntHandler(void){
	unsigned long i,j, wInd;
	float envelopeComp = 0;
	unsigned long sineComp = 0;
	unsigned long magnitude = 0;

	ROM_TimerIntClear(SINE_TIMER_BASE, SINE_TIMER_INT);
	g_sineIntCount ++;
	#ifdef __DEBUG_SINE_HEARTBEAT__
	if(g_sineIntCount > 200){ //Sine Timer Heartbeat
		UARTprintf("\nSine Timer Heartbeat");
			#ifdef __DEBUG_TRACEBACK__
				UARTprintf(" File: %s Line: %d", __FILE__,__LINE__);
			#endif
		g_sineIntCount = 0;
	}
	#endif
	g_sineTimerMS += (g_timerLoadVal*1000)/g_sysClk;


	for(i=0;i<N_MOTORS;i++){ //Calculate duty cycle for each motor
		if(motors[i].active == MOTOR_ACTIVE){ //If motor is active
			wInd = motors[i].waveform;
			g_sineTimerVal = (g_timerLoadVal - TimerValueGet(SINE_TIMER_BASE,SINE_TIMER_SIDE));
			g_sineTimerMS += (g_sineTimerVal*1000)/g_sysClk; //Add Access Time

			envelopeComp = (motors[i].amplitudeStretch) * calcEnvelope(waveforms[wInd].envelopeShape,
																	  waveforms[wInd].envelopeDutyCycle, ((motors[i].frequencyStretch*g_sineTimerMS)/10)%1000); //Ranges from 0-1 to define envelope
			sineComp = 0;
			magnitude = 0;
			for(j=0;j<N_SINE_COMPONENTS;j++){
				sineComp += (waveforms[wInd].sinAmplitude[j] * cos(waveforms[wInd].sinw[j]*(motors[i].frequencyStretch*g_sineTimerMS)/10000.0)+1)/2; //Ranges from 0-4095 to define sine components
				magnitude += waveforms[wInd].sinAmplitude[j];
			}
			dutyCycles[i] = (envelopeComp * sineComp)/magnitude;
			PCA9685_setDutyCycle(i, dutyCycles[i]); //set duty cycle
		}
	}

}
