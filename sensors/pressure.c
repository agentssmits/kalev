#include <stdio.h>

#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "i2c/i2c.h"
#include "bmp280/bmp280.h"

#include "shmem/shmem.h"
#include "mqtt/mqtt.h"
#include "../globals.h"

char mqttMsg[MQTT_MSG_LEN];

const uint8_t i2c_bus = 0;
const uint8_t scl_pin = 2;
const uint8_t sda_pin = 0;
bmp280_params_t  params;
bmp280_t bmp280_dev;
	
void initBmp280()
{
	i2c_init(i2c_bus, scl_pin, sda_pin, I2C_FREQ_400K);
	bmp280_init_default_params(&params);
	bmp280_dev.i2c_dev.bus = i2c_bus;
    bmp280_dev.i2c_dev.addr = BMP280_I2C_ADDRESS_0;
	
	uint8_t timeout = 0;
	while (!bmp280_init(&bmp280_dev, &params)) {
		printf("BMP280 initialization failed\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		if (++timeout > 10) {
			printf("BME280 initialization timeout!\n");
			return;
		}
	}
}

void getBmp280()
{
    float temperature, humidity, pressure;

	vTaskDelay(1000 / portTICK_PERIOD_MS);
	if (!bmp280_read_float(&bmp280_dev, &temperature, &pressure, &humidity)) {
		printf("Temperature/pressure reading failed\n");
		return;
	}
	
	setPressure(temperature, humidity, pressure);
	memset(mqttMsg, 0, MQTT_MSG_LEN);
	sprintf(mqttMsg, "BME,%.2f,%.2f,%.2f\r\n", temperature, humidity, pressure);
	mqttPublish();
	//printf("Pressure: %.2f Pa, Temperature: %.2f C, Humidity: %.2f\n", pressure, temperature, humidity);
}