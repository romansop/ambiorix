include ../makefile.inc

PKG_CONFIG_LIBDIR := /usr/lib/pkgconfig:/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:$(PKG_CONFIG_LIBDIR)
MACHINE = $(shell $(CC) -dumpmachine)
SUBDIRS := $(wildcard test_amxrt_*)
OBJDIR = ../output/$(MACHINE)/coverage
COVERREPORT = report

run: $(OBJDIR)/
	make -C test_remote_plugin/
	@for dir in $(SUBDIRS); do make -C $$dir $@  || exit -1; done
	@rm -rf $(OBJDIR)/test_*
	@rm -rf $(OBJDIR)/dummy_*

clean:
	rm -rf $(OBJDIR)
	rm -rf $(OBJDIR)/$(COVERREPORT)
	make -C test_remote_plugin/ clean
	find .. -name "run_test" -delete

coverage: $(OBJDIR)/$(COVERREPORT)/
	@cd $(OBJDIR) && for i in $$(find . -name "*.o"); do gcov -c -b -f --long-file-names --preserve-paths $$i > /dev/null; done
	@cd $(OBJDIR) && for i in $$(find . -name "*.h.gcov"); do rm $$i > /dev/null; done
	cd $(OBJDIR) && gcovr -k -p -r ../../.. -s -g --html --html-details -o ./$(COVERREPORT)/index.html
	cd $(OBJDIR) && gcovr -k -p -r ../../.. -s -g | tee ./$(COVERREPORT)/gcovr_summary.txt

$(OBJDIR)/:
	mkdir -p $@

$(OBJDIR)/$(COVERREPORT)/:
	mkdir -p $@

.PHONY: run clean coverage

