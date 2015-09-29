#!/bin/sh
cwd=`pwd`
src="mupdf-1.6-source"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi
mkdir -p ${dest}/src
cd ${dest}/src

if [ ! -d "${src}" ]; then
  if [ `which curl` ]; then
    cmd="curl -O"
  else
    cmd="wget"
  fi
  ${cmd} http://mupdf.com/downloads/archive/${src}.tar.gz
  tar -xf ${src}.tar.gz
  patch -p0 <${cwd}/mupdf.patch
fi

opts="prefix=${dest} HAVE_MUJS=no HAVE_CURL=no XCFLAGS=-fPIC"
if [ `uname` = "Darwin" ]; then
  opts="${opts} HAVE_X11=no"
fi

make -C ${src} ${opts}
make -C ${src} ${opts} install

cd ${cwd}

