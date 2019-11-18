# Component makefile for extras/softuart

# expected anyone using this driver includes it as 'softuart/softuart.h'
INC_DIRS += $(shmem_ROOT)..

# args for passing into compile rule generation
shmem_SRC_DIR = $(shmem_ROOT)

$(eval $(call component_compile_rules,shmem))

