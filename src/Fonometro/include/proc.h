// Setup routines.
inline void setup_gpio(){
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
