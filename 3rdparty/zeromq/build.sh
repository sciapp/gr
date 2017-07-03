#!/bin/sh
set -e
cwd=`pwd`
src="zeromq-4.0.4"
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
    cmd="wget"
  fi
  ${cmd} https://gr-framework.org/downloads/3rdparty/zeromq-4.0.4.tar.gz
  tar -xf ${src}.tar.gz
fi

cd ${src}

export CFLAGS=-fPIC
export CXXFLAGS=-fPIC
opts="SUBDIRS=src"
./configure --prefix=${dest} --libdir=${dest}/lib --disable-shared
make ${opts} -j4
make ${opts} install

cd ${cwd}

