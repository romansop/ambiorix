include ../test_defines.mk

TARGET = run_test
SOURCES += $(wildcard ./*.c)  ../../src/amxb_rbus_to_var.c
OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))

include ../test_targets.mk
