#include <Arduino.h>
#include <WiFi.h>

#include <password.h>
#include <const.h>

// Setup routines.
inline void setup_GPIOs(), setup_wifi(), spawn_threads(), setup_timers();

// ISR.
void IRAM_ATTR timer0_OVF_ISR(), timer1_OVF_ISR();

// Macros.
inline void start_timer(hw_timer_t*), stop_timer(hw_timer_t*);

// Hardware timers.
hw_timer_t *timer0, *timer1;

// Threads and IPC.
void sample_thread(void*);
TaskHandle_t sample_thread_handle;

void loop(){}
void setup(){
	Serial.begin(115200);

	setup_GPIOs();
	// setup_wifi();
	
	setup_timers();
	start_timer(timer0);

	spawn_threads();
}

void sample_thread(void *parameters){
	while(true){
	}
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

	xTaskCreatePinnedToCore(sample_thread, "sample_thread", 1024, NULL, 3, &sample_thread_handle, APP_CPU);

	// Deleting the spawner thread (setup thread).
	vTaskDelete(NULL);
}

inline void setup_timers(){
	timer0 = timerBegin(1, TIMER0_PRE, true);
	timerAlarmWrite(timer1, TIMER0_ARR, true);
	timerAttachInterrupt(timer1, &timer0_OVF_ISR, true);

	timer1 = timerBegin(1, TIMER1_PRE, true);
	timerAlarmWrite(timer1, TIMER1_ARR, true);
	timerAttachInterrupt(timer1, &timer1_OVF_ISR, true);
}

inline void start_timer(hw_timer_t *timer){
	timerWrite(timer, 0);
	timerAlarmEnable(timer);
}

inline void stop_timer(hw_timer_t *timer){
	timerAlarmDisable(timer);
}

// Sampling timer.
void IRAM_ATTR timer0_OVF_ISR(){

}

// Sinewave generator.
volatile uint16_t k = 0;

const uint16_t SAMPLES_LEN = 40;
const uint8_t SAMPLES[] = { 127, 138, 150, 161, 171, 180, 187, 193, 198, 201, 202, 201, 198, 193, 187, 180, 171, 161, 150, 138, 127, 115, 103, 92, 82, 73, 66, 60, 55, 52, 52, 52, 55, 60, 66, 73, 82, 92, 103, 115 };

void IRAM_ATTR timer1_OVF_ISR(){
	if(k == SAMPLES_LEN)
		k = 0;

	dacWrite(DAC1, SAMPLES[k++]);
}
