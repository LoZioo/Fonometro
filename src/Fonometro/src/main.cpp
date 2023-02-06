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

// Semaphores.
SemaphoreHandle_t sem_ready_for_sampling = NULL;
SemaphoreHandle_t sem_done_sampling = NULL;

// Variables.
uint16_t samples[SAMPLES_LEN];
uint32_t sample_index = 0;

void loop(){}
void setup(){
	Serial.begin(115200);

	setup_GPIOs();
	// setup_wifi();

	setup_timers();
	// start_timer(timer1);

	setup_IPCs();
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

inline void setup_IPCs(){
	// Sampling semaphore.
	// xSemaphoreCreateBinary initializes to ZERO.
	// This is fine as it is a binary semaphore.
	sem_ready_for_sampling = xSemaphoreCreateBinary();
	sem_done_sampling = xSemaphoreCreateBinary();
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

void IRAM_ATTR sample_thread(void *parameters){
	Serial.println("sample_thread");

	// Start sampling from MIC_SUM pin.
	start_timer(timer0);

	while(true){
		// Wait for the call of xSemaphoreGiveFromISR from timer0_OVF_ISR.
		xSemaphoreTake(sem_ready_for_sampling, portMAX_DELAY);
		samples[sample_index++] = analogRead(MIC_SUM);

		if(sample_index == SAMPLES_LEN){
			// sample_thread will be locked at the next loop.
			stop_timer(timer0);

			// Unlock main_thread.
			xSemaphoreGive(sem_done_sampling);
			sample_index = 0;
		}
	}
}

void main_thread(void *parameters){
	Serial.println("main_thread");

	// Wait for the call of xSemaphoreGive from sample_thread.
	xSemaphoreTake(sem_done_sampling, portMAX_DELAY);

	Serial.println("Done.");

	while(true){
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

inline void setup_timers(){
	timer0 = timerBegin(0, TIMER0_PRE, true);
	timerAlarmWrite(timer0, TIMER0_ARR, true);
	timerAttachInterrupt(timer0, &timer0_OVF_ISR, true);

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
	BaseType_t task_woken = pdFALSE;

	// Deferred interrupt for sampling.
	// If sample_thread can't handle the set speed,
	// this error will be printed out.
	if(xSemaphoreGiveFromISR(sem_ready_for_sampling, &task_woken) == errQUEUE_FULL)
		Serial.println("errQUEUE_FULL in xSemaphoreGiveFromISR.");
  
	// API to implement deferred interrupt.
  // Exit from ISR (Vanilla FreeRTOS).
  // portYIELD_FROM_ISR(task_woken);

  // Exit from ISR (ESP-IDF).
  if(task_woken)
    portYIELD_FROM_ISR();
}

// Sinewave generator.
volatile uint16_t k = 0;

void IRAM_ATTR timer1_OVF_ISR(){
	if(k == DAC_SAMPLES_LEN)
		k = 0;

	dacWrite(DAC1, DAC_SAMPLES[k++]);
}
