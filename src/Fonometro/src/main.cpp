#include <Arduino.h>
#include <WiFi.h>

#include <password.h>
#include <const.h>

// Setup routines.
inline void setup_wifi(), spawn_threads();

// Threads and IPC.
void thread_1(void*);
TaskHandle_t thread_1_handle;

void setup(){
	Serial.begin(115200);
	
	// setup_wifi();
	spawn_threads();
}

void loop(){}

void thread_1(void *parameters){
	int cont = 0;

	while(true){
		Serial.println(cont++);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
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

	xTaskCreatePinnedToCore(thread_1, "thread_1", 1024, NULL, 3, &thread_1_handle, APP_CPU);

	// Deleting the spawner thread (setup thread).
	vTaskDelete(NULL);
}
