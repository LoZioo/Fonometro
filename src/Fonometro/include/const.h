/*
	ESP32-D0WD-V3 (revision v3.0) (CH9102X):
		RAM:		320kB
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

// Main thread delay.
#define SENDING_DATA_PERIOD_MS	500

// Mic sensitivity (https://www.dsprelated.com/showthread/audiodsp/1153-1.php).
// Theoretical values.
// #define MIC_SENSITIVITY_DB			-50											// db(Vrms/Pa)
// #define MIC_SENSITIVITY				3.1622776601683793e-3		// pow(10, MIC_SENSITIVITY_DB / 20)			// Vrms/Pascal.
// #define MIN_SOUND_SPL_THR			2e-5										// 20 uPascal
// #define MIC_SPL_CONVERSION			6.3245553203367586e-8		// MIC_SENSITIVITY * MIN_SOUND_SPL_THR

AudioProcessor_init_t AudioProcessor_init {
	.i2s_unit =			I2S_NUM_0,
	.adc_unit =			ADC_UNIT_1,
	.adc_channel =	ADC1_CHANNEL_4,		// MIC_SUM, GPIO32.

	.sample_rate =			16000,				// Be careful: RAM isn't ever enough.
	.sampling_time_ms =	1000,

	.adc_val_to_volts =		8.056640625e-4,		// 3.3V / 4096
	.preamp_gain =				9.2,
	.mic_spl_conversion =	5.26799e-6	// Empyric value: you have to solve "20 * log10(READ_MIC_VOLTAGE_RMS / x) = REAL_DB_VALUE (from another audio meter)".
};
