#!/bin/sh

src="ffmpeg-2.1.4"
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
${cmd} https://ffmpeg.org/releases/${src}.tar.gz

tar xf ${src}.tar.gz

cd ${src}

./configure --prefix=${dest} --disable-yasm --disable-asm --enable-pic \
  --enable-libvpx --enable-libtheora --extra-cflags=-I${dest}/include \
  --extra-cxxflags=-I${dest}/include --extra-ldflags=-L${dest}/lib

make -j4
make install
make distclean

cd ..

rm -rf ${src} *.tar.gz

