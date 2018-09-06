#!/bin/sh
set -e
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
    cmd="curl -k -O -L"
  else
    cmd="wget --no-check-certificate"
  fi
  ${cmd} https://gr-framework.org/downloads/3rdparty/${src}.tar.xz
  tar -xf ${src}.tar.xz
fi

cd ${src}

export CFLAGS="-I${cwd}/../zlib -O"
if [ -f "${cwd}/../zlib/libz.a" ]; then
  export LIBS=${cwd}/../zlib/libz.a
elif [ -f "${dest}/lib/libz.a" ]; then
  export LIBS=${dest}/lib/libz.a
else
  echo "libz.a not found."
  exit 1
fi
export png_REQUIRES=libpng16
export png_CFLAGS=-I${dest}/../libpng16
export png_LIBS=${dest}/../libpng16/libpng.a
export png_NONPKGCONFIG_CFLAGS=-I${dest}/../libpng16
export png_NONPKGCONFIG_LIBS=${dest}/../libpng16/libpng.a
export pixman_CFLAGS=-I${dest}/include/pixman-1
export pixman_LIBS=${dest}/lib/libpixman-1.a
export FREETYPE_CFLAGS=-I${cwd}/../freetype/include/
export FREETYPE_LIBS=${dest}/lib/libfreetype.a

./configure --prefix=${dest} --libdir=${dest}/lib \
  --enable-static --with-pic --disable-quartz --enable-ft --disable-fc --disable-ps \
  --disable-pdf --disable-interpreter  --without-x --disable-xlib \
  --disable-xlib-xrender --disable-xcb --disable-xlib-xcb
# Building the Cairo test suite may fail
make -j4 || true
make install || true

cd ${cwd}

