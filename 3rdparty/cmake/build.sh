#!/bin/sh
set -e
cwd=`pwd`
if [ `which cmake` ]; then exit 0; fi

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
    cmd="curl -k -O -L"
  else
    cmd="wget --no-check-certificate"
  fi
  ${cmd} https://gr-framework.org/downloads/3rdparty/${src}.tar.gz
  tar -xf ${dest}/src/${src}.tar.gz
fi

cd ${src}

./bootstrap --prefix=${dest}
make -j4
make install

cd ${cwd}

