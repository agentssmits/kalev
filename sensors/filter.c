#include "filter.h"

long int CO2Array[medFilterWindow];
float tempArray[medFilterWindow];
float humidityArray[medFilterWindow];
float pressureArray[medFilterWindow];

void initMedianFilter(float currentVal, float medArray[])
{
	medArray[1] = currentVal;
	medArray[2] = currentVal;
}

float medianFilter(float currentVal, float medArray[])
{
  medArray[0] = medArray[1]; // shifting left
  medArray[1] = medArray[2]; // shifting left
  medArray[2] = currentVal;
    
  if (medArray[0] <= medArray[1] && medArray[0] <= medArray[2]) {
    if (medArray[1] <= medArray[2])
		return medArray[1];
    else
		return medArray[2];
  } else if (medArray[1] <= medArray[0] && medArray[1] <= medArray[2]) {
    if (medArray[0] <= medArray[2])
		return medArray[0];
    else
		return medArray[2];
  } else {
    if(medArray[0] <= medArray[1])
		return medArray[0];
    else
		return medArray[1];
  }
}

void initFilterCO2(unsigned long currentCO2)
{
	initMedianFilter((float)currentCO2, (float*)CO2Array);
}

void initFilterTemp(float currentTemp)
{
	initMedianFilter(currentTemp, tempArray);
}

void initFilterHumidity(float currentHumidity)
{
	initMedianFilter(currentHumidity, humidityArray);
}

void initFilterPressure(float currentPressure)
{
	initMedianFilter(currentPressure, pressureArray);
}

unsigned long filterCO2(unsigned long currentCO2)
{
	return (unsigned long)medianFilter((float)currentCO2, (float*)CO2Array);
}

float filterTemp(unsigned long currentTemp)
{
	return medianFilter(currentTemp, tempArray);
}

float filterHumidity(unsigned long currentHumidity)
{
	return medianFilter(currentHumidity, humidityArray);
}

float filterPressure(unsigned long currentPressure)
{
	return medianFilter(currentPressure, pressureArray);
}

