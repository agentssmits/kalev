#include "mqtt.h"
#include "../shmem/shmem.h"
#include "../credentials.h"
#include "../globals.h"
#include "../logging/logging.h"

#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>

#include <paho_mqtt_c/MQTTESP8266.h>
#include <paho_mqtt_c/MQTTClient.h>

QueueHandle_t dataPublishQueue;
QueueHandle_t statusPublishQueue;

static const char *  get_my_id(void)
{
    // Use MAC address for Station as unique ID
    static char my_id[13];
    static bool my_id_done = false;
    int8_t i;
    uint8_t x;
    if (my_id_done)
        return my_id;
    if (!sdk_wifi_get_macaddr(STATION_IF, (uint8_t *)my_id))
        return NULL;
    for (i = 5; i >= 0; --i)
    {
        x = my_id[i] & 0x0F;
        if (x > 9) x += 7;
        my_id[i * 2 + 1] = x + '0';
        x = my_id[i] >> 4;
        if (x > 9) x += 7;
        my_id[i * 2] = x + '0';
    }
    my_id[12] = '\0';
    my_id_done = true;
    return my_id;
}

int publish(const char* topic, QueueHandle_t* queue, mqtt_client_t* client)
{
	char msg[MQTT_MSG_LEN - 1] = "\0";
	int ret = MQTT_SUCCESS;
	while(xQueueReceive(*queue, (void *)msg, 0) == pdTRUE){
		mqtt_message_t message;
		message.payload = msg;
		message.payloadlen = MQTT_MSG_LEN;
		message.dup = 0;
		message.qos = MQTT_QOS1;
		message.retained = 0;
		ret = mqtt_publish(client, topic, &message);
		if (ret != MQTT_SUCCESS) {
			logStatus("message publish err: %d", ret );
			break;
		}
	}
	
	return ret;
}

void  mqtt_task(void *pvParameters)
{
	xSemaphoreTake(wifi_alive, portMAX_DELAY);
    int ret = 0;
    struct mqtt_network network;
    mqtt_client_t client   = mqtt_client_default;
    char mqtt_client_id[20];
    uint8_t mqtt_buf[100];
    uint8_t mqtt_readbuf[100];
	uint8_t failCount = 0;
    mqtt_packet_connect_data_t data = mqtt_packet_connect_data_initializer;

    memset(mqtt_client_id, 0, sizeof(mqtt_client_id));
    strcpy(mqtt_client_id, "ESP-");
    strcat(mqtt_client_id, get_my_id());

    while(1) {
		printf("Waiting semaphore\n");
        xSemaphoreTake(wifi_alive, portMAX_DELAY);
		mqtt_network_new( &network );
        printf("%s: (Re)connecting to MQTT server %s ... ",__func__, MQTT_HOST);
        ret = mqtt_network_connect(&network, MQTT_HOST, MQTT_PORT);
        if( ret ){
            logStatus("mqtt netw err: %d", ret);
            taskYIELD();
            continue;
        }
        printf("done\n\r");
        mqtt_client_new(&client, &network, 5000, mqtt_buf, 100,
                      mqtt_readbuf, 100);

        data.willFlag       = 0;
        data.MQTTVersion    = 3;
        data.clientID.cstring   = mqtt_client_id;
        data.username.cstring   = MQTT_USER;
        data.password.cstring   = MQTT_PASS;
        data.keepAliveInterval  = 10;
        data.cleansession   = 0;
        printf("Send MQTT connect ... ");
        ret = mqtt_connect(&client, &data);
        if(ret){
            logStatus("mqtt conn err: %d", ret);
            mqtt_network_disconnect(&network);
			
			sdk_wifi_station_disconnect();
			failCount++;
			if (failCount > 5)
				sdk_system_restart();
            taskYIELD();
            continue;
        }
        printf("done\r\n");
		failCount = 0;
        xQueueReset(dataPublishQueue);
		xQueueReset(statusPublishQueue);

        while(1){
			ret = publish(DATA_TOPIC, &dataPublishQueue, &client);
			if (ret != MQTT_SUCCESS ){
				break;
			}
			
			ret = publish(STS_TOPIC, &statusPublishQueue, &client);
			if (ret != MQTT_SUCCESS ){
				break;
			}
            ret = mqtt_yield(&client, 1000);
            if (ret == MQTT_DISCONNECTED)
                break;
        }
        logStatus("Conn dropped, request restart");
        mqtt_network_disconnect(&network);
        taskYIELD();
    }
}

void mqttPublishData()
{
	if (xQueueSend(dataPublishQueue, (void *)mqttMsg, 0) == pdFALSE) 
		logStatus("data queue overflow");
}

void mqttPublishStatus(char* buf)
{
	if (xQueueSend(statusPublishQueue, (void *)buf, 0) == pdFALSE) 
		printf("sts queue overflow");
}
