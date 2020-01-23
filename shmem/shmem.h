#ifndef _SHMEM_H
#define _SHMEM_H

#include <FreeRTOS.h>

struct SHMEM {
	long int CO2;
	float temperature;
	float pressure;
	float humidity;
	uint8_t windowStatus;
};

void initShmem();
void setCO2(long int CO2);
void setPressure(float temperature, float humidity, float pressure);
long int getCO2();
void setWindowStatus(uint8_t status);
uint8_t getWindowStatus();

#endif