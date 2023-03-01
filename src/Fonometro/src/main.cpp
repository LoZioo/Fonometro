#include <Arduino.h>
#include <WiFi.h>

#include <LittleFS.h>

#include <DNSServer.h>
#include <ESPmDNS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <PubSubClient.h>

// #include <fft.h>
#include <AudioProcessor.h>
#include <const.h>

// Threads.
void sample_thread(void*), main_thread(void*);
TaskHandle_t sample_thread_handle, main_thread_handle;

// Global persistent settings.
settings_t settings;

// MQTT.
WiFiClient mqtt_client;
PubSubClient mqtt(mqtt_client);

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

	randomSeed(micros());
	spawn_threads();
}

void sample_thread(void *parameters){
	Serial.println("sample_thread");

	while(true){
		sound.evaluate();
		delay(SENDING_DATA_PERIOD_MS / 4);
	}
}

void main_thread(void *parameters){
	Serial.println("main_thread");

	DNSServer *dns;
	AsyncWebServer server(WIFI_WEBSERVER_PORT);

	if(!wifi_connected){
		dns = new DNSServer;
		dns->start(WIFI_DNS_PORT, "*", WIFI_AP_IP);
		MDNS.begin(WIFI_AP_NAME);
	}

	else
		setup_mqtt();

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

	// Non blocking timer.
	uint64_t t0 = 0;

	// Temp data.
	AudioProcessor_data_t data;
	char str[32];

	while(true){
		if(!wifi_connected)
			dns->processNextRequest();

		else{
			if(!mqtt.connected()){
				// Create a random client ID.
				String id = MQTT_NAME_PREFIX;
				id += String(random(0xFFFF), HEX);

				//Attempt to connect.
				mqtt.connect(id.c_str());
				delay(1000);
			}

			else{
				if(millis() > t0){
					data = sound.get();

					sprintf(str, "%d", data.sample_dc_offset);
					mqtt.publish(MQTT_SAMPLE_DC_OFFSET_TOPIC, str);

					sprintf(str, "%.6e", data.adc_voltage_rms);
					mqtt.publish(MQTT_ADC_VOLTAGE_RMS_TOPIC, str);

					sprintf(str, "%.6e", data.mic_voltage_rms);
					mqtt.publish(MQTT_MIC_VOLTAGE_RMS_TOPIC, str);

					sprintf(str, "%d", data.db_spl);
					mqtt.publish(MQTT_DB_SPL_TOPIC, str);

					t0 = millis() + SENDING_DATA_PERIOD_MS;
				}

				mqtt.loop();
			}
		}
	}
}
