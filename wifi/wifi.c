/**
 * Very basic example showing usage of access point mode and the DHCP server.
 * The ESP in the example runs a telnet server on 172.16.0.1 (port 23) that
 * outputs some status information if you connect to it, then closes
 * the connection.
 *
 * This example code is in the public domain.
 */
#include <string.h>

#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <dhcpserver.h>

#include <lwip/api.h>

#define AP_SSID "kalev"
#define AP_PSK "legolego"

#define TELNET_PORT 23

enum SERVER_STATES {SERVER_INIT, WAIT_CONN, WRITE_CLIENT, READ_CLIENT};
enum SERVER_STATES serverState = SERVER_INIT;
typedef enum SERVER_RET {NEW_CLIENT, NO_CLIENT} serverRet;

struct netconn *client = NULL;
struct netconn *nc;
ip_addr_t client_addr;

void initAP()
{
	struct ip_info ap_ip;
    IP4_ADDR(&ap_ip.ip, 192, 168, 0, 1);
    IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
    IP4_ADDR(&ap_ip.netmask, 255, 255, 255, 0);
    sdk_wifi_set_ip_info(1, &ap_ip);
	
	sdk_wifi_set_opmode(SOFTAP_MODE);
	
	struct sdk_softap_config ap_config = { .ssid = AP_SSID, .ssid_hidden = 0, .channel = 3, .ssid_len = strlen(AP_SSID), .authmode =
            AUTH_WPA_WPA2_PSK, .password = AP_PSK, .max_connection = 3, .beacon_interval = 100, };
    sdk_wifi_softap_set_config(&ap_config);

}

void startDHCP()
{
	ip_addr_t first_client_ip;
    IP4_ADDR(&first_client_ip, 192, 168, 0, 2);
    dhcpserver_start(&first_client_ip, 4);
}

void initServer()
{
	nc = netconn_new(NETCONN_TCP);
    if (!nc)
    {
        printf("Status monitor: Failed to allocate socket.\r\n");
        return;
    }
    netconn_bind(nc, IP_ANY_TYPE, TELNET_PORT);
    netconn_listen(nc);
}

serverRet waitConn()
{
	err_t err = netconn_accept(nc, &client);

	if (err != ERR_OK)
	{
		if (client)
			netconn_delete(client);
		return NO_CLIENT;
	} 

	uint16_t port_ignore;
	netconn_peer(client, &client_addr, &port_ignore);
	return NEW_CLIENT;
}

void writeClient()
{      
	char buf[80];
	snprintf(buf, sizeof(buf), "Uptime %d seconds\r\n", xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
	netconn_write(client, buf, strlen(buf), NETCONN_COPY);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	snprintf(buf, sizeof(buf), "Free heap %d bytes\r\n", (int) xPortGetFreeHeapSize());
	netconn_write(client, buf, strlen(buf), NETCONN_COPY);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	char abuf[40];
	snprintf(buf, sizeof(buf), "Your address is %s\r\n\r\n", ipaddr_ntoa_r(&client_addr, abuf, sizeof(abuf)));
	netconn_write(client, buf, strlen(buf), NETCONN_COPY);
	vTaskDelay(500 / portTICK_PERIOD_MS);
	netconn_delete(client);
}

void serverStateMachine()
{
	switch (serverState) {
		case SERVER_INIT:
			initAP();
			startDHCP();
			initServer();
			serverState = WAIT_CONN;
			break;
		case WAIT_CONN:
			if (waitConn() == NEW_CLIENT)
				serverState = WRITE_CLIENT;
			break;
		case WRITE_CLIENT:
			serverState = READ_CLIENT;
			writeClient();
			serverState = WAIT_CONN; //temp!!
			break;
		case READ_CLIENT:
			serverState = WRITE_CLIENT;
			break;
		default:
			printf("Unsupported state %d!\n", serverState);
			break;
	}
}