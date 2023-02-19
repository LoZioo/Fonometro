#include <Arduino.h>
#include <WiFi.h>

#include <password.h>
#include <const.h>
#include <proc.h>

// Hardware timers.
hw_timer_t *timer0, *timer1;

// Threads.
void sample_thread(void*), main_thread(void*);
TaskHandle_t sample_thread_handle, main_thread_handle;

void loop(){}
void setup(){
	Serial.begin(115200);

	setup_GPIOs();
	// setup_wifi();

	spawn_threads();
}

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

void sample_thread(void *parameters){
	Serial.println("sample_thread");

	while(true){
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void main_thread(void *parameters){
	Serial.println("main_thread");

	while(true){
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
