#!/bin/bash

if [ ! -f /usr/include/rbus/rbus.h ]; then
    rm -rf rbus                                      
    git clone https://github.com/rdkcentral/rbus.git 
    cd rbus                                          
    git checkout v2.0.5                              
    mkdir build                                      
    cd build
    cmake -DBUILD_RTMESSAGE_LIB=ON -DBUILD_RTMESSAGE_SAMPLE_APP=ON -DBUILD_FOR_DESKTOP=ON \
        -DBUILD_DATAPROVIDER_LIB=ON -DCMAKE_BUILD_TYPE=release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_C_FLAGS="-Wno-stringop-truncation -Wno-stringop-overflow" .. 
    make                                             
    make install                                    
    cd ../../                                        
fi
