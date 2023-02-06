/*
	ESP32-D0WD-V3 (revision v3.0) (CH9102X):
		RAM:		?? 327.680 bytes
		Flash:	4MB
*/

// Protocol CPU and app CPU.
#define PRO_CPU 0
#define APP_CPU 1

// Sinewave timer parameters for the signal generator (DAC).
#define PRESCALER		2000		// 80.000.000Hz / 2000 = 40.000Hz = 40 * 1.000Hz
#define AUTORELOAD	1
