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
	samples = new int16_t[samples_to_read];
}

AudioProcessor::~AudioProcessor(){
	end();
	delete[] samples;
}

void AudioProcessor::begin(){
	// xSemaphoreCreateMutex initializes to ONE.
	// This is fine as it is a Mutex.
	mutex_data = xSemaphoreCreateMutex();

	// I2S configurations.
	i2s_config_t adc_i2s_config = {
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
		.sample_rate = conf.sample_rate,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
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
	for(int i=0; i<samples_len; i++){
		// Extract only the needed bits.
		samples[i] &= 0xFFF;

		// Running mean.
		dc_offset = dc_offset + ((float) samples[i] - dc_offset) / samples_len;

		// Data extraction.
		float adc_voltage = (samples[i] - round(dc_offset)) * conf.adc_val_to_volts;
		float mic_voltage = adc_voltage / conf.preamp_gain;

		data.adc_voltage_rms += pow(adc_voltage, 2);
	}

	data.adc_voltage_rms = sqrt(data.adc_voltage_rms / samples_len);
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

// Read samples from the I2S DMA buffer (blocking function).
void AudioProcessor::__read_adc_i2s_samples(){
	size_t bytes_read = 0;

	i2s_read(conf.i2s_unit, samples, samples_to_read * sizeof(uint16_t), &bytes_read, portMAX_DELAY);
	samples_len = bytes_read / sizeof(uint16_t);		// samples_read
}
