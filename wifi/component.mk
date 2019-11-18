# Component makefile for extras/softuart

# expected anyone using this driver includes it as 'softuart/softuart.h'
INC_DIRS += $(wifi_ROOT)..

# args for passing into compile rule generation
wifi_SRC_DIR = $(wifi_ROOT)

$(eval $(call component_compile_rules,wifi))

