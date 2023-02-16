#.rst:
# FindFfmpeg
# ----------
#
# Find the Ffmpeg video library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Ffmpeg::Ffmpeg``
#   The Ffmpeg library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``FFMPEG_INCLUDE_DIRS``
#   where to find libavcodec/avcodec.h, etc.
# ``FFMPEG_LIBRARIES``
#   the libraries to link against to use Ffmpeg.
# ``Ffmpeg_FOUND``
#   If false, do not try to use Ffmpeg.

find_package(Zlib)

if(NOT FFMPEG_INCLUDE_DIR)
  find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h)
endif()

if(NOT FFMPEG_LIBRARY_AVFORMAT)
  find_library(
    FFMPEG_LIBRARY_AVFORMAT NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}avformat${GR_THIRDPARTY_LIBRARY_SUFFIX} avformat
  )
endif()

if(NOT FFMPEG_LIBRARY_AVCODEC)
  find_library(
    FFMPEG_LIBRARY_AVCODEC NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}avcodec${GR_THIRDPARTY_LIBRARY_SUFFIX} avcodec
  )
endif()

if(NOT FFMPEG_LIBRARY_SWSCALE)
  find_library(
    FFMPEG_LIBRARY_SWSCALE NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}swscale${GR_THIRDPARTY_LIBRARY_SUFFIX} swscale
  )
endif()

if(NOT FFMPEG_LIBRARY_AVUTIL)
  find_library(FFMPEG_LIBRARY_AVUTIL NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}avutil${GR_THIRDPARTY_LIBRARY_SUFFIX} avutil)
endif()

if(NOT FFMPEG_LIBRARY_THEORA)
  find_library(FFMPEG_LIBRARY_THEORA NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}theora${GR_THIRDPARTY_LIBRARY_SUFFIX} theora)
endif()

if(NOT FFMPEG_LIBRARY_OGG)
  find_library(FFMPEG_LIBRARY_OGG NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}ogg${GR_THIRDPARTY_LIBRARY_SUFFIX} ogg)
endif()

if(NOT FFMPEG_LIBRARY_VPX)
  find_library(FFMPEG_LIBRARY_VPX NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}vpx${GR_THIRDPARTY_LIBRARY_SUFFIX} vpx)
endif()

if(NOT FFMPEG_LIBRARY_OPENH264)
  find_library(
    FFMPEG_LIBRARY_OPENH264 NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}openh264${GR_THIRDPARTY_LIBRARY_SUFFIX} openh264
  )
endif()

find_path(FFMPEG_VERSION_DIR libavcodec/version.h)
if(FFMPEG_VERSION_DIR)
  if(NOT FFMPEG_VERSION_STRING)
    if(EXISTS ${FFMPEG_VERSION_DIR}/libavcodec/version_major.h)
      file(READ ${FFMPEG_VERSION_DIR}/libavcodec/version_major.h FFMPEG_VERSION_MAJOR_H_TEXT)
      string(REGEX REPLACE ".*#define LIBAVCODEC_VERSION_MAJOR[ \t]*([0-9]+).*" "\\1." FFMPEG_MAJOR_STRING
                           ${FFMPEG_VERSION_MAJOR_H_TEXT}
      )
    endif()
    file(READ ${FFMPEG_VERSION_DIR}/libavcodec/version.h FFMPEG_VERSION_H_TEXT)
    if(NOT FFMPEG_VERSION_MAJOR_H_TEXT)
      string(REGEX REPLACE ".*#define LIBAVCODEC_VERSION_MAJOR[ \t]*([0-9]+).*" "\\1." FFMPEG_MAJOR_STRING
                           ${FFMPEG_VERSION_H_TEXT}
      )
    endif()
    string(REGEX REPLACE ".*#define LIBAVCODEC_VERSION_MINOR[ \t]*([0-9]+).*" "\\1." FFMPEG_MINOR_STRING
                         ${FFMPEG_VERSION_H_TEXT}
    )
    string(REGEX REPLACE ".*#define LIBAVCODEC_VERSION_MICRO[ \t]*([0-9]+).*" "\\1" FFMPEG_MICRO_STRING
                         ${FFMPEG_VERSION_H_TEXT}
    )
    string(CONCAT FFMPEG_VERSION_STRING "${FFMPEG_MAJOR_STRING}" "${FFMPEG_MINOR_STRING}" "${FFMPEG_MICRO_STRING}")
  endif()
endif()

if(FFMPEG_INCLUDE_DIR
   AND FFMPEG_LIBRARY_AVCODEC
   AND FFMPEG_LIBRARY_AVFORMAT
   AND FFMPEG_LIBRARY_AVUTIL
   AND FFMPEG_LIBRARY_SWSCALE
)
  if(NOT TARGET Ffmpeg::Avformat)
    add_library(Ffmpeg::Avformat UNKNOWN IMPORTED)
    set_target_properties(
      Ffmpeg::Avformat
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                 IMPORTED_LOCATION "${FFMPEG_LIBRARY_AVFORMAT}"
                 INTERFACE_LINK_LIBRARIES "Zlib::Zlib"
    )
  endif()

  if(NOT TARGET Ffmpeg::Avcodec)
    add_library(Ffmpeg::Avcodec UNKNOWN IMPORTED)
    set_target_properties(
      Ffmpeg::Avcodec
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                 IMPORTED_LOCATION "${FFMPEG_LIBRARY_AVCODEC}"
                 INTERFACE_LINK_LIBRARIES "Zlib::Zlib"
    )
  endif()

  if(NOT TARGET Ffmpeg::Swscale)
    add_library(Ffmpeg::Swscale UNKNOWN IMPORTED)
    set_target_properties(
      Ffmpeg::Swscale
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                 IMPORTED_LOCATION "${FFMPEG_LIBRARY_SWSCALE}"
    )
  endif()

  if(NOT TARGET Ffmpeg::Avutil)
    add_library(Ffmpeg::Avutil UNKNOWN IMPORTED)
    set_target_properties(
      Ffmpeg::Avutil
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                 IMPORTED_LOCATION "${FFMPEG_LIBRARY_AVUTIL}"
    )
  endif()

  set(FFMPEG_LIBRARIES "Ffmpeg::Avformat;Ffmpeg::Avcodec;Ffmpeg::Swscale;Ffmpeg::Avutil;m;pthread")
  if(APPLE)
    list(
      APPEND
      FFMPEG_LIBRARIES
      "-framework VideoToolbox;-framework CoreVideo;-framework CoreFoundation;-framework CoreServices;-framework CoreMedia"
    )
  endif()

  try_compile(
    FFMPEG_TEST_COMPILED ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_test ${CMAKE_CURRENT_LIST_DIR}/ffmpeg_test.cxx
    CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${FFMPEG_INCLUDE_DIR}"
    LINK_LIBRARIES ${FFMPEG_LIBRARIES}
  )

  if(NOT FFMPEG_TEST_COMPILED
     AND FFMPEG_LIBRARY_THEORA
     AND FFMPEG_LIBRARY_OGG
     AND FFMPEG_LIBRARY_VPX
     AND FFMPEG_LIBRARY_OPENH264
  )
    if(NOT TARGET Ffmpeg::Theora)
      add_library(Ffmpeg::Theora UNKNOWN IMPORTED)
      set_target_properties(
        Ffmpeg::Theora
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}"
                   IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                   IMPORTED_LOCATION "${FFMPEG_LIBRARY_THEORA}"
                   INTERFACE_LINK_LIBRARIES "Zlib::Zlib"
      )
    endif()

    if(NOT TARGET Ffmpeg::Ogg)
      add_library(Ffmpeg::Ogg UNKNOWN IMPORTED)
      set_target_properties(
        Ffmpeg::Ogg
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}"
                   IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                   IMPORTED_LOCATION "${FFMPEG_LIBRARY_OGG}"
      )
    endif()

    if(NOT TARGET Ffmpeg::Vpx)
      add_library(Ffmpeg::Vpx UNKNOWN IMPORTED)
      set_target_properties(
        Ffmpeg::Vpx
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}"
                   IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                   IMPORTED_LOCATION "${FFMPEG_LIBRARY_VPX}"
      )
    endif()

    if(NOT TARGET Ffmpeg::OpenH264)
      add_library(Ffmpeg::OpenH264 UNKNOWN IMPORTED)
      set_target_properties(
        Ffmpeg::OpenH264
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}"
                   IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                   IMPORTED_LOCATION "${FFMPEG_LIBRARY_OPENH264}"
      )
    endif()

    list(APPEND FFMPEG_LIBRARIES "Ffmpeg::Theora;Ffmpeg::Ogg;Ffmpeg::Vpx;Ffmpeg::OpenH264")
    try_compile(
      FFMPEG_TEST_COMPILED ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_test ${CMAKE_CURRENT_LIST_DIR}/ffmpeg_test.cxx
      CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${FFMPEG_INCLUDE_DIR}"
      LINK_LIBRARIES ${FFMPEG_LIBRARIES}
    )
  endif()
else()
  set(FFMPEG_TEST_COMPILED 0)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Ffmpeg
  VERSION_VAR
  FFMPEG_VERSION_STRING
  REQUIRED_VARS
  FFMPEG_TEST_COMPILED
  FFMPEG_LIBRARY_AVFORMAT
  FFMPEG_LIBRARY_AVCODEC
  FFMPEG_LIBRARY_SWSCALE
  FFMPEG_LIBRARY_AVUTIL
)

if(Ffmpeg_FOUND)
  set(FFMPEG_INCLUDE_DIRS "${FFMPEG_INCLUDE_DIR}")

  if(NOT TARGET Ffmpeg::Ffmpeg)
    add_library(Ffmpeg::Ffmpeg INTERFACE IMPORTED)
    set_target_properties(
      Ffmpeg::Ffmpeg PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INCLUDE_DIRS}" INTERFACE_LINK_LIBRARIES
                                                                                       "${FFMPEG_LIBRARIES}"
    )
  endif()
  if(APPLE)
    find_package(Iconv)
    if(Iconv_FOUND)
      target_link_libraries(Ffmpeg::Ffmpeg INTERFACE Iconv::Iconv)
    endif()
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(
  FFMPEG_INCLUDE_DIR
  FFMPEG_LIBRARY_AVFORMAT
  FFMPEG_LIBRARY_AVCODEC
  FFMPEG_LIBRARY_SWSCALE
  FFMPEG_LIBRARY_AVUTIL
  FFMPEG_LIBRARY_THEORA
  FFMPEG_LIBRARY_OGG
  FFMPEG_LIBRARY_VPX
  FFMPEG_LIBRARY_OPENH264
)
