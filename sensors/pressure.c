#include <stdio.h>
#include <string.h>

#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "i2c/i2c.h"
#include "bmp280/bmp280.h"

#include "shmem/shmem.h"
#include "mqtt/mqtt.h"
#include "../globals.h"
#include "filter.h"

char mqttMsg[MQTT_MSG_LEN];

const uint8_t i2c_bus = 0;
const uint8_t scl_pin = 2;
const uint8_t sda_pin = 0;
bmp280_params_t  params;
bmp280_t bme180_dev;
	
void initBME180()
{
	i2c_init(i2c_bus, scl_pin, sda_pin, I2C_FREQ_400K);
	bmp280_init_default_params(&params);
	bme180_dev.i2c_dev.bus = i2c_bus;
    bme180_dev.i2c_dev.addr = BMP280_I2C_ADDRESS_0;
	
	uint8_t timeout = 0;
	while (!bmp280_init(&bme180_dev, &params)) {
		logStatus("BME180 init failed");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		if (++timeout > 10) {
			logStatus("BME180 init timeout");
			return;
		}
	}
}

void getBME180()
{
    float temperature, humidity, pressure;
	static uint8_t callCount = 0;

	vTaskDelay(1000 / portTICK_PERIOD_MS);
	if (!bmp280_read_float(&bme180_dev, &temperature, &pressure, &humidity)) {
		logStatus("BME180 read failed");
		return;
	}
	
	// init median filter if it is first call
	if (callCount == 0) {
		initFilterBME180(temperature, humidity, pressure);
		callCount++;
		return;
	}
	
	temperature = filterTemp(temperature);
	humidity = filterHumidity(humidity);
	pressure = filterPressure(pressure);
	
	setPressure(temperature, humidity, pressure);
	memset(mqttMsg, 0, MQTT_MSG_LEN);
	sprintf(mqttMsg, "BME,%.2f,%.2f,%.2f\r\n", temperature, humidity, pressure);
	mqttPublishData();
	//printf("Pressure: %.2f Pa, Temperature: %.2f C, Humidity: %.2f\n", pressure, temperature, humidity);
}