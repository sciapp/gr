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
    cmd="curl -k -O"
  else
    cmd="wget"
  fi
  ${cmd} http://mupdf.com/downloads/archive/${src}.tar.gz
  tar -xf ${src}.tar.gz
fi
patch -N -p0 --dry-run --silent &>/dev/null <${cwd}/mupdf.patch &&\
patch -p0 <${cwd}/mupdf.patch

opts="prefix=${dest} HAVE_MUJS=no HAVE_CURL=no XCFLAGS=-fPIC"
if [ `uname` = "Darwin" ]; then
  opts="${opts} HAVE_X11=no"
fi

if [ "${MUPDF_CROSS_COMPILE}" != "" ]; then
  CC=gcc CXX=g++ AR=ar OS="" make -C ${src} generate
fi

make -C ${src} ${opts}
make -C ${src} ${opts} install

cd ${cwd}

