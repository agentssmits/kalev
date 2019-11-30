#include "shmem.h"
#include <FreeRTOS.h>
#include <semphr.h>

struct SHMEM shMem;

SemaphoreHandle_t xMutex;

void initShmem()
{	
	xMutex = xSemaphoreCreateMutex();
	
	shMem.CO2 = -1;
	shMem.temperature = -1;
	shMem.pressure = -1;
	shMem.humidity = -1;
	shMem.windowStatus = 2;
}

long int getCO2()
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	long int retVal = shMem.CO2;
	xSemaphoreGive(xMutex);
	return retVal;
}

void setCO2(long int CO2)
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	shMem.CO2 = CO2;
	xSemaphoreGive(xMutex);
}

void setWindowStatus(uint8_t status)
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	shMem.windowStatus = status;
	xSemaphoreGive(xMutex);
}

uint8_t getWindowStatus()
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	uint8_t retVal = shMem.windowStatus;
	xSemaphoreGive(xMutex);
	return retVal;
}