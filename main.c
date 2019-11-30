/*
 * Softuart example
 *
 * Copyright (C) 2017 Ruslan V. Uss <unclerus@gmail.com>
 * Copyright (C) 2016 Bernhard Guillon <Bernhard.Guillon@web.de>
 * Copyright (c) 2015 plieningerweb
 *
 * MIT Licensed as described in the file LICENSE
 */
#include <esp/gpio.h>
#include <esp/uart.h>
#include <espressif/esp_common.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>

#include "sensors/sensair.h"
#include "sensors/pressure.h"
#include "wifi/wifi.h"
#include "shmem/shmem.h"
#include "control/control.h"

static void co2_task(void *pvParameters)
{
	initCO2();

	while (1) {
		long int co2 = requestCO2();
		printf("CO2: %ld\n", co2);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}

static void bmp280_task_normal(void *pvParameters)
{
	initBmp280();
	
	while (1) {
		getBmp280();
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}

static void serverTask(void *pvParameters)
{

	while (1) {
		serverStateMachine();
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static void controlTask(void *pvParameters)
{

	while (1) {
		windowControl();
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}

void user_init(void)
{
    // setup real UART for now
    uart_set_baud(0, 115200);
	printf("\n");
    printf("SDK version:%s\n\n", sdk_system_get_sdk_version());
	printf("GIT version : %s\n", GITSHORTREV);
	
	initShmem();

    xTaskCreate(bmp280_task_normal, "bmp280_task", 256, NULL, 5, NULL);
	xTaskCreate(co2_task, "co2_task", 256, NULL, 5, NULL);
	xTaskCreate(serverTask, "serverTask", 512, NULL, 4, NULL);
	xTaskCreate(controlTask, "controlTask", 256, NULL, 2, NULL);
}
