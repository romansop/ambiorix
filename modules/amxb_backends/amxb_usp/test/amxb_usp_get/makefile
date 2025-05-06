include ../test_defines.mk

TEST_SO = ../mod-amxb-test-usp.so

TARGET = run_test
SOURCES += $(wildcard ./*.c) $(MOCK_SOURCES) $(COMMON_SOURCES)
SO_SOURCE = $(SOURCES)
SO_OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SO_SOURCES:.c=.o)))
OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))

include ../test_targets.mk

$(TEST_SO): $(SO_OBJECTS)
	$(CC) -Wl,-soname,$(TARGET)-$(VMAJOR).so -o $@ $(OBJECTS) $(LDFLAGS_MAIN) -shared -fPIC
