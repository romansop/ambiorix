# TARGETS
TARGET = remote-test-plugin

# files
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

# compilation and linking flags
CFLAGS += -Werror -Wall -Wextra -pthread -g3

LDFLAGS += -lamxc -lamxp -lamxj -lamxd -lamxo -lamxb -lamxrt

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
