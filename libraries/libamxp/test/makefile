PKG_CONFIG_LIBDIR := /usr/lib/pkgconfig:/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:$(PKG_CONFIG_LIBDIR)
MACHINE = $(shell $(CC) -dumpmachine)
SUBDIRS := $(wildcard amxp_*)
OBJDIR = ../output/$(MACHINE)/coverage
RESULTSDIR = ../output/result
COVERREPORT = report

run: $(OBJDIR)/ $(RESULTSDIR)/
	mkdir -p $(RESULTSDIR)/test
	@for dir in $(SUBDIRS); do make -C $$dir $@  || exit -1; done
	@rm -rf $(OBJDIR)/test_*

clean: 
	rm -rf $(OBJDIR)
	rm -rf $(OBJDIR)/$(COVERREPORT)
	find .. -name "run_test" -delete

coverage: $(OBJDIR)/$(COVERREPORT)/
	@cd $(OBJDIR) && \
	for i in $$(find . -type f -iname "*.o" ! -iname "lex.*.o" ! -name "*.tab.o"); do \
		gcov -c -b -f --long-file-names --preserve-paths $$i > /dev/null; \
    done
	@cd $(OBJDIR) && for i in $$(find . -name "*.h.gcov"); do rm $$i > /dev/null; done
	cd $(OBJDIR)/.. && gcovr -k -p -r ../.. -s -g --html --html-details -o ./coverage/$(COVERREPORT)/index.html
	cd $(OBJDIR)/.. && gcovr -k -p -r ../.. -s -g | tee ./coverage/$(COVERREPORT)/gcovr_summary.txt


$(OBJDIR)/:
	mkdir -p $@

$(RESULTSDIR)/:
	mkdir -p $@

$(OBJDIR)/$(COVERREPORT)/:
	mkdir -p $@

.PHONY: run clean coverage

