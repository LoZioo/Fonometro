/**
 * @file AudioProcessor.cpp
 * @author Davide Scalisi
 * @brief class to acquire (from I2S), analyze and extract the main parameters from an audio signal.
 * @version 0.1
 * @date 2023-02-26
 * 
 * @copyright Copyright (c) 2023 Davide Scalisi
 * 
 */

#include <AudioProcessor.h>

AudioProcessor::AudioProcessor(AudioProcessor_init_t &init) :
	conf(init), samples_to_read((uint32_t) ceil(init.sample_rate * init.sampling_time_ms / 1000.0)) {

	// Buffer allocation.
	buff32_int = new int32_t[samples_to_read];
	buff32_float = (float*) buff32_int;
	// Use the same allocated memory, but use it as float memory.
}

AudioProcessor::~AudioProcessor(){
	end();
	delete[] buff32_int;
}

void AudioProcessor::begin(){
	// xSemaphoreCreateMutex initializes to ONE.
	// This is fine as it is a Mutex.
	mutex_data = xSemaphoreCreateMutex();

	// I2S configurations.
	i2s_config_t adc_i2s_config = {
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
		.sample_rate = conf.sample_rate,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
		.communication_format = I2S_COMM_FORMAT_STAND_MSB,
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
		.dma_buf_count = 4,
		.dma_buf_len = 1024,
		.use_apll = false,
		.tx_desc_auto_clear = false,
		.fixed_mclk = 0
	};

	// I2S initialization.
	i2s_driver_install(conf.i2s_unit, &adc_i2s_config, 0, NULL);		// Using the I2S_0 peripheral with the given configuration.
	i2s_set_adc_mode(conf.adc_unit, conf.adc_channel);							// Connect ADC1_CH4 (GPIO32) to the I2S bus; in this mode, the ADC maximum sampling rate is 150KHz.
	i2s_adc_enable(conf.i2s_unit);																	// Attach directly I2S_0 to the previously selected ADC1_CH4.

	// From now ADC1 can only be accessed by the I2S_0 peripheral.
}

void AudioProcessor::end(){
	vSemaphoreDelete(mutex_data);

	i2s_adc_disable(conf.i2s_unit);
	i2s_driver_uninstall(conf.i2s_unit);
}

void AudioProcessor::evaluate(){
	xSemaphoreTake(mutex_data, portMAX_DELAY);

	// Read samples [0, 4096].
	__read_adc_i2s_samples();

	data.adc_voltage_rms = 0;
	for(int i=0; i<buff32_len; i++){
		// Extract only the needed bits.
		buff32_int[i] = (buff32_int[i] >> 16) & 0xFFF;

		// Running mean.
		dc_offset = dc_offset + ((float) buff32_int[i] - dc_offset) / buff32_len;

		// Data extraction.
		float adc_voltage = (buff32_int[i] - round(dc_offset)) * conf.adc_val_to_volts;
		float mic_voltage = adc_voltage / conf.preamp_gain;

		// Needed for the FFT (not implemented).
		// From now, buff32_float contains the SPL values [20uPa, 30Pa].
		buff32_float[i] = round(20 * log10(mic_voltage / conf.mic_spl_conversion));

		// Needed for the other values.
		data.adc_voltage_rms += pow(adc_voltage, 2);
	}

	// Aggregate data.
	data.adc_voltage_rms = sqrt(data.adc_voltage_rms / buff32_len);
	data.mic_voltage_rms = data.adc_voltage_rms / conf.preamp_gain;
	data.db_spl = round(20 * log10(data.mic_voltage_rms / conf.mic_spl_conversion));

	xSemaphoreGive(mutex_data);
}

AudioProcessor_data_t AudioProcessor::get(){
	AudioProcessor_data_t tmp;

	xSemaphoreTake(mutex_data, portMAX_DELAY);
	data.sample_dc_offset = round(dc_offset);
	tmp = data;
	xSemaphoreGive(mutex_data);

	return tmp;
}

/**
 * @brief Read samples from the I2S DMA buffer (blocking function).
 * 
 * @param buf 
 * @param samples_to_read 
 * @return uint32_t number of read samples.
 */
void AudioProcessor::__read_adc_i2s_samples(){
	size_t bytes_read = 0;

	i2s_read(conf.i2s_unit, buff32_int, samples_to_read * sizeof(uint32_t), &bytes_read, portMAX_DELAY);
	buff32_len = bytes_read / sizeof(uint32_t);		// samples_read
}
