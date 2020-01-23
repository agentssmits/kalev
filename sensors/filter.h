#ifndef _FILTER_H
#define _FILTER_H

#define medFilterWindow 3

void initFilterCO2(unsigned long currentCO2);
void initFilterTemp(float currentTemp);
void initFilterHumidity(float currentHumidity);
void initFilterPressure(float currentPressure);

unsigned long filterCO2(unsigned long currentCO2);
float filterTemp(unsigned long currentTemp);
float filterHumidity(unsigned long currentHumidity);
float filterPressure(unsigned long currentPressure);

#endif