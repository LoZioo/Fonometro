// Setup routines.
inline void setup_gpio(){
	pinMode(MIC_SUM, INPUT);
	pinMode(MIC_1, INPUT);
	pinMode(MIC_2, INPUT);
	pinMode(MIC_3, INPUT);
	pinMode(MIC_4, INPUT);
}

// Try to connect to WiFi, otherwise go into access point mode; return true if WiFi is connected, else return false.
bool setup_wifi(){
	WiFi.mode(WIFI_STA);
	WiFi.begin(settings.ssid, settings.pass);

	uint8_t attempts = 0;
	while(WiFi.status() != WL_CONNECTED && attempts < WIFI_STA_CONNECTION_ATTEMPTS){
		attempts++;
		delay(1000);
	}

	if(attempts == WIFI_STA_CONNECTION_ATTEMPTS){
		WiFi.mode(WIFI_AP);
		WiFi.softAP(WIFI_AP_NAME);
		WiFi.softAPConfig(WIFI_AP_IP, WIFI_AP_IP, WIFI_AP_SUBNET);

		return false;
	}

	return true;
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

inline void save_settings(){
	File file = LittleFS.open(SETTINGS_FILE, "w");

	file.write((uint8_t*) &settings, sizeof(settings));
	file.close();
}

inline void load_settings(){
	File file = LittleFS.open(SETTINGS_FILE, "r");

	if(file){
		file.read((uint8_t*) &settings, sizeof(settings));
		file.close();
	}
}

uint32_t ip_str_to_uint32(const char *ip){
	if(strcmp(ip, "") == 0)
		return 0;
	
	char tmp_ip[16];
	strcpy(tmp_ip, ip);

	char *octet = strtok(tmp_ip, ".");
	uint32_t res = 0;

	for(int i=0; i<4; i++){
		res <<= 8;
		res |= (uint8_t) atoi(octet);

		octet = strtok(NULL, ".");
	}

	return res;
}

char* uint32_to_ip_str(uint32_t ip, char* buf){
	uint8_t *octet = (uint8_t*) &ip;
	sprintf(buf, "%d.%d.%d.%d", octet[3], octet[2], octet[1], octet[0]);

	return buf;
}
