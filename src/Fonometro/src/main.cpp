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

	float *adc_voltages = new float[SAMPLES_TO_READ];

	while(true){
		samples_len = read_adc_i2s_samples(samples, SAMPLES_TO_READ);

		uint64_t dc_offset = 0;
		for(int i=0; i<samples_len; i++){
			samples[i] &= 0x0FFF;
			dc_offset += samples[i];
		}
		dc_offset /= samples_len;

		float adc_voltage_rms = 0;
		for(int i=0; i<samples_len; i++){
			samples[i] -= dc_offset;
			adc_voltages[i] = samples[i] * SAMPLE_ADC_VAL_TO_VOLTS;
			adc_voltage_rms += pow(adc_voltages[i], 2);
		}
		// ??? Valore da verificare.
		adc_voltage_rms = sqrt(adc_voltage_rms / samples_len);
		
		float mic_voltage_rms = adc_voltage_rms / PREAMP_GAIN;
		float db_spl = 20 * log10(mic_voltage_rms / (MIC_SENSITIVITY * MIN_SOUND_SPL_THR));

		Serial.printf("dc_offset: %d\n", dc_offset);
		Serial.printf("adc_voltage_rms: %f\n", adc_voltage_rms);
		Serial.printf("mic_voltage_rms: %f\n", mic_voltage_rms);
		Serial.printf("db_spl: %f\n", db_spl);
		Serial.println();

		// for(int i=0; i<10; i++)
		// 	Serial.printf("samples[%d] = %d\n", i, samples[i]);

		vTaskDelay(SENDING_DATA_PERIOD_MS / portTICK_PERIOD_MS);
	}
}

void main_thread(void *parameters){
	Serial.println("main_thread");

	while(true){
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
