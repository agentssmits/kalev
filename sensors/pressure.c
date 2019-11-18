#include <stdio.h>

#include "espressif/esp_common.h"
#include "esp/uart.h"

#include "FreeRTOS.h"
#include "task.h"

#include "i2c/i2c.h"
#include "bmp280/bmp280.h"

// In forced mode user initiate measurement each time.
// In normal mode measurement is done continuously with specified standby time.
// #define MODE_FORCED
const uint8_t i2c_bus = 0;
const uint8_t scl_pin = 2;
const uint8_t sda_pin = 0;

bmp280_params_t  params;
bmp280_t bmp280_dev;

#ifdef MODE_FORCED
static void bmp280_task_forced(void *pvParameters)
{
    bmp280_params_t  params;
    float pressure, temperature, humidity;

    bmp280_init_default_params(&params);
    params.mode = BMP280_MODE_FORCED;

    bmp280_t bmp280_dev;
    bmp280_dev.i2c_dev.bus = i2c_bus;
    bmp280_dev.i2c_dev.addr = BMP280_I2C_ADDRESS_0;

    while (1) {
        while (!bmp280_init(&bmp280_dev, &params)) {
            printf("BMP280 initialization failed\n");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        bool bme280p = bmp280_dev.id == BME280_CHIP_ID;
        printf("BMP280: found %s\n", bme280p ? "BME280" : "BMP280");

        while(1) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            if (!bmp280_force_measurement(&bmp280_dev)) {
                printf("Failed initiating measurement\n");
                break;
            }
            // wait for measurement to complete
            while (bmp280_is_measuring(&bmp280_dev)) {};

            if (!bmp280_read_float(&bmp280_dev, &temperature, &pressure, &humidity)) {
                printf("Temperature/pressure reading failed\n");
                break;
            }
            printf("Pressure: %.2f Pa, Temperature: %.2f C", pressure, temperature);
            if (bme280p)
                printf(", Humidity: %.2f\n", humidity);
            else
                printf("\n");
        }
    }
}
#else
	
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
    float pressure, temperature, humidity;

	vTaskDelay(1000 / portTICK_PERIOD_MS);
	if (!bmp280_read_float(&bmp280_dev, &temperature, &pressure, &humidity)) {
		printf("Temperature/pressure reading failed\n");
		return;
	}
	
	printf("Pressure: %.2f Pa, Temperature: %.2f C, Humidity: %.2f\n", pressure, temperature, humidity);

}
#endif