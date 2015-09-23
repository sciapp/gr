#!/bin/sh

src="libtheora-1.1.1"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

curl -O http://downloads.xiph.org/releases/theora/${src}.tar.gz

tar xf ${src}.tar.gz

cd ${src}

./configure --prefix=${dest} --disable-shared --with-pic \
  --with-ogg-includes=`pwd`/../../build/include \
  --with-ogg-libraries=`pwd`/../../build/lib
make -j4
make install
make clean

cd ..

rm -rf ${src} *.tar.gz

