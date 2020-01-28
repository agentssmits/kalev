#include "logging.h"
#include "../mqtt/mqtt.h"
//#include "../globals.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

char stsBuf[36];

void logStatus(char* format, ...)
{
	char tempBuf[30];
 	va_list args;
	va_start(args, format);
	memset(stsBuf, 0, 36);
	if (vsprintf(tempBuf, format, args) < 0)
		printf("vsprintf @ logging.c failed!");
	va_end(args); 
	sprintf(stsBuf, "STS,%s\r\n", tempBuf);
	printf("%s", stsBuf);
	mqttPublishStatus(stsBuf);
}
