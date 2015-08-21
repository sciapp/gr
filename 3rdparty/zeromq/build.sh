#!/bin/sh

src="zeromq-3.2.5"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

curl -O http://download.zeromq.org/zeromq-3.2.5.tar.gz

tar xf ${src}.tar.gz

cd ${src}

./configure --prefix=${dest} --disable-shared
make -j4
make install
make distclean

cd ..

rm -rf ${src} *.tar.gz

