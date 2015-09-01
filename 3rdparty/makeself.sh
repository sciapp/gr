#!/bin/sh

opts="gtk=no wx=no"
if [ `uname` = 'Darwin' ]; then
  opts="${opts} x11=no"
fi
if [ -d /opt/anaconda ]; then
  export QTDIR=/opt/anaconda
fi

extras=`pwd`/3rdparty/build
export PATH=${PATH}:${extras}/bin

make -C 3rdparty extras
make EXTRA_CFLAGS=-I${extras}/include \
     EXTRA_CXXFLAGS=-I${extras}/include \
     EXTRA_LDFLAGS=-L${extras}/lib ${opts}
