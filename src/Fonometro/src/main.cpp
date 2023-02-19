#include <Arduino.h>
#include <WiFi.h>

#include <driver/i2s.h>

#include <password.h>
#include <const.h>

// Hardware timers.
hw_timer_t *timer0, *timer1;

// Threads.
void sample_thread(void*), main_thread(void*);
TaskHandle_t sample_thread_handle, main_thread_handle;

#include <proc.h>

void loop(){}
void setup(){
	Serial.begin(115200);

	setup_gpio();
	// setup_wifi();

	setup_adc_i2s();
	spawn_threads();
}

void sample_thread(void *parameters){
	Serial.println("sample_thread");

	int16_t *samples = new int16_t[SAMPLES_TO_READ];
	uint32_t samples_len;

	float *db_spl = new float[SAMPLES_TO_READ];

	while(true){
		samples_len = read_adc_i2s_samples(samples, SAMPLES_TO_READ);

		float db_spl_rms = 0;
		for(int i=0; i<samples_len; i++){
			float refined_sample = (samples[i] & 0x0FFF) - SAMPLE_SHIFT;
			float adc_voltage = refined_sample * SAMPLE_ADC_VAL_TO_VOLTS;
			float mic_voltage = adc_voltage / AMP_GAIN;
			db_spl[i] = 20 * log10(mic_voltage / (MIC_SENSITIVITY * MIN_SOUND_SPL_THR));

			db_spl_rms += pow(db_spl[i], 2);
		}

		db_spl_rms = sqrt(db_spl_rms / samples_len);
		Serial.printf("db_spl_rms: %f\n", db_spl);

		// int16_t min = 5000, max = -1;
		// float mean = 0, rms = 0;

		// for(int i=0; i<samples_read; i++){
		// 	int16_t filtered_sample = (samples[i] & 0x0FFF) - SAMPLE_SHIFT;

		// 	if(filtered_sample < min)
		// 		min = filtered_sample;
			
		// 	if(filtered_sample > max)
		// 		max = filtered_sample;

		// 	mean += filtered_sample;
		// 	rms += pow(filtered_sample, 2);
		// }
		
		// mean = mean / samples_read;
		// rms = sqrt(rms / samples_read);

		// float rms = 0;

		// for(int i=0; i<samples_read; i++){
		// 	int16_t filtered_sample = (samples[i] & 0x0FFF) - SAMPLE_SHIFT;

		// 	float adc_voltage = filtered_sample * SAMPLE_ADC_VAL_TO_VOLTS;
		// 	float mic_voltage = adc_voltage / AMP_GAIN;
		// 	float db_spl = 20 * log10(mic_voltage / (MIC_SENSITIVITY * MIN_SOUND_SPL_THR));

		// 	rms += pow(db_spl, 2);
		// 	buf[i] = db_spl;
		// }

		// Serial.printf("rms: %f, sample: %f\n", rms, buf[0]);

		vTaskDelay(SENDING_DATA_PERIOD_MS / portTICK_PERIOD_MS);
	}
}

void main_thread(void *parameters){
	Serial.println("main_thread");

	while(true){
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
