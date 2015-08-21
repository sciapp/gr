#!/bin/sh

src="ffmpeg-2.1.4"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

curl -O https://ffmpeg.org/releases/${src}.tar.gz

tar xf ${src}.tar.gz

cd ${src}

./configure --prefix=${dest} --disable-yasm --enable-libvpx --enable-libtheora
make -j4
make install
make distclean

cd ..

rm -rf ${src} *.tar.gz

