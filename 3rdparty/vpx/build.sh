#!/bin/sh

export PATH=`pwd`/../build/bin:${PATH}

src="libvpx-1.4.0"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

curl -O http://downloads.webmproject.org/releases/webm/${src}.tar.bz2

tar xf ${src}.tar.bz2

cd ${src}

export CFLAGS="-fPIC"
./configure --prefix=${dest} --disable-unit-tests --target=generic-gnu \
  --enable-pic
make -j4
make install
make clean

cd ..

rm -rf ${src} *.tar.bz2

