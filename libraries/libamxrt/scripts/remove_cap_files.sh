#!/bin/sh

# Removes all the capability files from components, this forces the plugins to run as root

find /etc/amx/ -type f -name "*_caps.odl" -exec rm {} \;

exit 0
