#!/bin/sh
cwd=`pwd`

src="libvpx-1.4.0"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi
export PATH=${dest}/bin:${PATH}
mkdir -p ${dest}/src
cd ${dest}/src

if [ ! -d "${src}" ]; then
  if [ `which curl` ]; then
    cmd="curl -k -O"
  else
    cmd="wget"
  fi
  ${cmd} http://downloads.webmproject.org/releases/webm/${src}.tar.bz2
  tar -xf ${src}.tar.bz2
fi

cd ${src}

export CFLAGS="-fPIC"
./configure --prefix=${dest} --disable-unit-tests --target=generic-gnu \
  --enable-pic
make -j4
make install

cd ${cwd}

