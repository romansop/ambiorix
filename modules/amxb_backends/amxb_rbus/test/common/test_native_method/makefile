# build destination directories
# TARGETS
TARGET = test_native_method

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

# compilation and linking flags
CFLAGS += -I /usr/include/rbus/ -Werror -Wall -Wextra -g3

LDFLAGS += -lrbus -pthread

# targets
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

-include $(OBJECTS:.o=.d)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

clean:
	rm -rf ./*.o ./*.d $(TARGET)

.PHONY: all clean
