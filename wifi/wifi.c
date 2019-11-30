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

#include "../shmem/shmem.h"

#define AP_SSID "kalev"
#define AP_PSK "legolego"

#define TELNET_PORT 23

#define LWIP_SO_RCVTIMEO   1


enum SERVER_STATES {SERVER_INIT, WAIT_CONN, WRITE_CLIENT, READ_CLIENT, CLOSE_CONN};
enum SERVER_STATES serverState = SERVER_INIT;
typedef enum SERVER_RET {NEW_CLIENT, NO_CLIENT} serverRet;

struct netconn *client = NULL;
struct netconn *nc;
ip_addr_t client_addr;
err_t errCode;

void printErr(char* msg)
{
	if (errCode == ERR_OK)
		return;
	
	printf("%s failed with code %d: ", msg, errCode);
	switch (errCode){
		case ERR_MEM:
			printf("Out of memory!\n");
			break;
		case ERR_BUF:
			printf("Buffer error!\n");
			break;
		case ERR_TIMEOUT:
			printf("Timeout!\n");
			break;
		case ERR_RTE:
			printf("Routing problem!\n");
			break;
		case ERR_INPROGRESS:
			printf("Operation in progress!\n");
			break;
		case ERR_VAL:
			printf("Illegal value!\n");
			break;
		case ERR_WOULDBLOCK:
			printf("Operation would block!\n");
			break;
		case ERR_USE:
			printf("Address in use!\n");
			break;
		case ERR_ALREADY:
			printf("Already connecting!\n");
			break;
		case ERR_ISCONN:
			printf("Conn already established!\n");
			break;
		case ERR_CONN:
			printf("Not connected!\n");
			break;
		case ERR_IF:
			printf("Low-level netif error!\n");
			break;
		case ERR_ABRT:
			printf("Connection aborted!\n");
			break;
		case ERR_RST:
			printf("Connection reset!\n");
			break;
		case ERR_CLSD:
			printf("Connection closed!\n");
			break;
		case ERR_ARG:
			printf("Illegal argument!\n");
			break;
		default:
			printf("Unknown lwip err_t error code!\n");
			break;
	}
}

err_t initAP()
{
	struct ip_info ap_ip;
    IP4_ADDR(&ap_ip.ip, 192, 168, 0, 1);
    IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
    IP4_ADDR(&ap_ip.netmask, 255, 255, 255, 0);
    if (!sdk_wifi_set_ip_info(1, &ap_ip)){
		printf("sdk_wifi_set_ip_info failed!\n");
		return ERR_CONN;
	}
	
	if (!sdk_wifi_set_opmode(SOFTAP_MODE)){
		printf("sdk_wifi_set_opmode failed!\n");
		return ERR_CONN;
	}
	
	struct sdk_softap_config ap_config = {.ssid = AP_SSID, 
										  .ssid_hidden = 0, 
										  .channel = 3, 
										  .ssid_len = strlen(AP_SSID), 
										  .authmode = AUTH_WPA_WPA2_PSK, 
										  .password = AP_PSK, 
										  .max_connection = 3, 
										  .beacon_interval = 100,};
    if (!sdk_wifi_softap_set_config(&ap_config)){
		printf("sdk_wifi_softap_set_config failed!");
		return ERR_CONN;
	}

	return ERR_OK;
}

void startDHCP()
{
	ip_addr_t first_client_ip;
    IP4_ADDR(&first_client_ip, 192, 168, 0, 2);
    dhcpserver_start(&first_client_ip, 4);
}

err_t initServer()
{
	nc = netconn_new(NETCONN_TCP);
    if (!nc){
        printf("Status monitor: Failed to allocate socket.\r\n");
        return ERR_CONN;
    }
	
    errCode = netconn_bind(nc, IP_ANY_TYPE, TELNET_PORT);
	printErr("netconn_bind");
	
    errCode = netconn_listen(nc);
	printErr("netconn_bind");
	
	return errCode;
}

serverRet waitConn()
{
	errCode = netconn_accept(nc, &client);
	
	if (errCode != ERR_OK){
		printErr("netconn_accept");
		if (client)
			netconn_delete(client);
		return NO_CLIENT;
	} 

	uint16_t port_ignore;
	errCode = netconn_peer(client, &client_addr, &port_ignore);
	printErr("netconn_peer");
	
	netconn_set_nonblocking(client, true);
	netconn_set_recvtimeout(client, 1000);
	return NEW_CLIENT;
}

err_t writeClient()
{      
	static char buf[80];
	if (snprintf(buf, sizeof(buf), "%d,%ld\n", getWindowStatus(), getCO2()) < 0)
		printf("snprintf failed!");
	
	printf("write start...");
	size_t written = 0;
	size_t len = strlen(buf);
	
	TickType_t end = xTaskGetTickCount() + 5000*portTICK_PERIOD_MS;
	do {
		errCode = netconn_write_partly(client, buf, len, NETCONN_COPY, &written);
		if (errCode != ERR_OK) {
			printErr("netconn_write_partly");
			return errCode;
		}
		
		if (xTaskGetTickCount() > end) {
			printf("Write loop timeout!\n");
			break;
		}
	} while (written != len);
	
	//err_t retVal =  netconn_write(client, buf, strlen(buf), NETCONN_COPY);
	printf("end\n");
	//return retVal;
	return ERR_OK;
}

err_t readClient()
{	
 	static struct netbuf *buf = NULL;
	static char recvBuf[127];
	memset(recvBuf, '\0', 127);
	static uint8_t wouldBlockCount = 0;
	
	printf("read start...");
	errCode = netconn_recv(client, &buf);
	if (errCode == ERR_OK) {
		netbuf_copy(buf, recvBuf, 127);	
		printf("RX: %s", recvBuf);
		memset(recvBuf, '\0', 127);
		netbuf_delete(buf);
	}
	
	//This is workaround to ignore first occurance of ERR_WOULDBLOCK
	if (errCode == ERR_WOULDBLOCK)
		wouldBlockCount++;
		if (wouldBlockCount > 3) 
			wouldBlockCount = 0;
		else
			errCode = ERR_OK;
	
	
	printf("end with errcode %d\n", errCode);
	return errCode; 
}

void serverStateMachine()
{
	err_t status;
	switch (serverState) {
		case SERVER_INIT:
			if (initAP() != ERR_OK)
				break;
			startDHCP();
			if (initServer() != ERR_OK)
				break;
			serverState = WAIT_CONN;
			break;
		case WAIT_CONN:
			if (waitConn() == NEW_CLIENT)
				serverState = WRITE_CLIENT;
			break;
		case WRITE_CLIENT:
			if (writeClient() == ERR_OK)
				serverState = READ_CLIENT;
			else {
				printf("write failed!\n");
				serverState = CLOSE_CONN;
			}
			break;
		case READ_CLIENT:
			status = readClient();
			if (status == ERR_OK || status == ERR_TIMEOUT)
				serverState = WRITE_CLIENT;
			else {
				printf("read failed with %d!\n", status);
				serverState = CLOSE_CONN;
			}
			break;
		case CLOSE_CONN:
			printf("will close client!\n");
			netconn_delete(client);
			serverState = WAIT_CONN;
			break;
		default:
			printf("Unsupported state %d!\n", serverState);
			break;
	}
}