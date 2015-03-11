#include "mqtt.h"
#include "osapi.h"
#include "espmissingincludes.h"
#include "mem.h"
#include "user_interface.h"

MQTT_Client client;

void ICACHE_FLASH_ATTR mqttConnectedCB (uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	MQTT_Publish (client, (const char *)"test/topic", (const char *)"helloESP", 8, 0,0);
	MQTT_Publish (client, (const char *)"test/topic", (const char *)"helloESP", 8, 1,0);
	MQTT_Publish (client, (const char *)"test/topic", (const char *)"helloESP", 8, 2,0);
	os_printf ("MQTT Connected");
}

void ICACHE_FLASH_ATTR mqttDataCB(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	char *topicBuf = (char*)os_zalloc(topic_len+1),
		*dataBuf = (char*)os_zalloc(data_len+1);

	MQTT_Client* client = (MQTT_Client*)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	os_printf ("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
	os_free(topicBuf);
	os_free(dataBuf);
}

void ICACHE_FLASH_ATTR mqttDisconnectedCB (uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	os_printf ("MQTT Disconnected");
}

static ETSTimer mqttInitTimer; 

static void ICACHE_FLASH_ATTR mqttTimerCB (void * arg)
{
	//check wifi status connection
	int status = wifi_station_get_connect_status();
	//if wifi connection OK: connect MQTT and disarm timer
	if (status == STATION_GOT_IP)
	{
		MQTT_Connect (&client);
		os_timer_disarm (&mqttInitTimer);
	} 
	//if not connected: disconnect MQTT
	else 
	{
		MQTT_Disconnect (&client);
	}		
}

void mqtt_init (void)
{
	MQTT_InitConnection (&client, (const char *)"192.168.58.133", 1883,0);
	MQTT_InitClient (&client, (const char *)"ESP8266", (const char *)"", (const char *)"",30,1);
	MQTT_OnConnected (&client, mqttConnectedCB);
	MQTT_OnDisconnected(&client, mqttDisconnectedCB);
	MQTT_OnPublished (&client, NULL);
	MQTT_OnData(&client, mqttDataCB);
	os_timer_disarm(&mqttInitTimer);
	os_timer_setfn(&mqttInitTimer, mqttTimerCB, NULL);
	os_timer_arm(&mqttInitTimer, 1000, 1);
	os_printf ("MQTT Started");
}
