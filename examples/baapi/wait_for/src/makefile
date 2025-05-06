include ../makefile.inc

# TARGETS
TARGET = $(COMPONENT)-$(VERSION)

# Directories
SRCDIR = .
INCDIR_PRIV = ../include_priv
INCDIRS = $(INCDIR_PRIV) $(if $(STAGINGDIR), $(STAGINGDIR)/usr/include)
STAGING_LIBDIR = $(if $(STAGINGDIR), $(STAGINGDIR)/lib $(STAGINGDIR)/usr/lib)
OBJDIR = ../output/$(MACHINE)

# Files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))

# Compilation and linking flags
CFLAGS += -Werror -Wall -Wextra \
          -Wformat=2 -Wshadow -Wwrite-strings -Wredundant-decls \
          -Wno-unused-parameter -Wno-format-nonliteral \
          -g3 -Wmissing-declarations -Wmissing-declarations -pthread \
          $(addprefix -I,$(INCDIRS))

ifeq ($(CC_NAME),g++)
    CFLAGS += -std=c++2a
else
    CFLAGS += -Wstrict-prototypes -Wold-style-definition -Wnested-externs -std=c11
endif

LDFLAGS += $(addprefix -L,$(STAGING_LIBDIR)) -lamxc -lamxp -lamxb -lamxrt

# Targets
all: $(COMPONENT)

$(COMPONENT): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $<

$(OBJDIR)/:
	$(MKDIR) -p $@

clean:
	rm -rf $(OBJDIR) $(COMPONENT)

.PHONY: all clean

# Include dependency files
-include $(OBJECTS:.o=.d)
