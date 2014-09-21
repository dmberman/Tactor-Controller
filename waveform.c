#include "waveform.h"
#include "main.h"
#include "inc/hw_types.h"
#include "PCA9685.h"
#include "inc/hw_flash.h"
#include "driverlib/flash.h"



extern unsigned long g_unsavedWaveform;
const unsigned char flashBlock[2048]__attribute__ ((aligned(2048))) = { 0x00, 0x00, 0x00, 0x00 };

volatile tWaveform waveforms[WAVEFORM_LIBRARY_LENGTH];
volatile tMotor motors[N_MOTORS];
volatile unsigned long dutyCycles[N_MOTORS];

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
		break;

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
		break;

	case SINE:
		if(t < onTime){
			return (sin(t/onTime * PI_2)+1)/2;
		}
		break;
	}
	return 0;
}

//Calculates the next duty cycle values and updates motors.
/* Input: currentMS = current time in ms
 *
 */
void stepWaveform(unsigned long currentMS){
	unsigned long i,j, wInd;
	float envelopeComp = 0;
	unsigned long sineComp = 0;
	unsigned long magnitude = 0;
	for(i=0;i<N_MOTORS;i++){ //Calculate duty cycle for each motor
		if(motors[i].active == MOTOR_ACTIVE){ //If motor is active
			wInd = motors[i].waveform;
			envelopeComp = (motors[i].amplitudeStretch) * calcEnvelope(waveforms[wInd].envelopeShape,
																	  waveforms[wInd].envelopeDutyCycle, ((motors[i].frequencyStretch*currentMS)/10)%1000); //Ranges from 0-1 to define envelope
			sineComp = 0;
			magnitude = 0;
			for(j=0;j<N_SINE_COMPONENTS;j++){
				sineComp += (waveforms[wInd].sinAmplitude[j] * cos(waveforms[wInd].sinw[j]*(motors[i].frequencyStretch*currentMS)/10000.0)+1)/2; //Ranges from 0-4095 to define sine components
				magnitude += waveforms[wInd].sinAmplitude[j];
			}
			dutyCycles[i] = (envelopeComp * sineComp)/magnitude;
		}
	}
	for(i=0;i<N_MOTORS;i++){ //Set duty cycle for each motor
		if(motors[i].active == MOTOR_ACTIVE){
			PCA9685_setDutyCycle(i, dutyCycles[i]);
		}
	}
}
