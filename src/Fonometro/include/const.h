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

// Conversion factors.
#define ADC_VAL_TO_VOLTS	8.056640625e-4	// 3.3V / 4096
#define PREAMP_GAIN				9.2

// Mic sensitivity (https://www.dsprelated.com/showthread/audiodsp/1153-1.php).
// Theoretical values.
// #define MIC_SENSITIVITY_DB			-50											// db(Vrms/Pa)
// #define MIC_SENSITIVITY				3.1622776601683793e-3		// pow(10, MIC_SENSITIVITY_DB / 20)			// Vrms/Pa.
// #define MIN_SOUND_SPL_THR			2e-5										// 20 uPascal
// #define MIC_DB_SPL_CONVERSION	6.3245553203367586e-8		// MIC_SENSITIVITY * MIN_SOUND_SPL_THR

// Empyric value: you have to solve "20 * log10(READ_MIC_VOLTAGE_RMS / x) = REAL_DB_VALUE (from another audio meter)".
#define MIC_DB_SPL_CONVERSION	5.26799e-6		// MIC_SENSITIVITY * MIN_SOUND_SPL_THR

// I2S and ADC configurations.
#define I2S_UNIT					I2S_NUM_0

#define ADC_UNIT					ADC_UNIT_1
#define ADC_CHANNEL				ADC1_CHANNEL_4		// MIC_SUM, GPIO32.
#define ADC_SAMPLE_RATE		16000
#define ADC_DEPTH					I2S_BITS_PER_SAMPLE_16BIT

// Sampling configurations.
#define	REC_TIME_SEC						1
#define SAMPLES_TO_READ					(uint32_t) ceil(REC_TIME_SEC * ADC_SAMPLE_RATE)
#define SENDING_DATA_PERIOD_MS	500

// I2S configurations.
i2s_config_t adc_i2s_config = {
	.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
	.sample_rate = ADC_SAMPLE_RATE,
	.bits_per_sample = ADC_DEPTH,
	.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
	.communication_format = I2S_COMM_FORMAT_STAND_MSB,
	.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
	.dma_buf_count = 4,
	.dma_buf_len = 1024,
	.use_apll = false,
	.tx_desc_auto_clear = false,
	.fixed_mclk = 0
};
