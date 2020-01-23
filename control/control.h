#ifndef _CONTROL_H
#define _CONTROL_H

#define CO2Treshold 2000
#define hysteresisWidth 300

//#define tresholdON	CO2Treshold+hysteresisWidth
//#define tresholdOFF CO2Treshold-hysteresisWidth
#define tresholdON	CO2Treshold
#define tresholdOFF CO2Treshold-hysteresisWidth

void windowControl();

#endif