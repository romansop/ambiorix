include ../makefile.inc

# build destination directories
OBJDIR = ../output/$(MACHINE)

# TARGETS
TARGET_SO = $(OBJDIR)/$(COMPONENT).so
TARGET_LIB_SO = $(OBJDIR)/libamxb_ubus.so.$(VERSION)
TARGET_LIB_A = $(OBJDIR)/libamxb_ubus.a

# directories
# source directories
SRCDIR = .
INCDIR_PUB = ../include /usr/include/rbus/
INCDIR_PRIV = ../include_priv
INCDIRS = $(INCDIR_PUB) $(INCDIR_PRIV) $(if $(STAGINGDIR), $(STAGINGDIR)/include) $(if $(STAGINGDIR), $(STAGINGDIR)/usr/include) $(if $(STAGINGDIR), $(STAGINGDIR)/usr/include/rbus)
STAGING_LIBDIR = $(if $(STAGINGDIR), -L$(STAGINGDIR)/lib) $(if $(STAGINGDIR), -L$(STAGINGDIR)/usr/lib)

# files
HEADERS = $(wildcard $(INCDIR_PUB)/$(TARGET_NAME)/*.h)
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))
		
# compilation and linking flags
CFLAGS += -Werror -Wall \
          -Wformat=2 -Wshadow \
          -Wwrite-strings -Wredundant-decls \
		  -Wmissing-declarations -Wno-attributes \
          -fPIC -g3 $(addprefix -I ,$(INCDIRS)) \
		  -pthread

ifeq ($(CC_NAME),g++)
    CFLAGS += -std=c++2a
else
	CFLAGS += -Wstrict-prototypes -Wold-style-definition -Wnested-externs -std=c11
endif

LDFLAGS += $(STAGING_LIBDIR) -shared -fPIC -lamxc -lamxp -lamxd -lamxb -lrbus -pthread

# targets
all: ../include_priv/amxb_rbus_version.h $(TARGET_SO)

$(TARGET_SO): $(OBJECTS)
	$(CC) -Wl,-soname,$(COMPONENT).so -o $@ $(OBJECTS) $(LDFLAGS)

-include $(OBJECTS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

$(OBJDIR)/:
	$(MKDIR) -p $@

../include_priv/amxb_rbus_version.h: ../include_priv/amxb_rbus_version.h.m4
	@echo "Current backend version = $(VMAJOR).$(VMINOR).$(VBUILD)"
	m4 -DMAJOR=$(VMAJOR) -DMINOR=$(VMINOR) -DBUILD=$(VBUILD) $(<) > $(@)

clean:
	rm -rf ../output/ ../$(COMPONENT)-*.* ../$(COMPONENT)_*.* ../include_priv/amxb_rbus_version.h

.PHONY: all clean
