MACHINE = $(shell $(CC) -dumpmachine)

SRCDIR = $(realpath ../../src)
OBJDIR = $(realpath ../../output/$(MACHINE)/coverage)
INCDIR = $(realpath ../../include ../../include_priv ../include)

HEADERS = $(wildcard $(INCDIR)/amxtui/*.h)
SOURCES = $(wildcard $(SRCDIR)/amxtui_*.c) \
          $(wildcard $(SRCDIR)/controllers/*.c)

CFLAGS += -Werror -Wall -Wextra \
          --std=gnu99 -g3 -Wmissing-declarations \
		  $(addprefix -I ,$(INCDIR)) \
		  -fprofile-arcs -ftest-coverage \
		  -fkeep-inline-functions -fkeep-static-functions \
		  $(shell pkg-config --cflags cmocka) -pthread

LDFLAGS += -fprofile-arcs -ftest-coverage \
           -fkeep-inline-functions -fkeep-static-functions \
		   $(shell pkg-config --libs cmocka) -pthread -lncurses -lamxc -lamxt -lamxp