#include <Arduino.h>
#include <WiFi.h>

#include <DNSServer.h>
#include <ESPmDNS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <LittleFS.h>

// #include <fft.h>
#include <AudioProcessor.h>
#include <const.h>

// Threads.
void sample_thread(void*), webserver_thread(void*);
TaskHandle_t sample_thread_handle, webserver_thread_handle;

// Global persistent settings.
settings_t settings;

#include <proc.h>

AudioProcessor sound(AudioProcessor_init);
bool wifi_connected;

void loop(){}
void setup(){
	Serial.begin(115200);
	LittleFS.begin();
	sound.begin();

	setup_gpio();
	load_settings();
	wifi_connected = setup_wifi();

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
	// AudioProcessor_data_t data;
	// data = sound.get();

	DNSServer *dns;
	AsyncWebServer server(WIFI_WEBSERVER_PORT);

	if(!wifi_connected){
		dns = new DNSServer;
		dns->start(WIFI_DNS_PORT, "*", WIFI_AP_IP);
		MDNS.begin(WIFI_AP_NAME);
	}

	server.on("/wifi-set", HTTP_POST, [](AsyncWebServerRequest *req){
		strcpy(settings.ssid, req->arg("ssid").c_str());
		strcpy(settings.pass, req->arg("pass").c_str());
		settings.mqtt_broker_ip = ip_str_to_uint32(req->arg("mqtt-broker-ip").c_str());

		req->send(200);
		save_settings();

		ESP.restart();
	});

	server.onNotFound([](AsyncWebServerRequest *req){
		req->redirect("/config.html");
	});

	server.serveStatic(SETTINGS_FILE, LittleFS, SETTINGS_FILE);
	server.serveStatic("/", LittleFS, "/wifi-config/");
	server.begin();

	while(true){
		if(!wifi_connected)
			dns->processNextRequest();

		delay(1000);
	}
}
