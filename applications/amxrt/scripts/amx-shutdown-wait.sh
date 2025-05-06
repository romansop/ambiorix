#!/bin/sh

# shellcheck disable=SC1091 # Do not follow sourced files
. /usr/lib/amx/scripts/amx_init_functions.sh

case $1 in
    boot)
        ;;
    start)
        ;;
    stop)
        ;;
    shutdown)
        process_shutdown_wait_all 15
        ;;
    restart)
        ;;
    debuginfo)
        ;;
    *)
        echo "Usage : $0 [shutdown]"
        ;;
esac
