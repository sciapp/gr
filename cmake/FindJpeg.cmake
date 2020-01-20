#.rst:
# FindJpeg
# --------
#
# Find the JPEG library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Jpeg::Jpeg``
#   The Jpeg library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``JPEG_INCLUDE_DIRS``
#   where to find jpeglib.h, etc.
# ``JPEG_LIBRARIES``
#   the libraries to link against to use Jpeg.
# ``Jpeg_FOUND``
#   If false, do not try to use Jpeg.

if(NOT JPEG_INCLUDE_DIR)
  find_path(JPEG_INCLUDE_DIR jpeglib.h)
endif()

if(NOT JPEG_LIBRARY)
  find_library(JPEG_LIBRARY NAMES jpeg)
endif()

if(NOT JCONFIG_INCLUDE_DIR)
  find_path(JCONFIG_INCLUDE_DIR jconfig.h)
endif()

if(JPEG_INCLUDE_DIR AND JCONFIG_INCLUDE_DIR)
  if(NOT JPEG_VERSION_STRING)
    file(READ ${JPEG_INCLUDE_DIR}/jpeglib.h JPEG_H_TEXT)
    if(JPEG_H_TEXT MATCHES "#define JPEG_LIB_VERSION_MAJOR")
      string(REGEX REPLACE ".*#define JPEG_LIB_VERSION_MAJOR[ \t]*([0-9]+).*" "\\1" JPEG_MAJOR_STRING ${JPEG_H_TEXT})
      string(REGEX REPLACE ".*#define JPEG_LIB_VERSION_MINOR[ \t]*([0-9]+).*" "\\1" JPEG_MINOR_STRING ${JPEG_H_TEXT})
      set(JPEG_VERSION_STRING "${JPEG_MAJOR_STRING}.${JPEG_MINOR_STRING}")
    else()
      file(GLOB _JPEG_CONFIG_HEADERS_FEDORA "${JPEG_INCLUDE_DIR}/jconfig*.h")
      file(GLOB _JPEG_CONFIG_HEADERS_DEBIAN "${JPEG_INCLUDE_DIR}/*/jconfig.h")
      set(_JPEG_CONFIG_HEADERS ${_JPEG_CONFIG_HEADERS_FEDORA} ${_JPEG_CONFIG_HEADERS_DEBIAN})
      foreach(_JPEG_CONFIG_HEADER IN LISTS _JPEG_CONFIG_HEADERS)
        if(NOT EXISTS "${_JPEG_CONFIG_HEADER}")
          continue()
        endif()
        file(READ "${_JPEG_CONFIG_HEADER}" JCONFIG_H_TEXT)
        if(JCONFIG_H_TEXT MATCHES "#define JPEG_LIB_VERSION")
          string(REGEX REPLACE ".*#define JPEG_LIB_VERSION[ \t]*([0-9]+).*" "\\1" JPEG_VERSION_STRING ${JCONFIG_H_TEXT})
          break()
        endif()
      endforeach()
      unset(_JPEG_CONFIG_HEADER)
      unset(_JPEG_CONFIG_HEADERS)
      unset(_JPEG_CONFIG_HEADERS_FEDORA)
      unset(_JPEG_CONFIG_HEADERS_DEBIAN)
    endif()
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Jpeg
  VERSION_VAR
  JPEG_VERSION_STRING
  REQUIRED_VARS
  JPEG_LIBRARY
  JPEG_INCLUDE_DIR
  JPEG_VERSION_STRING
)

if(Jpeg_FOUND)
  set(JPEG_INCLUDE_DIRS ${JPEG_INCLUDE_DIR})
  set(JPEG_LIBRARIES ${JPEG_LIBRARY})

  if(NOT TARGET Jpeg::Jpeg)
    add_library(Jpeg::Jpeg UNKNOWN IMPORTED)
    set_target_properties(
      Jpeg::Jpeg
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${JPEG_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${JPEG_LIBRARY}"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(JPEG_INCLUDE_DIR JCONFIG_INCLUDE_DIR JPEG_LIBRARY)
