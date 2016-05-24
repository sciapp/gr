#!/bin/sh

if [ $# = 0 ]; then
  opts="xft=no"
  if [ "`which wx-config 2>/dev/null`" = "" ]; then
    opts="${opts} wx=no"
  fi
  if [ "`which gtk-demo 2>/dev/null`" = "" ]; then
    opts="${opts} gtk=no"
  fi
else
  opts="$*"
fi
if [ `uname` = 'Darwin' ]; then
  opts="${opts} x11=no"
fi
if [ ! -z "${DESTDIR}" ]; then
  opts="${opts} DESTDIR=${DESTDIR}"
  unset DESTDIR     # important because DESTDIR is also used by 3rd party build scripts
fi
if [ ! -z "${GRDIR}" ]; then
  opts="${opts} GRDIR=${GRDIR}"
  gr_lib="${GRDIR}/lib"
else
  gr_lib="/usr/local/gr/lib"
fi
if [ -z "${QTDIR}" ]; then
  for dir in ${HOME}/anaconda /opt/anaconda /usr/local/anaconda
  do
    if [ -d ${dir} ]; then
      export QTDIR=${dir}
      break
    fi
  done
fi

extras=`pwd`/3rdparty/build
extras_lib=${extras}/lib
export PATH=${PATH}:${extras}/bin

if [ "`uname`" != "Darwin" ]; then
  opts="${opts} USE_STATIC_CAIRO_LIBS=1"
fi

mkdir -p ${extras_lib}
make -C 3rdparty
cp -p 3rdparty/freetype/libfreetype.a ${extras_lib}/
cp -p 3rdparty/jpeg/libjpeg.a ${extras_lib}/
cp -p 3rdparty/libpng16/libpng.a ${extras_lib}/
cp -p 3rdparty/zlib/libz.a ${extras_lib}/
cp -p 3rdparty/qhull/libqhull.a ${extras_lib}/

make -C 3rdparty extras
make EXTRA_CFLAGS=-I${extras}/include \
     EXTRA_CXXFLAGS=-I${extras}/include \
     EXTRA_LDFLAGS=-L${extras_lib} \
     ${opts} install

if [ "`uname`" = "Darwin" ]; then
  cp -p ${extras_lib}/libcairo.2.dylib ${gr_lib}/
  install_name_tool -id @rpath/libcairo.2.dylib ${gr_lib}/libcairo.2.dylib
  old=`otool -L ${gr_lib}/cairoplugin.so|grep libcairo.2.dylib|awk '{print $1}'`
  install_name_tool -change ${old} @rpath/libcairo.2.dylib \
    ${gr_lib}/cairoplugin.so
fi
