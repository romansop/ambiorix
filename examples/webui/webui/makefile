include makefile.inc

NOW = $(shell date +"%Y-%m-%d(%H:%M:%S %z)")

# Extra destination directories
PKGDIR = ./output/$(MACHINE)/pkg/

# helper functions - used in multiple targets
define install_to
	$(INSTALL) -d -m 0755 $(1)//webui/etc/conf.d
	$(INSTALL) -D -p -m 0644 webui/etc/conf.d/* $(1)/webui/etc/conf.d/
	$(INSTALL) -d -m 0755 $(1)//webui/etc/vhosts.d
	$(INSTALL) -D -p -m 0644 webui/etc/vhosts.d/* $(1)/webui/etc/vhosts.d/
	$(INSTALL) -D -p -m 0644 webui/etc/lighttpd.conf $(1)/webui/etc/lighttpd.conf
	$(INSTALL) -D -p -m 0644 webui/etc/lighttpd.user $(1)/webui/etc/lighttpd.user
	$(INSTALL) -D -p -m 0644 webui/etc/modules.conf $(1)/webui/etc/modules.conf
	$(INSTALL) -d -m 0755 $(1)//webui/htdocs/animate/4.1.1/css
	$(INSTALL) -D -p -m 0644 webui/htdocs/animate/4.1.1/css/* $(1)/webui/htdocs/animate/4.1.1/css/
	$(INSTALL) -d -m 0755 $(1)//webui/htdocs/animate/4.1.1/css
	$(INSTALL) -D -p -m 0644 webui/htdocs/animate/4.1.1/css/* $(1)/webui/htdocs/animate/4.1.1/css/
	$(INSTALL) -d -m 0755 $(1)//webui/htdocs/bootstrap/4.6.0/css
	$(INSTALL) -D -p -m 0644 webui/htdocs/bootstrap/4.6.0/css/* $(1)/webui/htdocs/bootstrap/4.6.0/css/
	$(INSTALL) -d -m 0755 $(1)//webui/htdocs/bootstrap/4.6.0/js
	$(INSTALL) -D -p -m 0644 webui/htdocs/bootstrap/4.6.0/js/* $(1)/webui/htdocs/bootstrap/4.6.0/js/
	$(INSTALL) -d -m 0755 $(1)//webui/htdocs/jquery/3.5.1/js
	$(INSTALL) -D -p -m 0644 webui/htdocs/jquery/3.5.1/js/* $(1)/webui/htdocs/jquery/3.5.1/js/
	$(INSTALL) -d -m 0755 $(1)//webui/htdocs/tr181_web/css
	$(INSTALL) -D -p -m 0644 webui/htdocs/tr181_web/css/* $(1)/webui/htdocs/tr181_web/css/
	$(INSTALL) -d -m 0755 $(1)//webui/htdocs/tr181_web/js
	$(INSTALL) -D -p -m 0644 webui/htdocs/tr181_web/js/* $(1)/webui/htdocs/tr181_web/js/
	$(INSTALL) -d -m 0755 $(1)//webui/htdocs/tr181_web/templates
	$(INSTALL) -D -p -m 0644 webui/htdocs/tr181_web/templates/* $(1)/webui/htdocs/tr181_web/templates/
	$(INSTALL) -d -m 0755 $(1)//webui/htdocs
	$(INSTALL) -D -p -m 0644 webui/htdocs/*.html $(1)/webui/htdocs/
	$(INSTALL) -D -p -m 0644 webui/htdocs/favicon.ico $(1)/webui/htdocs/favicon.ico
	$(INSTALL) -D -p -m 0644 webui/lib/lighttpd/.gitkeep $(1)/webui/lib/lighttpd/.gitkeep
	$(INSTALL) -D -p -m 0644 webui/logs/.gitkeep $(1)/webui/logs/.gitkeep
	$(INSTALL) -D -p -m 0644 webui/uploads/.gitkeep $(1)/webui/uploads/.gitkeep
	$(INSTALL) -D -p -m 0644 webui/var/run/.gitkeep $(1)/webui/var/run/.gitkeep
endef

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

clean:

install: all
	$(call install_to,$(DEST))

package: all
	$(call install_to,$(PKGDIR))
	cd $(PKGDIR) && $(TAR) -czvf ../$(COMPONENT)-$(VERSION).tar.gz .
	cp $(PKGDIR)../$(COMPONENT)-$(VERSION).tar.gz .
	make -C packages

changelog:
	$(call create_changelog)

.PHONY: all clean changelog install package