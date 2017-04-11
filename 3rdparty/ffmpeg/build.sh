#!/bin/sh
cwd=`pwd`
src="ffmpeg-2.1.4"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi
mkdir -p ${dest}/src
cd ${dest}/src

if [ ! -d "${src}" ]; then
  if [ `which curl` ]; then
    cmd="curl -k -O"
  else
    cmd="wget --no-check-certificate"
  fi
  ${cmd} https://ffmpeg.org/releases/${src}.tar.gz
  tar -xf ${dest}/src/${src}.tar.gz
fi

cd ${src}

./configure --prefix=${dest} --disable-yasm --disable-asm --enable-pic \
  --enable-libvpx --enable-libtheora --extra-cflags=-I${dest}/include \
  --extra-cxxflags=-I${dest}/include --extra-ldflags=-L${dest}/lib \
  ${FFMPEG_EXTRA_CONFIGURE_FLAGS}

make -j4
make install

cd ${cwd}

