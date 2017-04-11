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
    cmd="curl -k -O"
  else
    cmd="wget"
  fi
  ${cmd} http://downloads.xiph.org/releases/theora/${src}.tar.gz
  tar -xf ${src}.tar.gz
fi
patch -N -p0 --dry-run --silent &>/dev/null <${cwd}/libtheora.patch &&\
patch -p0 <${cwd}/libtheora.patch

cd ${src}

./configure --prefix=${dest} --libdir=${dest}/lib --disable-shared --with-pic \
  --disable-spec --disable-examples --includedir=${dest}/include \
  --with-ogg-includes=${dest}/include --with-ogg-libraries=${dest}/lib \
  ${THEORA_EXTRA_CONFIGURE_FLAGS}
make -j4
make install

cd ${cwd}

