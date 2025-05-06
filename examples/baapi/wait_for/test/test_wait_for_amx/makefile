include ../test_defines.mk

TARGET = run_test
SOURCES += $(wildcard ./*.c)
OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o))) 

LDFLAGS += -g \
           -Wl,--wrap=amxrt_connect \
		   -Wl,--wrap=amxb_wait_for_object \

		   
include ../test_targets.mk
