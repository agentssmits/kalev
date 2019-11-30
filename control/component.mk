# Component makefile for extras/softuart

# expected anyone using this driver includes it as 'softuart/softuart.h'
INC_DIRS += $(control_ROOT)..

# args for passing into compile rule generation
control_SRC_DIR = $(control_ROOT)

$(eval $(call component_compile_rules,control))

