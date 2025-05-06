#!/bin/sh

. /usr/lib/amx/scripts/amx_init_functions.sh

name="amx-fcgi"

prepare(){
    export FCGI_SOCKET_DIR=/var/run/http/
    export LIGHTTPD_USER=lighttpd
    export LIGHTTPD_CHROOT=/webui
    export FCGI_SOCKET=amx-fcgi.sock
    mkdir -p $LIGHTTPD_CHROOT$FCGI_SOCKET_DIR
    chown lighttpd $LIGHTTPD_CHROOT$FCGI_SOCKET_DIR
    mkdir -p /tmp/upload && chmod 1777 /tmp/upload
    mkdir -p /tmp/download && chmod 1777 /tmp/download  
}

case $1 in
    boot)
        prepare
        process_boot ${name} -D
        ;;
    start)
        prepare
        process_start ${name} -D
        ;;
    stop)
        process_stop ${name}
        ;;
    shutdown)
        process_shutdown ${name}
        ;;
    restart)
        $0 stop
        $0 start
        ;;
    debuginfo)
        ;;
    *)
        echo "Usage : $0 [start|boot|stop|shutdown|debuginfo|restart]"
        ;;
esac
