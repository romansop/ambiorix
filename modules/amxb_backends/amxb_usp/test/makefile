include ../makefile.inc

PKG_CONFIG_LIBDIR := /usr/lib/pkgconfig:/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:$(PKG_CONFIG_LIBDIR)
MACHINE = $(shell $(CC) -dumpmachine)
SUBDIRS := $(wildcard amxb_usp_*)
SUBDIRS := $(filter-out amxb_usp_common, $(SUBDIRS))
OBJDIR = ../output/$(MACHINE)/coverage
COVERREPORT = report

run: ../include_priv/amxb_usp_version.h $(OBJDIR)/
	make -C amxb_usp_common/test_mod/
	@for dir in $(SUBDIRS); do make -C $$dir $@  || exit -1; done
	@rm -rf $(OBJDIR)/test_*

clean:
	rm -rf $(OBJDIR)
	rm -rf $(OBJDIR)/$(COVERREPORT)
	make -C amxb_usp_common/test_mod/ clean
	find .. -name "run_test" -delete
	find .. -name "mod-amxb-test-usp.so" -delete

coverage: $(OBJDIR)/$(COVERREPORT)/
	@cd $(OBJDIR) && \
	for i in $$(find . -type f -iname "*.o" ! -iname "*_mock.o" ! -iname "mock.o"); do \
		gcov -c -b -f --long-file-names --preserve-paths $$i > /dev/null; \
    done
	@cd $(OBJDIR) && for i in $$(find . -name "*.h.gcov"); do rm $$i > /dev/null; done
	cd $(OBJDIR) && gcovr -k -p -r ../../.. -s -g --html --html-details -o ./$(COVERREPORT)/index.html
	cd $(OBJDIR) && gcovr -k -p -r ../../.. -s -g | tee ./$(COVERREPORT)/gcovr_summary.txt

../include_priv/amxb_usp_version.h : ../include_priv/amxb_usp_version.h.m4
	@echo "Current backend version = $(VMAJOR).$(VMINOR).$(VBUILD)"
	m4 -DMAJOR=$(VMAJOR) -DMINOR=$(VMINOR) -DBUILD=$(VBUILD) $(<) > $(@)

$(OBJDIR)/:
	mkdir -p $@

$(OBJDIR)/$(COVERREPORT)/:
	mkdir -p $@

.PHONY: run clean coverage