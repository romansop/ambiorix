include ../makefile.inc

PKG_CONFIG_LIBDIR := /usr/lib/pkgconfig:/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:$(PKG_CONFIG_LIBDIR)
SUBDIRS := $(wildcard amxb_rbus*)
OBJDIR = ../output/$(MACHINE)/coverage
COVERREPORT = report

TEST_SO = mod-amxb-test-rbus.so
SO_SRCDIR = $(realpath ../src)
SO_SOURCES = $(wildcard $(SO_SRCDIR)/*.c)  ./common/src/test_wrap.c
SO_OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SO_SOURCES:.c=.o)))

INCDIR = $(realpath ../include ../include_priv) /usr/include/rbus/ /usr/include/rtmessage/

CFLAGS += -Werror -Wall -Wextra -Wno-attributes \
          -g3 -Wmissing-declarations \
		  $(addprefix -I ,$(INCDIR)) \
		  -fprofile-arcs -ftest-coverage \
		  -fkeep-inline-functions -fkeep-static-functions \
 		  $(shell pkg-config --cflags cmocka) \
		  -fPIC 
		  
LDFLAGS += -fprofile-arcs -ftest-coverage \
		   -shared -fPIC -lamxc -lamxp -lamxd -lamxb -lrbus -lrbuscore -pthread \
		   $(shell pkg-config --libs cmocka) \
		   -Wl,--wrap=rbus_open \
		   -Wl,--wrap=rbus_close \
		   -Wl,--wrap=rbus_regDataElements \
		   -Wl,--wrap=rbus_unregDataElements \
		   -Wl,--wrap=rbusTable_registerRow \
		   -Wl,--wrap=rbusTable_unregisterRow \
		   -Wl,--wrap=socketpair \
		   -Wl,--wrap=pthread_create \

run: ../include_priv/amxb_rbus_version.h $(OBJDIR)/ $(TEST_SO)
	make -C common/test_mod/ 
	make -C common/test_client/
	-killall rtrouted
	-rm -rf /tmp/rtrouted
	ps aux | grep rtrouted
	for dir in $(SUBDIRS); do \
	  echo "Test: $$dir"; \
	  rtrouted; \
	  sleep 1; \
	  ps aux | grep rtrouted; \
	  make -C $$dir $@ || exit -1; \
	  killall rtrouted; \
	  rm -rf /tmp/rtrouted; \
	done
	@rm -rf $(OBJDIR)/test_*

clean: 
	rm -rf $(OBJDIR)
	rm -rf $(OBJDIR)/$(COVERREPORT)
	rm -rf $(TEST_SO)
	make -C common/test_mod/ clean
	make -C common/test_client/ clean
	find .. -name "run_test" -delete

coverage: $(OBJDIR)/$(COVERREPORT)/
	@cd $(OBJDIR) && for i in $$(find . -name "*.o"); do gcov -c -b -f --long-file-names --preserve-paths $$i > /dev/null; done
	@cd $(OBJDIR) && for i in $$(find . -name "*.h.gcov"); do rm $$i > /dev/null; done

	cd $(OBJDIR) && gcovr -k -p -r ../../.. -s -g --html --html-details -o ./$(COVERREPORT)/index.html
	cd $(OBJDIR) && gcovr -k -p -r ../../.. -s -g | tee ./$(COVERREPORT)/gcovr_summary.txt

../include_priv/amxb_rbus_version.h : ../include_priv/amxb_rbus_version.h.m4
	@echo "Current backend version = $(VMAJOR).$(VMINOR).$(VBUILD)"
	m4 -DMAJOR=$(VMAJOR) -DMINOR=$(VMINOR) -DBUILD=$(VBUILD) $(<) > $(@)

$(TEST_SO): $(SO_OBJECTS)
	$(CC) -Wl,-soname,$(TEST_SO) -o $@ $(SO_OBJECTS) $(LDFLAGS)

$(OBJDIR)/%.o: ./common/src/%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

$(OBJDIR)/%.o: ./common/dummy_mocka/%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

$(OBJDIR)/%.o: $(SO_SRCDIR)/%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

$(OBJDIR)/:
	mkdir -p $@

$(OBJDIR)/$(COVERREPORT)/:
	mkdir -p $@

.PHONY: run clean coverage
