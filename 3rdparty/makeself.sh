#!/bin/sh

if [ $# = 0 ]; then
  opts="xft=no"
  if [ "`which wx-config 2>/dev/null`" = "" ]; then
    opts="${opts} wx=no"
  fi
  if [ "`which gtk-demo 2>/dev/null`" = "" ]; then
    opts="${opts} gtk=no"
  fi
else
  opts="$*"
fi
if [ `uname` = 'Darwin' ]; then
  opts="${opts} x11=no"
fi
if [ -z "${QTDIR}" ]; then
  for dir in ${HOME}/anaconda /opt/anaconda /usr/local/anaconda
  do
    if [ -d ${dir} ]; then
      export QTDIR=${dir}
      break
    fi
  done
fi

extras=`pwd`/3rdparty/build
export PATH=${PATH}:${extras}/bin

make -C 3rdparty extras
make EXTRA_CFLAGS=-I${extras}/include \
     EXTRA_CXXFLAGS=-I${extras}/include \
     EXTRA_LDFLAGS=-L${extras}/lib ${opts} install
