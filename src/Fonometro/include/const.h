/*
	ESP32-D0WD-V3 (revision v3.0) (CH9102X):
		RAM:		?? 327.680 bytes
		Flash:	4MB
		Timers:	
			16 bit prescaler.
			64 bit autoreload.
*/

// GPIOs.
#define MIC_SUM	32
#define MIC_1		36
#define MIC_2		39
#define MIC_3		34
#define MIC_4		35

// Protocol CPU and app CPU.
#define PRO_CPU 0
#define APP_CPU 1

// Sampling timer parameters (16kHz).
#define TIMER0_PRE	5000		// 80.000.000Hz / 5000 = 16.000Hz
#define TIMER0_ARR	1

// The RAM requirements are: 2 byte/sample * SAMPLING_FREQ * SAMPLING_DURATION_S = 32kB
#define SAMPLING_FREQ					16000
#define SAMPLING_DURATION_S		1
#define SAMPLES_LEN						SAMPLING_FREQ * SAMPLING_DURATION_S

// Sinewave timer parameters for the signal generator (DAC).
#define TIMER1_PRE	2000		// 80.000.000Hz / 2000 = 40.000Hz = 40 * 1.000Hz
#define TIMER1_ARR	1

// Sinewave samples.
#define DAC_SAMPLES_LEN 40
const uint8_t DAC_SAMPLES[] = { 127, 138, 150, 161, 171, 180, 187, 193, 198, 201, 202, 201, 198, 193, 187, 180, 171, 161, 150, 138, 127, 115, 103, 92, 82, 73, 66, 60, 55, 52, 52, 52, 55, 60, 66, 73, 82, 92, 103, 115 };
