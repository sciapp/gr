#!/bin/sh
cwd=`pwd`
src="openjpeg-2.0.0"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

export PATH=${dest}/bin:${PATH}
if [ ! `which cmake` ]; then exit 0; fi

mkdir -p ${dest}/src
cd ${dest}/src

if [ ! -d "${src}" ]; then
  if [ `which curl` ]; then
    cmd="curl -O"
  else
    cmd="wget"
  fi
  ${cmd} https://openjpeg.googlecode.com/files/openjpeg-2.0.0.tar.gz
  tar -xf ${src}.tar.gz
fi

cd ${src}

export CFLAGS=-fPIC
cmake -DCMAKE_INSTALL_PREFIX:PATH=${dest} -DBUILD_SHARED_LIBS=OFF
make -j4
make install

cd ${cwd}

