# Component makefile for extras/softuart

# expected anyone using this driver includes it as 'softuart/softuart.h'
INC_DIRS += $(logging_ROOT)..

# args for passing into compile rule generation
logging_SRC_DIR = $(logging_ROOT)

$(eval $(call component_compile_rules,logging))

