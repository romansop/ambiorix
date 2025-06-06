MACHINE = $(shell $(CC) -dumpmachine)

OBJDIR = $(realpath ../../output/$(MACHINE)/coverage)
INCDIR = $(realpath ../../include_priv)

HEADERS = 
SOURCES = $(wildcard $(SRCDIR)/*.c)

CFLAGS += -Werror -Wall -Wextra -Wno-attributes\
          --std=gnu99 -g3 -Wmissing-declarations \
		  $(addprefix -I ,$(INCDIR)) -I$(OBJDIR)/.. \
		  -fkeep-inline-functions -fkeep-static-functions \
		   -Wno-format-nonliteral \
		  $(shell pkg-config --cflags cmocka) -pthread
LDFLAGS += -fkeep-inline-functions -fkeep-static-functions \
		   $(shell pkg-config --libs cmocka) \
		   -lamxc -lamxp -lamxd -lamxb -lamxo -ldl -lpthread -lsahtrace



COMPONENT = mod-busstats
OBJDIR_COMPONENT = ../../output/$(MACHINE)
TARGET_COMPONENT_SO = $(OBJDIR)/$(COMPONENT).so