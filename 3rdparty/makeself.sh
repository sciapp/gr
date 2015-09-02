#!/bin/sh

opts="gtk=no wx=no"
if [ `uname` = 'Darwin' ]; then
  opts="${opts} x11=no"
fi
for dir in ${HOME}/anaconda /opt/anaconda /usr/local/anaconda
do
  if [ -d ${dir} ]; then
    export QTDIR=${dir}
    break
  fi
done

extras=`pwd`/3rdparty/build
export PATH=${PATH}:${extras}/bin

make -C 3rdparty extras
make EXTRA_CFLAGS=-I${extras}/include \
     EXTRA_CXXFLAGS=-I${extras}/include \
     EXTRA_LDFLAGS=-L${extras}/lib ${opts}
