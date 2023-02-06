/*
	ESP32-D0WD-V3 (revision v3.0) (CH9102X):
		RAM:		?? 327.680 bytes
		Flash:	4MB
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

// Sampling timer parameters.
#define TIMER0_PRE	2000		// 80.000.000Hz / 2000 = 40.000Hz = 40 * 1.000Hz
#define TIMER0_ARR	1

// Sinewave timer parameters for the signal generator (DAC).
#define TIMER1_PRE	2000		// 80.000.000Hz / 2000 = 40.000Hz = 40 * 1.000Hz
#define TIMER1_ARR	1
