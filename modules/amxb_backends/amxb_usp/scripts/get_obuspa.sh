#!/bin/bash
set -e # exit on (most) errors

if [ ! -f /usr/local/bin/obuspa ]; then
    rm -rf obuspa
    git clone https://github.com/BroadbandForum/obuspa.git
    cd obuspa/
    # Would prefer to checkout a tag, but there is no recent tag
    git checkout c5554313d5ea77edfdfe24e31cd786399609304b
    export PKG_CONFIG_PATH="/usr/lib/pkgconfig:/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig"
    autoreconf --force --install
    ./configure --disable-coap --disable-websockets --disable-stomp --disable-bulkdata
    make CFLAGS="-DOPENSSL_API_COMPAT=0x10000000L"
    make install
    cd ../
fi
