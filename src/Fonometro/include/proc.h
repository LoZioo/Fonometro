// Setup routines.
inline void setup_gpio(){
	pinMode(MIC_SUM, INPUT);
	pinMode(MIC_1, INPUT);
	pinMode(MIC_2, INPUT);
	pinMode(MIC_3, INPUT);
	pinMode(MIC_4, INPUT);
}

// Spawn and handle the captive portal and then reset after the configuration.
inline void handle_captive_portal(){
	DNSServer dns;
	AsyncWebServer server(WIFI_WEBSERVER_PORT);

	dns.start(WIFI_DNS_PORT, "*", WIFI_AP_IP);
	MDNS.begin(WIFI_AP_NAME);
	LittleFS.begin();

	server.on("/wifi-set", HTTP_POST, [](AsyncWebServerRequest *req){
		Serial.printf("SSID: %s\nPassword: %s\n", req->arg("ssid").c_str(), req->arg("pass").c_str());
		req->redirect("/index.html");
	});

	server.onNotFound([](AsyncWebServerRequest *req){
		req->redirect("/index.html");
	});

	server.serveStatic("/", LittleFS, "/wifi-config/");
	server.begin();

	while(true)
		dns.processNextRequest();
}

// Try to connect to WiFi, otherwise go into access point mode, wait to be configured and then reset.
inline void setup_wifi(){
	WiFi.mode(WIFI_STA);
	WiFi.begin(STA_SSID, STA_PSK);

	uint8_t attempts = 0;
	while(WiFi.status() != WL_CONNECTED && attempts < WIFI_STA_CONNECTION_ATTEMPTS){
		attempts++;
		delay(1000);
	}

	if(attempts == WIFI_STA_CONNECTION_ATTEMPTS){
		WiFi.mode(WIFI_AP);
		WiFi.softAP(WIFI_AP_NAME);
		WiFi.softAPConfig(WIFI_AP_IP, WIFI_AP_IP, WIFI_AP_SUBNET);

		handle_captive_portal();
	}
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

	xTaskCreatePinnedToCore(sample_thread,		"sample_thread",		10240,	NULL,	1,	&sample_thread_handle,		APP_CPU);
	xTaskCreatePinnedToCore(webserver_thread,	"webserver_thread",	10240,	NULL,	1,	&webserver_thread_handle,	PRO_CPU);

	// Deleting the spawner thread (setup thread).
	vTaskDelete(NULL);
}
