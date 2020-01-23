#include "sensair.h"

#include <softuart/softuart.h>
#include <espressif/esp_common.h>
#include <FreeRTOS.h>
#include <task.h>

#include "mqtt/mqtt.h"
#include "../globals.h"
#include "shmem/shmem.h"

#define RX_PIN 4
#define TX_PIN 5

#define co2RequestLen	8
#define co2ResponseLen	7

char co2RequestArr[] = {0xFE, 0x04, 0x00, 0x03, 0x00, 0x01, 0xD5, 0xC5}; //CO2 read sequence
char co2ResponseArr[co2ResponseLen];

void softuart_put_array(char array[], uint8_t size)
{
	uint8_t i;
	for (i = 0; i < size; i++) {
		softuart_put(0, array[i]);
		sdk_os_delay_us(100);
	}
}

void clearArr(char array[], uint8_t size)
{
	uint8_t i;
	for (i = 0; i < size; i++)
		array[i] = 0;
}

char initCO2()
{
	// setup software uart to 9600 8n1
    return softuart_open(0, 9600, RX_PIN, TX_PIN);
}

unsigned long requestCO2()
{
	unsigned int timeout;
	softuart_put_array(co2RequestArr, co2RequestLen);
	sdk_os_delay_us(10000); 

	clearArr(co2ResponseArr, co2RequestLen);
	
	for (int i=0; i < co2ResponseLen; i++){
		timeout = 0;                       //set a timeout counter
		while (!softuart_available(0)) {
			timeout++;
			sdk_os_delay_us(10000);  //set a timeout counter
			if (timeout > 10) {//if it takes too long there was probably an error
				while(softuart_available(0))               //flush whatever we have
					softuart_read(0);
				printf("CO2 sensor read timeout!\n");
				return 0;  //exit and try again later
			}
		};
		co2ResponseArr[i] = softuart_read(0);
	}
	
	int high = co2ResponseArr[3];                         //high byte for value is 4th byte in packet in the packet
	int low = co2ResponseArr[4];                          //low byte for value is 5th byte in the packet
	long int retVal = high*256 + low;             //Combine high byte and low byte with this formula to get value
	setCO2(retVal);
	memset(mqttMsg, 0, MQTT_MSG_LEN);
	sprintf(mqttMsg, "CO2,%ld\r\n", retVal);
	mqttPublish();
	return retVal;
}