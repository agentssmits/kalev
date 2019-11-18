#include "shmem.h"
#include <FreeRTOS.h>

struct SHMEM shMem;

void initShmem()
{	
	shMem.CO2 = -1;
	shMem.temperature = -1;
	shMem.pressure = -1;
	shMem.humidity = -1;
	shMem.windowStatus = 2;
}