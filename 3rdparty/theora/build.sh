#!/bin/sh

src="libtheora-1.1.1"
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
${cmd} http://downloads.xiph.org/releases/theora/${src}.tar.gz

tar xf ${src}.tar.gz

patch -p0 <libtheora.patch

cd ${src}

./configure --prefix=${dest} --libdir=${dest}/lib --disable-shared --with-pic \
  --with-ogg-includes=`pwd`/../../build/include \
  --with-ogg-libraries=`pwd`/../../build/lib
make -j4
make install
make clean

cd ..

rm -rf ${src} *.tar.gz

