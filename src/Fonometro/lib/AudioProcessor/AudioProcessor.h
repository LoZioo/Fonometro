/**
 * @file AudioProcessor.h
 * @author Davide Scalisi
 * @brief class to acquire (from I2S), analyze and extract the main parameters from an audio signal.
 * @version 0.1
 * @date 2023-02-26
 * 
 * @copyright Copyright (c) 2023 Davide Scalisi
 * 
 */

#pragma once
#include <Arduino.h>
#include <driver/i2s.h>

typedef struct {
	i2s_port_t			i2s_unit;
	adc_unit_t			adc_unit;
	adc1_channel_t	adc_channel;

	uint32_t sample_rate;					// Be careful: RAM isn't ever enough.
	uint32_t sampling_time_ms;

	float adc_val_to_volts;
	float preamp_gain;
	float mic_spl_conversion;

} AudioProcessor_init_t;

typedef struct {
	int16_t sample_dc_offset;

	float adc_voltage_rms;
	float mic_voltage_rms;

	int16_t db_spl;
} AudioProcessor_data_t;

class AudioProcessor {
	private:
		const AudioProcessor_init_t &conf;		// Class configurations.
		const uint32_t samples_to_read;

		SemaphoreHandle_t mutex_data = NULL;	// Mutex between evaluate() and get() to prevent critical sections on "data" and "dc_offset".
		AudioProcessor_data_t data;						// AudioProcessor::get return value.
		float dc_offset = 2047;								// Knuth running mean (2047 is the expected DC value).

		int32_t *buff32_int;			// Intended to save samples; range [-2048, 2047].
		float *buff32_float;			// Intended to save SPL values; use the same allocated memory, but use it as float memory.
		uint32_t buff32_len;

		void __read_adc_i2s_samples();

	public:
		AudioProcessor(AudioProcessor_init_t &init);
		~AudioProcessor();

		void begin();
		void end();

		void evaluate();
		AudioProcessor_data_t get();
};
