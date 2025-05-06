include ../test_defines.mk

TARGET = run_test
SOURCES += $(wildcard ./*.c) $(wildcard ../../src/*.c)
OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))

include ../test_targets.mk
