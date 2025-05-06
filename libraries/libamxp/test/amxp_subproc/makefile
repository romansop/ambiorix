include ../test_defines.mk

TARGET = run_test
SOURCES += $(wildcard ./*.c)
OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o))) \
          $(OBJDIR)/../amxp_expr.tab.o \
		  $(OBJDIR)/../lex.amxp_expr.o

WRAP_FUNC=-Wl,--wrap=
MOCK_WRAP= geteuid \
           capng_apply_caps_fd \
		   capng_update \
		   capng_get_caps_process \
		   capng_have_capability

LDFLAGS += -g $(addprefix $(WRAP_FUNC),$(MOCK_WRAP))

include ../test_targets.mk
