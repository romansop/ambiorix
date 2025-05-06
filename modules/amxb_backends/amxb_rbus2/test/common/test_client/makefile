# build destination directories
# TARGETS
TARGET_SO = test_client.so

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

# compilation and linking flags
CFLAGS += -Werror -Wall -Wextra -fPIC -g3

LDFLAGS += -shared -fPIC -lamxc -lamxp -lamxd -lamxo

# targets
all: $(TARGET_SO)

$(TARGET_SO): $(OBJECTS)
	$(CC) -Wl,-soname,$@ $(LDFLAGS) -o $@ $(OBJECTS)

-include $(OBJECTS:.o=.d)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

clean:
	rm -rf ./*.o ./*.d $(TARGET_SO)

.PHONY: all clean
