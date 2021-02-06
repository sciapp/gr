set prefix=S:/opt/msvc2019_64
cmake -D CMAKE_INSTALL_PREFIX=%prefix% ^
  -D CMAKE_LIBRARY_PATH=%prefix%/lib ^
  -D CMAKE_INCLUDE_PATH=%prefix%/include ^
  -D ZLIB_LIBRARY=%prefix%/lib/zlib.lib ^
  -D LIBPNG_LIBRARY=%prefix%/lib/libpng16.lib ^
  -D FREETYPE_LIBRARY=%prefix%/lib/freetype.lib ^
  -D JPEG_LIBRARY=%prefix%/lib/Libjpeg.lib ^
  -D QHULL_LIBRARY=%prefix%/lib/qhullstatic.lib ^
  -D BZip2_LIBRARY=%prefix%/lib/bz2.lib ^
  ..
