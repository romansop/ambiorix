# the processor architecture
MACHINE ?= $(shell $(CC) -dumpmachine)

# Target
COMPONENT = phonebook

# build destination directories
OUTPUTDIR = ./output/$(MACHINE)
OBJDIR = $(OUTPUTDIR)/object

# TARGETS
TARGET_SO = $(OBJDIR)/$(COMPONENT).so

# sources
SRCDIR = .
INCDIR = .
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))

CFLAGS += -Wall -Wextra \
          -Wformat=2 -Wshadow \
          -Wwrite-strings -Wredundant-decls \
          -Wmissing-declarations -Wno-attributes \
          -Wno-format-nonliteral \
          -fPIC -g3 $(addprefix -I ,$(INCDIR))

LDFLAGS += -shared -fPIC \
           -lamxc \
           -lamxp \
           -lamxd \
           -lamxb \
           -lamxo

# targets
all: $(TARGET_SO)

$(TARGET_SO): $(OBJECTS)
	$(CC) -Wl,-soname,$(COMPONENT).so -o $@ $(OBJECTS) $(LDFLAGS)

-include $(OBJECTS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

$(OBJDIR)/:
	mkdir -p $@

install: all
	install -D -p -m 0644 $(OBJDIR)/$(COMPONENT).so .

clean:
	rm -rf ./output/ $(COMPONENT).so
