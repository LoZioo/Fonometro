// Setup routines.
inline void setup_GPIOs(){
	pinMode(MIC_SUM, INPUT);
	pinMode(MIC_1, INPUT);
	pinMode(MIC_2, INPUT);
	pinMode(MIC_3, INPUT);
	pinMode(MIC_4, INPUT);
}

inline void setup_wifi(){
	WiFi.mode(WIFI_STA);
	WiFi.begin(STA_SSID, STA_PSK);

	while(WiFi.status() != WL_CONNECTED)
		delay(500);
}

inline void setup_adc_i2s(){
	i2s_driver_install(I2S_UNIT, &adc_i2s_config, 0, NULL);		// Using the I2S_0 peripheral with the given configuration.
	i2s_set_adc_mode(ADC_UNIT, ADC_CHANNEL);									// Connect ADC1_CH4 (GPIO32) to the I2S bus; in this mode, the ADC maximum sampling rate is 150KHz.
	i2s_adc_enable(I2S_UNIT);																	// Attach directly I2S_0 to the previously selected ADC1_CH4.

	// From now ADC1 can only be accessed by the I2S_0 peripheral.
}

// Spawn the needed threads and kill the spawner thread.
inline void spawn_threads(){
	/*
		Parameters:
			Function to implement the task.
			Name of the task.
			Stack size in bytes (words in vanilla FreeRTOS).
			Task input parameter.
			Priority of the task.
			Task handle.
			Core where the task should run.
	*/

	xTaskCreatePinnedToCore(sample_thread,	"sample_thread",	10240,	NULL,	1,	&sample_thread_handle,	APP_CPU);
	xTaskCreatePinnedToCore(main_thread,		"main_thread",		10240,	NULL,	1,	&main_thread_handle,		PRO_CPU);

	// Deleting the spawner thread (setup thread).
	vTaskDelete(NULL);
}

// Functions.
/**
 * @brief Read samples from the I2S DMA buffer (blocking function).
 * 
 * @tparam sample_type Type of a single sample.
 * @param buf Buffer where the samples will be loaded.
 * @param to_read Samples to read.
 * @return uint32_t Samples read.
 */
template<class sample_type> uint32_t read_adc_i2s_samples(sample_type *buf, uint32_t to_read){
	size_t bytes_read = 0;

	i2s_read(I2S_UNIT, buf, to_read * sizeof(sample_type), &bytes_read, portMAX_DELAY);
	uint32_t samples_read = bytes_read / sizeof(sample_type);

	return samples_read;
}
