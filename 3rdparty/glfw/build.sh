#!/bin/sh

if [ ! `which cmake` ]; then exit 0; fi

src="3.1.1"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

if [ `which curl` ]; then
  cmd="curl -O -L"
else
  cmd="wget"
fi
${cmd} https://github.com/glfw/glfw/archive/${src}.tar.gz

tar xf ${src}.tar.gz

cd glfw-${src}

cmake -DCMAKE_INSTALL_PREFIX:PATH=${dest} -DGLFW_USE_RETINA=Off .
make -j4
make install
make clean

cd ..

rm -rf glfw-${src} ${src}.tar.gz

