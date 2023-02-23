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

/*
	In this example, the Arduino simulates the sampling of a sinusoidal 1000 Hz
	signal with an amplitude of 100, sampled at 5000 Hz. Samples are stored
	inside the vReal array. The samples are windowed according to Hamming
	function. The FFT is computed using the windowed samples. Then the magnitudes
	of each of the frequencies that compose the signal are calculated. Finally,
	the frequency with the highest peak is obtained, being that the main frequency
	present in the signal.
*/

// #include <Arduino.h>
// #include <arduinoFFT.h>

// void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType);

// arduinoFFT FFT = arduinoFFT();	// Create FFT object

// // These values can be changed in order to evaluate the functions
// const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
// const double signalFrequency = 1000;
// const double samplingFrequency = 5000;
// const uint8_t amplitude = 100;

// /*
// These are the input and output vectors
// Input vectors receive computed results from FFT
// */
// double vReal[samples];
// double vImag[samples];

// #define SCL_INDEX 0x00
// #define SCL_TIME 0x01
// #define SCL_FREQUENCY 0x02
// #define SCL_PLOT 0x03

// void setup(){
// 	Serial.begin(115200);
// 	while(!Serial);
// 	Serial.println("Ready");

// 	/* Build raw data */
// 	double cycles = (((samples-1) * signalFrequency) / samplingFrequency); //Number of signal cycles that the sampling will read
// 	for(uint16_t i = 0; i < samples; i++){
// 		vReal[i] = int8_t((amplitude * (sin((i * (twoPi * cycles)) / samples))) / 2.0);/* Build data with positive and negative values*/
// 		//vReal[i] = uint8_t((amplitude * (sin((i * (twoPi * cycles)) / samples) + 1.0)) / 2.0);/* Build data displaced on the Y axis to include only positive values*/
// 		vImag[i] = 0.0; //Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
// 	}

// 	/* Print the results of the simulated sampling according to time */
// 	Serial.println("Data:");
// 	PrintVector(vReal, samples, SCL_TIME);

// 	FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
// 	Serial.println("Weighed data:");
// 	PrintVector(vReal, samples, SCL_TIME);

// 	FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
// 	Serial.println("Computed Real values:");
// 	PrintVector(vReal, samples, SCL_INDEX);

// 	Serial.println("Computed Imaginary values:");
// 	PrintVector(vImag, samples, SCL_INDEX);

// 	FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
// 	Serial.println("Computed magnitudes:");
// 	PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);

// 	double x = FFT.MajorPeak(vReal, samples, samplingFrequency);
// 	Serial.println(x, 6);
// }

// void loop(){}

// void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType){
// 	for(uint16_t i = 0; i < bufferSize; i++){
// 		double abscissa;
// 		/* Print abscissa value */
// 		switch (scaleType){
// 			case SCL_INDEX:
// 				abscissa = (i * 1.0);
// 				break;

// 			case SCL_TIME:
// 				abscissa = ((i * 1.0) / samplingFrequency);
// 				break;

// 			case SCL_FREQUENCY:
// 				abscissa = ((i * 1.0 * samplingFrequency) / samples);
// 				break;
// 		}

// 		Serial.print(abscissa, 6);

// 		if(scaleType==SCL_FREQUENCY)
// 			Serial.print("Hz");

// 		Serial.print(" ");
// 		Serial.println(vData[i], 4);
// 	}
// 	Serial.println();
// }
