#!/bin/sh
cwd=`pwd`
src="cairo-1.14.6"
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
  ${cmd} https://cairographics.org/releases/${src}.tar.xz
  tar -xf ${src}.tar.xz
fi

cd ${src}

export CFLAGS=-I${dest}/../zlib
export LIBS=${dest}/../zlib/libz.lib
export png_REQUIRES=libpng16
export png_CFLAGS=-I${dest}/../libpng16
export png_LIBS=${dest}/../libpng16/libpng.lib
export png_NONPKGCONFIG_CFLAGS=-I${dest}/../libpng16
export png_NONPKGCONFIG_LIBS=${dest}/../libpng16/libpng.lib
export pixman_CFLAGS=-I${dest}/include/pixman-1
export pixman_LIBS=${dest}/lib/libpixman-1.a
export MAKE=make

./configure --prefix=${dest} --libdir=${dest}/lib \
  --enable-static --disable-shared --with-pic --disable-quartz --disable-ft --host=x86_64-w64-mingw32 --disable-ps --disable-pdf --disable-xlib --disable-interpreter --disable-valgrind
make
make install
export MAKE="make -f makefile.mingw"
cd ${cwd}
ln -s -f ${dest}/src/${src}/src ${dest}/src/cairo
