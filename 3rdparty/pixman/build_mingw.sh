#!/bin/sh
cwd=`pwd`
src="pixman-0.34.0"
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
  ${cmd} https://cairographics.org/releases/${src}.tar.gz
  tar -xf ${src}.tar.gz
fi

cd ${src}

export MAKE=make
./configure --prefix=${dest} --libdir=${dest}/lib \
  --disable-shared --enable-static --with-pic --host=x86_64-w64-mingw32
make
make install
export MAKE="make -f makefile.mingw"
cd ${cwd}
