#!/bin/sh

if [ `which cmake` ]; then exit 0; fi

src="cmake-2.8.12.2"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

curl -O http://www.cmake.org/files/v2.8/${src}.tar.gz

tar xf ${src}.tar.gz

cd ${src}

./bootstrap --prefix=${dest}
make -j4
make install
make clean

cd ..

rm -rf ${src} *.tar.gz

