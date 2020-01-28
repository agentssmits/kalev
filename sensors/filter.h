#ifndef _FILTER_H
#define _FILTER_H

#define medFilterWindow 3

void initFilterCO2(unsigned long currentCO2);
void initFilterBME180(float currentTemp, float currentHumidity, float currentPressure);

unsigned long filterCO2(unsigned long currentCO2);
float filterTemp(float currentTemp);
float filterHumidity(float currentHumidity);
float filterPressure(float currentPressure);

#endif