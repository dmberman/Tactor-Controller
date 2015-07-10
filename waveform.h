#ifndef waveform_h
#define waveform_h

#define WAVEFORM_LIBRARY_LENGTH 16
#define N_SINE_COMPONENTS 2
#define N_MOTORS 16

#define MOTOR_ACTIVE 1
#define MOTOR_INACTIVE 0

//Envelope Shape Parameters
#define WAVEFORM_UNDEFINED 0
#define SQUARE 1
#define TRIANGLE 2
#define SINE 3

#define OFF_DUTY_CYCLE 0 //Duty Cycle sent to PCA9685 when motor is stopped

#define SINE_TIMER_BASE TIMER0_BASE
#define SINE_TIMER_SIDE TIMER_A
#define SINE_TIMER_INT TIMER_TIMA_TIMEOUT
#define SINE_TIMER_PERHIP SYSCTL_PERIPH_TIMER0
#define SINE_TIMER_IE INT_TIMER0A

typedef struct tagWaveform{
	unsigned long envelopeShape;
	unsigned long envelopeDutyCycle;
	unsigned long sinAmplitude[N_SINE_COMPONENTS];
	float sinw[N_SINE_COMPONENTS];
}tWaveform;

typedef struct tagMotor{
	unsigned long active;
	unsigned long waveform;
	unsigned long amplitudeStretch;
	unsigned long frequencyStretch;
}tMotor;

unsigned long initWaveform(void);
void initSineTimer(unsigned long sineFreq);

unsigned long waveformFlashSave(void);
unsigned long getWaveform(unsigned long slot, tWaveform *waveform);
unsigned long setWaveform(unsigned long slot, unsigned long envelopeShape, unsigned long envelopeDutyCycle,
		unsigned long sinFreq[], unsigned long sinAmpl[]);
unsigned long slotFull(unsigned long slot);
unsigned long stopMotor(unsigned long motor);
unsigned long runMotor(unsigned long motor, unsigned long waveform, unsigned long amplitudeStretch, unsigned long frequencyStretch);
unsigned long getCurrentMS(void);
#endif
