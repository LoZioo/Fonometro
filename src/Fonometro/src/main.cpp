#include <Arduino.h>
#include <WiFi.h>

#include <password.h>
#include <const.h>

// Setup routines.
inline void setup_wifi(), spawn_threads();

// Threads and IPC.
void thread_1(void*);
TaskHandle_t thread_1_handle;

void loop(){}
void setup(){
	Serial.begin(115200);

	// setup_wifi();
	spawn_threads();
}

void thread_1(void *parameters){
	// f = 1 / (SAMPLES_LEN * DELAY)
	// f = 1 / (50 * 0.001)

	const uint16_t SAMPLES_LEN = 50;
	const uint8_t SAMPLES[] = { 100, 112, 124, 136, 148, 158, 168, 177, 184, 190, 195, 198, 199, 199, 198, 195, 190, 184, 177, 168, 158, 148, 136, 124, 112, 100, 87, 75, 63, 51, 41, 31, 22, 15, 9, 4, 1, 0, 0, 1, 4, 9, 15, 22, 31, 41, 51, 63, 75, 87 };

	while(true){
		for(int i=0; i<SAMPLES_LEN; i++){
			dacWrite(DAC1, SAMPLES[i]);
			delayMicroseconds(100);

			// vTaskDelay(1 / portTICK_PERIOD_MS);
		}
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
