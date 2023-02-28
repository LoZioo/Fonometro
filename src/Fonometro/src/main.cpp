#include <Arduino.h>
#include <WiFi.h>

#include <DNSServer.h>
#include <ESPmDNS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <LittleFS.h>

// #include <fft.h>
#include <AudioProcessor.h>

#include <password.h>
#include <const.h>

// Threads.
void sample_thread(void*), webserver_thread(void*);
TaskHandle_t sample_thread_handle, webserver_thread_handle;

#include <proc.h>

AudioProcessor sound(AudioProcessor_init);

void loop(){}
void setup(){
	Serial.begin(115200);
	sound.begin();

	setup_gpio();
	setup_wifi();

	while(true)
		delay(1000);

	spawn_threads();
}

void sample_thread(void *parameters){
	Serial.println("sample_thread");

	while(true){
		sound.evaluate();
		delay(SENDING_DATA_PERIOD_MS / 4);
	}
}

void webserver_thread(void *parameters){
	Serial.println("webserver_thread");

	AudioProcessor_data_t data;
	while(true){
		data = sound.get();

		Serial.printf("dc_offset: %d\n", data.sample_dc_offset);
		Serial.printf("adc_voltage_rms_mV: %f\n", data.adc_voltage_rms * 1000);
		Serial.printf("mic_voltage_rms_mV: %f\n", data.mic_voltage_rms * 1000);
		Serial.printf("db_spl: %d\n", data.db_spl);
		Serial.println();

		delay(SENDING_DATA_PERIOD_MS);
	}
}
