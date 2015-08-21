#!/bin/sh

opts="gtk=no wx=no"
if [ `uname` = 'Darwin' ]; then
  opts="${opts} x11=no"
fi
if [ -d /usr/local/anaconda ]; then
  export QTDIR=/usr/local/anaconda
fi

make -C 3rdparty extras
extras=`pwd`/3rdparty/build
make EXTRA_CFLAGS=-I${extras}/include \
     EXTRA_CXXFLAGS=-I${extras}/include \
     EXTRA_LDFLAGS=-L${extras}/lib ${opts}
