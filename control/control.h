#ifndef _CONTROL_H
#define _CONTROL_H

#define CO2Treshold 2500
#define hysteresisWidth 10

#define tresholdON	CO2Treshold+hysteresisWidth
#define tresholdOFF CO2Treshold-hysteresisWidth

void windowControl();

#endif