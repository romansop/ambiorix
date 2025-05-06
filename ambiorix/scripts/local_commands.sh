#!/bin/bash

function uncrust() {
    CONF=$1
    if [[ -z $CONF ]]; then
            CONF="uncrust.conf"
    fi
    find . -name "*.c" > files.txt; find . -name "*.h" >> files.txt
    uncrustify --replace --no-backup -c $CONF -F files.txt
    rm files.txt
}

function amx_remove_installed() {
	sudo rm -rf /usr/lib/x86_64-linux-gnu/libamx*
	sudo rm -rf /usr/bin/mods/amxb/*
	sudo rm -rf /usr/lib/amx/*
	sudo rm -rf /usr/bin/amx*
	sudo rm -rf /usr/include/amx*
	sudo rm -rf /usr/lib/amx
	sudo rm -rf /usr/bin/mods/amx*
	sudo rm -rf /etc/amx*
}

function amx_rebuild_libs() {
	AMX_LIBS="libraries/libamxc libraries/libamxj libraries/libamxp libraries/libamxm libraries/libamxt libraries/libamxd libraries/libamxb libraries/libamxa libraries/libamxs libraries/libamxo libraries/libamxrt"
	for proj in $AMX_LIBS; do repo forall -e -v "$proj" -c "sudo make clean && make && sudo -E make install"; done
}


function amx_rebuild_bus_backends() {
	repo forall -e -v bus_adaptors/* -c "sudo make clean && make && sudo -E make install"
}

function amx_rebuild_apps() {
	repo forall -e -v applications/* -c "sudo make clean && make && sudo -E make install"
	repo forall -e -v amxlab/tui/applications/* -c "sudo make clean && make && sudo -E make install"
}

function amx_rebuild_cli_mods() {
	export CONFIG_SAH_MOD_BA_CLI_INSTALL_PCB_CLI=y
	export CONFIG_SAH_MOD_BA_CLI_INSTALL_UBUS_CLI=y
	repo forall -e -v cli_extensions/* -c "sudo make clean && make && sudo -E make install"
}

function amx_rebuild_bindings() {
	repo forall -e -v bindings/lua/* -c "sudo make clean && make && sudo -E make install"
	repo forall -e -v bindings/python3 -c "sudo make clean && make && sudo -E make install"
}

function amx_rebuild_examples() {
	repo forall -e -v examples/*/* -c "sudo make clean && make && sudo -E make install DEST="
}

function amx_rebuild_all() {
	amx_rebuild_libs
	amx_rebuild_bus_backends
	amx_rebuild_apps
	amx_rebuild_cli_mods
	amx_rebuild_bindings
	amx_rebuild_examples
	sudo ldconfig
}

