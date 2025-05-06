include ../test_defines.mk

TARGET = run_test
SOURCES += $(wildcard ./*.c) ../mocks.c
OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))

LDFLAGS += -g -Wl,--wrap=initscr \
              -Wl,--wrap=start_color \
			  -Wl,--wrap=cbreak \
			  -Wl,--wrap=keypad \
			  -Wl,--wrap=noecho \
			  -Wl,--wrap=resize_term \
			  -Wl,--wrap=mvwprintw \
			  -Wl,--wrap=amxt_read \
			  -Wl,--wrap=ioctl \

include ../test_targets.mk
