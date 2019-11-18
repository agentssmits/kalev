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

#endif