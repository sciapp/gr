#!/bin/sh
make -C 3rdparty extras
extras=`pwd`/3rdparty/build
make EXTRA_CFLAGS=-I${extras}/include \
     EXTRA_CXXFLAGS=-I${extras}/include \
     EXTRA_LDFLAGS=-L${extras}/lib
