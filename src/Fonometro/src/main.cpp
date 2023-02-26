#include <Arduino.h>
#include <WiFi.h>

#include <driver/i2s.h>
// #include <fft.h>

#include <password.h>
#include <const.h>

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
	
	// Range [-2048, 2047].
	int32_t *buff32_int = new int32_t[SAMPLES_TO_READ];							// Intended to save samples. 
	float *buff32_float = (float*) buff32_int;											// Intended to save SPL values.
	// Use the same allocated memory, but use it as float memory.

	uint32_t buff32_len;

	// Knuth running mean (2047 is the expected DC value).
	float dc_offset = 2047;

	while(true){
		// Read samples [0, 4096].
		buff32_len = read_adc_i2s_samples(buff32_int, SAMPLES_TO_READ);

		float adc_voltage_rms = 0;
		for(int i=0; i<buff32_len; i++){
			// Extract only the needed bits.
			buff32_int[i] = (buff32_int[i] >> 16) & 0xFFF;

			// Running mean.
			dc_offset = dc_offset + ((float) buff32_int[i] - dc_offset) / buff32_len;

			// Data extraction.
			float adc_voltage = (buff32_int[i] - round(dc_offset)) * ADC_VAL_TO_VOLTS;
			float mic_voltage = adc_voltage / PREAMP_GAIN;

			// Needed for the FFT.
			// From now, buff32_float contains the SPL values [20uPa, 30Pa].
			buff32_float[i] = round(20 * log10(mic_voltage / MIC_DB_SPL_CONVERSION));

			// Needed for the other values.
			adc_voltage_rms += pow(adc_voltage, 2);
		}

		// Aggregate data.
		adc_voltage_rms = sqrt(adc_voltage_rms / buff32_len);
		float mic_voltage_rms = adc_voltage_rms / PREAMP_GAIN;
		int16_t db_spl = round(20 * log10(mic_voltage_rms / MIC_DB_SPL_CONVERSION));

		Serial.printf("buff32_len: %d\n", buff32_len);
		Serial.printf("dc_offset: %f\n", dc_offset);
		Serial.printf("adc_voltage_rms_mV: %f\n", adc_voltage_rms * 1000);
		Serial.printf("mic_voltage_rms_mV: %f\n", mic_voltage_rms * 1000);
		Serial.printf("db_spl: %d\n", db_spl);
		Serial.println();

		vTaskDelay(SENDING_DATA_PERIOD_MS / portTICK_PERIOD_MS);
	}
}

void main_thread(void *parameters){
	Serial.println("main_thread");

	while(true){
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
