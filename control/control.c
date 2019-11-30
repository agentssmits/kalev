#include "control.h"
#include "../shmem/shmem.h"

void windowControl()
{
	long int CO2 = getCO2();
	if (CO2 >= tresholdON)
		setWindowStatus(1);
	else if (CO2 <= tresholdOFF)
		setWindowStatus(0);
}