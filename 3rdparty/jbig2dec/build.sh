#!/bin/sh
if [ "$1" = "" ]; then
  dest=`pwd`/../build
else
  dest=$1
fi

make -j4
mkdir -p ${dest}/lib
cp -p libjbig2dec.a ${dest}/lib
mkdir -p ${dest}/include
cp -p jbig2.h ${dest}/include
