include ../makefile.inc

# build destination directories
OBJDIR = ../output/$(MACHINE)

# TARGETS
TARGET_SO = $(OBJDIR)/$(COMPONENT).so
TARGET_LIB_SO = $(OBJDIR)/libamxb_dbus.so.$(VERSION)
TARGET_LIB_A = $(OBJDIR)/libamxb_dbus.a

# directories
# source directories
SRCDIR = .
INCDIR_PRIV = ../include_priv
INCDBUSDIR = /usr/include/dbus-1.0 /usr/lib/x86_64-linux-gnu/dbus-1.0/include
INCDIRS = $(INCDIR_PRIV) $(if $(STAGINGDIR), $(STAGINGDIR)/include) $(if $(STAGINGDIR), $(STAGINGDIR)/usr/include)
STAGING_LIBDIR = $(if $(STAGINGDIR), -L$(STAGINGDIR)/lib) $(if $(STAGINGDIR), -L$(STAGINGDIR)/usr/lib)

# files
#HEADERS = $(wildcard $(INCDIR_PUB)/$(TARGET_NAME)/*.h)
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))

# compilation and linking flags
CFLAGS += -Werror -Wall \
          -Wformat=2 -Wshadow \
          -Wwrite-strings -Wredundant-decls \
          -Wmissing-declarations -Wno-attributes \
          -Wno-format-nonliteral \
          $(addprefix -I , $(INCDBUSDIR))\
          -fPIC -g3 $(addprefix -I ,$(INCDIRS))

ifeq ($(CC_NAME),g++)
    CFLAGS += -std=c++2a
else
    CFLAGS += -Wstrict-prototypes -Wold-style-definition -Wnested-externs -std=c11
endif

LDFLAGS += $(STAGING_LIBDIR) -shared -fPIC -lamxc -lamxp -lamxd -lamxb -ldbus-1

# targets
all: ../include_priv/amxb_dbus_version.h $(TARGET_SO) $(TARGET_LIB_SO)  $(TARGET_LIB_A)

$(TARGET_SO): $(OBJECTS)
	$(CC) -Wl,-soname,$(COMPONENT).so -o $@ $(OBJECTS) $(LDFLAGS)

$(TARGET_LIB_SO): $(OBJECTS)
	$(CC) -Wl,-soname,libamxb_dbus.so.$(VMAJOR) -o $@ $(OBJECTS) $(LDFLAGS)

$(TARGET_LIB_A): $(OBJECTS)
	$(AR) rcs $(@) $(OBJECTS)

-include $(OBJECTS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

$(OBJDIR)/:
	$(MKDIR) -p $@

../include_priv/amxb_dbus_version.h: ../include_priv/amxb_dbus_version.h.m4
	@echo "Current backend version = $(VMAJOR).$(VMINOR).$(VBUILD)"
	m4 -DMAJOR=$(VMAJOR) -DMINOR=$(VMINOR) -DBUILD=$(VBUILD) $(<) > $(@)

clean:
	rm -rf ../output/ ../$(COMPONENT)-*.* ../$(COMPONENT)_*.* ../include_priv/amxb_dbus_version.h
	make -C ../test clean
	find ../test -name "run_test" -delete
	find ../test -name "*.so" -delete

.PHONY: all clean
