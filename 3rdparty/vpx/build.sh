#!/bin/sh

export PATH=`pwd`/../build/bin:${PATH}

src="libvpx-v1.3.0"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

curl -O https://webm.googlecode.com/files/${src}.tar.bz2

tar xf ${src}.tar.bz2

patch -p0 <vpx.patch

cd ${src}

./configure --prefix=${dest} --disable-unit-tests --as=yasm
make -j4
make install
make clean

cd ..

rm -rf ${src} *.tar.bz2

