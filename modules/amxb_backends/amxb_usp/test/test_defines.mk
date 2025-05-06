MACHINE = $(shell $(CC) -dumpmachine)

SRCDIR = $(realpath ../../src)
OBJDIR = $(realpath ../../output/$(MACHINE)/coverage)
INCDIR = $(realpath ../../include ../../include_priv ../include ../mocks ../amxb_usp_common)

HEADERS = $(wildcard $(INCDIR)/*.h)
SOURCES = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/local-agent/*.c)

MOCK_SRC_DIR = ../mocks
MOCK_SOURCES = $(wildcard $(MOCK_SRC_DIR)/*.c)
COMMON_SRC_DIR = ../amxb_usp_common
COMMON_SOURCES = $(wildcard $(COMMON_SRC_DIR)/*.c)

MOCK_WRAP = imtp_connection_connect \
            imtp_connection_delete \
            imtp_connection_listen \
            imtp_connection_accept \
            imtp_connection_get_con \
            imtp_connection_read_frame \
            imtp_connection_write_frame \
            read

WRAP_FUNC=-Wl,--wrap=

CFLAGS += -Werror -Wall -Wextra -Wno-attributes \
          --std=gnu99 -g3 -Wmissing-declarations \
          $(addprefix -I ,$(INCDIR)) \
          -fprofile-arcs -ftest-coverage \
          -fkeep-inline-functions -fkeep-static-functions \
          $(shell pkg-config --cflags cmocka) \
          -fPIC \
          -DCONFIG_SAH_MOD_AMXB_USP_REQUIRES_DEVICE_PREFIX

LDFLAGS_MAIN += -fprofile-arcs -ftest-coverage \
           -fkeep-inline-functions -fkeep-static-functions \
           $(shell pkg-config --libs cmocka) \
           -lamxc -lamxj -lamxb -lamxd -lamxp -lamxo -limtp -lusp -luspi -lamxut

LDFLAGS = $(LDFLAGS_MAIN) -g $(addprefix $(WRAP_FUNC),$(MOCK_WRAP))
