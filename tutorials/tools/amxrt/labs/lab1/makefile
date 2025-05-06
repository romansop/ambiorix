install: dir
	install phonebook.odl phonebook_definition.odl /etc/amx/phonebook
	ln -sf /usr/bin/amxrt phonebook

dir:
	mkdir -p /etc/amx/phonebook

uninstall:
	rm -rf /etc/amx/phonebook
	rm -f ./phonebook

