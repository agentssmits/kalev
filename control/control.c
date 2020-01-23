#include "control.h"
#include "../shmem/shmem.h"
#include "../mqtt/mqtt.h"
#include "../globals.h"

void windowControl()
{
	static uint8_t prevWindowState = 0;
	long int CO2 = getCO2();
	if (CO2 >= tresholdON) {
		setWindowStatus(1);
		if (prevWindowState != 1) {
			memset(mqttMsg, 0, MQTT_MSG_LEN);
			sprintf(mqttMsg, "EV,Win open\r\n");
			mqttPublish();
			prevWindowState = 1;
		}
	}
	else if (CO2 <= tresholdOFF) {
		setWindowStatus(0);
		if (prevWindowState != 0) {
			memset(mqttMsg, 0, MQTT_MSG_LEN);
			sprintf(mqttMsg, "EV,Win close\r\n");
			mqttPublish();
			prevWindowState = 0;
		}
	}
}