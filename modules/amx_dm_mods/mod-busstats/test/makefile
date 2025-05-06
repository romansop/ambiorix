PKG_CONFIG_LIBDIR := /usr/lib/pkgconfig:/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:$(PKG_CONFIG_LIBDIR)
MACHINE = $(shell $(CC) -dumpmachine)
SUBDIRS := $(wildcard test_busstats_*)
OBJDIR = ../output/$(MACHINE)/coverage
COVERREPORT = report

TEST_SO = mod-busstats.so
SO_SRCDIR = $(realpath ../src)
SO_SOURCES = $(wildcard $(SO_SRCDIR)/*.c)
SO_OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SO_SOURCES:.c=.o)))

INCDIR = $(realpath ../include_priv)

CFLAGS += -Werror -Wall -Wextra -Wno-attributes \
          -g3 -Wmissing-declarations \
		  $(addprefix -I ,$(INCDIR)) \
		  -fprofile-arcs -ftest-coverage \
		  -fkeep-inline-functions -fkeep-static-functions \
          $(shell pkg-config --cflags cmocka) \
		  -fPIC 
		  
LDFLAGS += -fprofile-arcs -ftest-coverage \
		   -shared -fPIC -lamxc -lamxp -lamxd -lamxb -pthread \
		   $(shell pkg-config --libs cmocka)

run: $(OBJDIR)/ $(TEST_SO)
	@echo "Test dirs = $(SUBDIRS)"
	@for dir in $(SUBDIRS); do make -C $$dir $@  || exit -1; done
	@rm -rf $(OBJDIR)/test_*

clean: 
	@for dir in $(SUBDIRS); do make -C $$dir $@  || exit -1; done
	rm -rf $(OBJDIR)
	rm -rf $(OBJDIR)/$(COVERREPORT)
	rm -rf $(TEST_SO)

# Note: standard coverage measuring does not measure the code ran under a separate amxrt process,
#       since the module is so tiny it's probably not worth the effort to measure coverage,
#       but if you want it feel free to set that up.
coverage: $(OBJDIR)/$(COVERREPORT)/
	cd $(OBJDIR) && \
	for i in $$(find . -type f -iname "*.o" ! -iname "dummy_be.o" ! -iname "wrap_*.o"); do \
		gcov -c -b -f --long-file-names --preserve-paths $$i > /dev/null; \
    done
	@cd $(OBJDIR) && for i in $$(find . -name "*.h.gcov"); do rm $$i > /dev/null; done
	cd $(OBJDIR) && gcovr -k -p -r ../../.. -s -g --html --html-details -o ./$(COVERREPORT)/index.html
	cd $(OBJDIR) && gcovr -k -p -r ../../.. -s -g | tee ./$(COVERREPORT)/gcovr_summary.txt


$(TEST_SO): $(SO_OBJECTS)
	$(CC) -Wl,-soname,$(TEST_SO) -o $@ $(SO_OBJECTS) $(LDFLAGS)

$(OBJDIR)/%.o: $(SO_SRCDIR)/%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

$(OBJDIR)/:
	mkdir -p $@

$(OBJDIR)/$(COVERREPORT)/:
	mkdir -p $@

.PHONY: run clean coverage

