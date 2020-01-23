#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <semphr.h>

#define MQTT_MSG_LEN 30

extern SemaphoreHandle_t wifi_alive;
extern QueueHandle_t publish_queue;
extern char mqttMsg[MQTT_MSG_LEN];

#endif