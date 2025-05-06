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

install: all
	$(INSTALL) -D -p -m 0644 odl/amx-scheduler.odl $(DEST)/etc/amx/amx-scheduler/amx-scheduler.odl
	$(INSTALL) -D -p -m 0644 odl/amx-scheduler_definition.odl $(DEST)/etc/amx/amx-scheduler/amx-scheduler_definition.odl
	$(INSTALL) -D -p -m 0644 output/$(MACHINE)/object/amx-scheduler.so $(DEST)/usr/lib/amx/amx-scheduler/amx-scheduler.so
	$(INSTALL) -d -m 0755 $(DEST)$(BINDIR)
	ln -sfr $(DEST)$(BINDIR)/amxrt $(DEST)$(BINDIR)/amx-scheduler

package: all
	$(INSTALL) -D -p -m 0644 odl/amx-scheduler.odl $(PKGDIR)/etc/amx/amx-scheduler/amx-scheduler.odl
	$(INSTALL) -D -p -m 0644 odl/amx-scheduler_definition.odl $(PKGDIR)/etc/amx/amx-scheduler/amx-scheduler_definition.odl
	$(INSTALL) -D -p -m 0644 output/$(MACHINE)/object/amx-scheduler.so $(PKGDIR)/usr/lib/amx/amx-scheduler/amx-scheduler.so
	$(INSTALL) -d -m 0755 $(PKGDIR)$(BINDIR)
	ln -sfr $(PKGDIR)$(BINDIR)/amxrt $(PKGDIR)$(BINDIR)/amx-scheduler
	cd $(PKGDIR) && $(TAR) -czvf ../$(COMPONENT)-$(VERSION).tar.gz .
	cp $(PKGDIR)../$(COMPONENT)-$(VERSION).tar.gz .
	make -C packages

changelog:
	$(call create_changelog)

doc:
	$(eval ODLFILES += odl/amx-scheduler.odl)
	$(eval ODLFILES += odl/amx-scheduler_definition.odl)

	mkdir -p output/xml
	mkdir -p output/html
	mkdir -p output/confluence
	amxo-cg -Gxml,output/xml/$(COMPONENT).xml $(or $(ODLFILES), "")
	amxo-xml-to -x html -o output-dir=output/html -o title="$(COMPONENT)" -o version=$(VERSION) -o sub-title="Datamodel reference" output/xml/*.xml
	amxo-xml-to -x confluence -o output-dir=output/confluence -o title="$(COMPONENT)" -o version=$(VERSION) -o sub-title="Datamodel reference" output/xml/*.xml

.PHONY: all clean changelog install package doc