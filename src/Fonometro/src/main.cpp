#include <Arduino.h>
#include <WiFi.h>

#include <password.h>
#include <const.h>

// Setup routines.
inline void setup_wifi(), spawn_threads(), setup_timers();

// ISR.
void IRAM_ATTR timer0_OVF_ISR();

// Hardware timers.
hw_timer_t *timer0;

// Threads and IPC.
void thread_1(void*);
TaskHandle_t thread_1_handle;

void loop(){}
void setup(){
	Serial.begin(115200);

	// setup_wifi();
	setup_timers();
	// spawn_threads();
}

void thread_1(void *parameters){
	// const uint16_t SAMPLES_LEN = 10;
	// const uint8_t SAMPLES[] = { 100, 158, 195, 195, 158, 100, 41, 4, 4, 41 };

	// while(true){
	// 	for(int i=0; i<SAMPLES_LEN; i++){
	// 		dacWrite(DAC1, SAMPLES[i]);
	// 		// delayMicroseconds(20);

	// 		vTaskDelay(1 / portTICK_PERIOD_MS);
	// 	}
	// }
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

inline void setup_timers(){
	timer0 = timerBegin(0, PRESCALER, true);
	timerAlarmWrite(timer0, AUTORELOAD, true);
	timerAttachInterrupt(timer0, &timer0_OVF_ISR, true);

	timerWrite(timer0, 0);
	timerAlarmEnable(timer0);
}

// Sinewave generator.
volatile uint16_t k = 0;

const uint16_t SAMPLES_LEN = 40;
const uint8_t SAMPLES[] = { 127, 138, 150, 161, 171, 180, 187, 193, 198, 201, 202, 201, 198, 193, 187, 180, 171, 161, 150, 138, 127, 115, 103, 92, 82, 73, 66, 60, 55, 52, 52, 52, 55, 60, 66, 73, 82, 92, 103, 115 };

void IRAM_ATTR timer0_OVF_ISR(){
	if(k == SAMPLES_LEN)
		k = 0;

	dacWrite(DAC1, SAMPLES[k++]);
}
