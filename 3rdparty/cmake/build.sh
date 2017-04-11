#!/bin/sh
cwd=`pwd`
if [ `which cmake >/dev/null 2>&1` ]; then exit 0; fi

src="cmake-2.8.12.2"
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
  ${cmd} https://cmake.org/files/v2.8/${src}.tar.gz
  tar -xf ${dest}/src/${src}.tar.gz
fi

cd ${src}

./bootstrap --prefix=${dest}
make -j4
make install

cd ${cwd}

