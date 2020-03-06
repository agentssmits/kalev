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

#include "globals.h"
#include "sensors/sensair.h"
#include "sensors/pressure.h"
#include "wifi/wifi.h"
#include "shmem/shmem.h"
#include "control/control.h"
#include "mqtt/mqtt.h"
#include "logging/logging.h"

static void co2_task(void *pvParameters)
{
	initCO2();

	while (1) {
		while (requestCO2() == 0){};
		vTaskDelay(60000 / portTICK_PERIOD_MS);
	}
}

static void bme180_task_normal(void *pvParameters)
{
	initBME180();
	
	while (1) {
		getBME180();
		vTaskDelay(60000 / portTICK_PERIOD_MS);
	}
}

static void serverTask(void *pvParameters)
{

	while (1) {
		serverStateMachine();
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static void staTask(void *pvParameters)
{

	while (1) {
		stationStateMachine();
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static void controlTask(void *pvParameters)
{

	while (1) {
		windowControl();
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void user_init(void)
{
	gpio_enable(R_PIN, GPIO_OUTPUT);
	gpio_enable(G_PIN, GPIO_OUTPUT);
	gpio_enable(B_PIN, GPIO_OUTPUT);
	gpio_write(R_PIN, 0);
	gpio_write(G_PIN, 0);
	gpio_write(B_PIN, 1);
    uart_set_baud(0, 115200);
	printf("\n");
	
	vSemaphoreCreateBinary(wifi_alive);
	dataPublishQueue = xQueueCreate(10, MQTT_MSG_LEN);
	statusPublishQueue = xQueueCreate(10, MQTT_MSG_LEN);
	
	logStatus("System started!");
    logStatus("SDK version:%s", sdk_system_get_sdk_version());
	logStatus("GIT version : %s", GITSHORTREV);
	
	initShmem();

    xTaskCreate(bme180_task_normal, "bmp280_task", 456, NULL, 5, NULL);
	xTaskCreate(co2_task, "co2_task", 256, NULL, 5, NULL);
	xTaskCreate(serverTask, "serverTask", 512, NULL, 5, NULL);
	xTaskCreate(staTask, "staTask", 512, NULL, 5, NULL);
    xTaskCreate(&mqtt_task, "mqtt_task", 1024, NULL, 4, NULL);
	xTaskCreate(controlTask, "controlTask", 256, NULL, 1, NULL);
}
