LOCAL_CFLAGS += -I../../include -I../../../libamxc/include
LOCAL_LDFLAGS += -L../../ -L../../../libamxc/

# Enable these if you dont want to install
#CFLAGS += LOCAL_CFLAGS
#LDFLAGS += LOCAL_LDFLAGS

CFLAGS += -g -O0 -Wall -Werror
LDFLAGS += -lamxc -lamxm -ldl

run: module.so executable
	valgrind --leak-check=full --error-exitcode=1 ./executable

executable: executable.o
	$(CC) -o $@ $^ $(LDFLAGS) 

module.so: module.o
	$(CC) -shared module.o -Wl,-soname,$(@) -o $@ $(LDFLAGS)

module.o: module.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<
	@$(CC) $(CFLAGS) -fPIC -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

executable.o: executable.c
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

clean:
	-rm -rf *.d *.o *.so executable

.PHONY: run clean