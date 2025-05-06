# Target
COMPONENT = phonebook
SOURCES = phonebook.c

# directories
OUTPUTDIR = ./output

# TARGETS
TARGET_SO = $(OUTPUTDIR)/$(COMPONENT).so

LDFLAGS += -shared -fPIC \
           -lamxc -lamxp -lamxd -lamxb -lamxo

# targets
all: $(TARGET_SO)

$(TARGET_SO): $(SOURCES) $(OUTPUTDIR)
	$(CC) -Wl,-soname,$(COMPONENT).so -o $@ $(SOURCES) $(LDFLAGS)

$(OUTPUTDIR):
	mkdir -p $@

clean:
	rm -rf $(OUTPUTDIR)

install: dir
	install phonebook.odl phonebook_definition.odl /etc/amx/phonebook
	install $(TARGET_SO) /usr/lib/amx/phonebook
	ln -sf /usr/bin/amxrt /usr/bin/phonebook

dir:
	mkdir -p /etc/amx/phonebook
	mkdir -p /usr/lib/amx/phonebook

uninstall:
	rm -rf /etc/amx/phonebook
	rm -rf /usr/lib/amx/phonebook
	rm -f /usr/bin/phonebook
