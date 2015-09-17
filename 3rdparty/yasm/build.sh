#!/bin/sh

src="yasm-1.3.0"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

curl -O http://www.tortall.net/projects/yasm/releases/${src}.tar.gz

tar xf ${src}.tar.gz

cd ${src}

./configure --prefix=${dest}
make -j4
make install
make clean

cd ..

rm -rf ${src} *.tar.gz

