MACHINE = $(shell $(CC) -dumpmachine)

SRCDIR = $(realpath ../../src)
OBJDIR = $(realpath ../../output/$(MACHINE)/coverage)
INCDIR = $(realpath ../../include ../../include_priv ../include) /usr/include/rbus/ /usr/include/rtmessage/

HEADERS = $(wildcard $(INCDIR)/*.h)
SOURCES = $(wildcard ../common/src/*.c)

CFLAGS += -Werror -Wall -Wextra -Wno-attributes \
          --std=gnu99 -g3 -Wmissing-declarations \
		  $(addprefix -I ,$(INCDIR)) \
		  -fprofile-arcs -ftest-coverage \
		  -fkeep-inline-functions -fkeep-static-functions \
		  $(shell pkg-config --cflags cmocka) \
		  -fPIC
		  
LDFLAGS += -fprofile-arcs -ftest-coverage \
           -fkeep-inline-functions -fkeep-static-functions \
		   $(shell pkg-config --libs cmocka) \
		   -lamxc -lamxj -lamxp -lamxb -lamxd -lamxo -lrbus \
		   -lrbuscore -lrtMessage -pthread -ldl \
		   -lamxut

LDFLAGS += -g \
		   -Wl,--wrap=rbus_open \
		   -Wl,--wrap=rbus_close \
   		   -Wl,--wrap=rbus_regDataElements \
   		   -Wl,--wrap=rbus_unregDataElements \
		   -Wl,--wrap=rbusTable_registerRow \
		   -Wl,--wrap=rbusTable_unregisterRow \
		   -Wl,--wrap=socketpair \
		   -Wl,--wrap=pthread_create
