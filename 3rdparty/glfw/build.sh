#!/bin/sh
cwd=`pwd`
src="3.1.1"
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

export PATH=${dest}/bin:${PATH}
if [ ! `which cmake` ]; then exit 0; fi

mkdir -p ${dest}/src
cd ${dest}/src

if [ ! -d "glfw-${src}" ]; then
  if [ `which curl` ]; then
    cmd="curl -k -O -L"
  else
    cmd="wget --no-check-certificate"
  fi
  ${cmd} https://github.com/glfw/glfw/archive/${src}.tar.gz
  tar -xf ${src}.tar.gz
fi

cd glfw-${src}

cmake -DCMAKE_INSTALL_PREFIX:PATH=${dest} -DGLFW_USE_RETINA=Off .
make -j4
make install

cd ${cwd}

