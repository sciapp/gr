#!/bin/sh

if [ ! `which cmake` ]; then exit 0; fi

src="openjpeg-2.0.0"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

if [ `which curl` ]; then
  cmd="curl -O"
else
  cmd="wget"
fi
${cmd} https://openjpeg.googlecode.com/files/openjpeg-2.0.0.tar.gz

tar xf ${src}.tar.gz

cd ${src}

export CFLAGS=-fPIC
cmake -DCMAKE_INSTALL_PREFIX:PATH=${dest} -DBUILD_SHARED_LIBS=OFF
make -j4
make install
make clean

cd ..

rm -rf ${src} *.tar.gz

