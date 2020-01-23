PROGRAM = kalev
EXTRA_COMPONENTS = 	extras/softuart extras/i2c extras/bmp280 extras/dhcpserver extras/paho_mqtt_c \
					sensors wifi shmem control mqtt
ESPBAUD = 460800
include common.mk
