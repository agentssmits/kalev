# Component makefile for extras/softuart

# expected anyone using this driver includes it as 'softuart/softuart.h'
INC_DIRS += $(mqtt_ROOT)..

# args for passing into compile rule generation
mqtt_SRC_DIR = $(mqtt_ROOT)

$(eval $(call component_compile_rules,mqtt))

