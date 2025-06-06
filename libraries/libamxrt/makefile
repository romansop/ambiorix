include makefile.inc

NOW = $(shell date +"%Y-%m-%d(%H:%M:%S %z)")

# Extra destination directories
PKGDIR = ./output/$(MACHINE)/pkg/

define create_changelog
	@$(ECHO) "Update changelog"
	mv CHANGELOG.md CHANGELOG.md.bak
	head -n 9 CHANGELOG.md.bak > CHANGELOG.md
	$(ECHO) "" >> CHANGELOG.md
	$(ECHO) "## Release $(VERSION) - $(NOW)" >> CHANGELOG.md
	$(ECHO) "" >> CHANGELOG.md
	$(GIT) log --pretty=format:"- %s" $$($(GIT) describe --tags | grep -v "merge" | cut -d'-' -f1)..HEAD  >> CHANGELOG.md
	$(ECHO) "" >> CHANGELOG.md
	tail -n +10 CHANGELOG.md.bak >> CHANGELOG.md
	rm CHANGELOG.md.bak
endef

# targets
all:
	$(MAKE) -C src all

clean:
	$(MAKE) -C src clean
	$(MAKE) -C test clean

install: all
	$(INSTALL) -d -m 0755 $(DEST)/$(INCLUDEDIR)/amxrt
	$(INSTALL) -D -p -m 0644 include/amxrt/*.h $(DEST)$(INCLUDEDIR)/amxrt/
	$(INSTALL) -D -p -m 0644 output/$(MACHINE)/$(COMPONENT).so.$(VERSION) $(DEST)$(LIBDIR)/$(COMPONENT).so.$(VERSION)
ifeq ($(CONFIG_REMOVE_CAPS_ODLS),y)
	$(INSTALL) -D -p -m 0775 scripts/remove_cap_files.sh $(DEST)/etc/uci-defaults/15_remove_cap_files.sh
endif
	ln -sfr $(DEST)$(LIBDIR)/$(COMPONENT).so.$(VERSION) $(DEST)$(LIBDIR)/$(COMPONENT).so.$(VMAJOR)
	ln -sfr $(DEST)$(LIBDIR)/$(COMPONENT).so.$(VERSION) $(DEST)$(LIBDIR)/$(COMPONENT).so

package: all
	$(INSTALL) -d -m 0755 $(PKGDIR)/$(INCLUDEDIR)/amxrt
	$(INSTALL) -D -p -m 0644 include/amxrt/*.h $(PKGDIR)$(INCLUDEDIR)/amxrt/
	$(INSTALL) -D -p -m 0644 output/$(MACHINE)/$(COMPONENT).so.$(VERSION) $(PKGDIR)$(LIBDIR)/$(COMPONENT).so.$(VERSION)
ifeq ($(CONFIG_REMOVE_CAPS_ODLS),y)
	$(INSTALL) -D -p -m 0775 scripts/remove_cap_files.sh $(PKGDIR)/etc/uci-defaults/15_remove_cap_files.sh
endif
	cd $(PKGDIR) && $(TAR) -czvf ../$(COMPONENT)-$(VERSION).tar.gz .
	cp $(PKGDIR)../$(COMPONENT)-$(VERSION).tar.gz .
	make -C packages

changelog:
	$(call create_changelog)

doc:
	$(MAKE) -C doc doc


test:
	$(MAKE) -C test run
	$(MAKE) -C test coverage

.PHONY: all clean changelog install package doc test