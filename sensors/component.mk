# Component makefile for extras/softuart

# expected anyone using this driver includes it as 'softuart/softuart.h'
INC_DIRS += $(sensors_ROOT)..

# args for passing into compile rule generation
sensors_SRC_DIR = $(sensors_ROOT)

$(eval $(call component_compile_rules,sensors))

