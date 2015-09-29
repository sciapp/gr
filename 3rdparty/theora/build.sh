#!/bin/sh
cwd=`pwd`
src="libtheora-1.1.1"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi
mkdir -p ${dest}/src
cd ${dest}/src

if [ ! -d "${src}" ]; then
  if [ `which curl` ]; then
    cmd="curl -O"
  else
    cmd="wget"
  fi
  ${cmd} http://downloads.xiph.org/releases/theora/${src}.tar.gz
  tar -xf ${src}.tar.gz
  patch -p0 <${cwd}/libtheora.patch
fi

cd ${src}

./configure --prefix=${dest} --libdir=${dest}/lib --disable-shared --with-pic \
  --disable-spec --includedir=${dest}/include \
  --with-ogg-includes=${dest}/include --with-ogg-libraries=${dest}/lib
make -j4
make install

cd ${cwd}

