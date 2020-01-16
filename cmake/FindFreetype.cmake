#.rst:
# FindFreetype
# ------------
#
# Find the Freetype font renderer library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Freetype::Freetype``
#   The Freetype library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``FREETYPE_INCLUDE_DIRS``
#   where to find ft2build.h, etc.
# ``FREETYPE_LIBRARIES``
#   the libraries to link against to use Freetype.
# ``Freetype_FOUND``
#   If false, do not try to use Freetype.

find_package(Libpng)

if(NOT FT2BUILD_INCLUDE_DIR)
  find_path(FT2BUILD_INCLUDE_DIR ft2build.h PATH_SUFFIXES freetype2)
endif()

if(NOT FREETYPE_INCLUDE_DIR)
  find_path(FREETYPE_INCLUDE_DIR freetype/freetype.h PATH_SUFFIXES freetype2)
endif()

if(NOT FREETYPE_LIBRARY)
  find_library(FREETYPE_LIBRARY NAMES freetype)
endif()

if(FREETYPE_INCLUDE_DIR)
  if(NOT FREETYPE_VERSION_STRING AND EXISTS ${FREETYPE_INCLUDE_DIR}/freetype/freetype.h)
    file(READ ${FREETYPE_INCLUDE_DIR}/freetype/freetype.h FREETYPE_H_TEXT)
    string(REGEX REPLACE ".*#define FREETYPE_MAJOR[ \t]*([0-9]+).*" "\\1" FREETYPE_MAJOR_STRING ${FREETYPE_H_TEXT})
    string(REGEX REPLACE ".*#define FREETYPE_MINOR[ \t]*([0-9]+).*" "\\1" FREETYPE_MINOR_STRING ${FREETYPE_H_TEXT})
    string(REGEX REPLACE ".*#define FREETYPE_PATCH[ \t]*([0-9]+).*" "\\1" FREETYPE_PATCH_STRING ${FREETYPE_H_TEXT})
    set(FREETYPE_VERSION_STRING "${FREETYPE_MAJOR_STRING}.${FREETYPE_MINOR_STRING}.${FREETYPE_PATCH_STRING}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Freetype
  VERSION_VAR
  FREETYPE_VERSION_STRING
  REQUIRED_VARS
  FREETYPE_LIBRARY
  FT2BUILD_INCLUDE_DIR
  FREETYPE_INCLUDE_DIR
  Libpng_FOUND
  FREETYPE_VERSION_STRING
)

if(Freetype_FOUND)
  set(FREETYPE_INCLUDE_DIRS "${FT2BUILD_INCLUDE_DIR};${FREETYPE_INCLUDE_DIR}")
  set(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY})

  if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype UNKNOWN IMPORTED)
    set_target_properties(
      Freetype::Freetype
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${FREETYPE_LIBRARY}"
                 INTERFACE_LINK_LIBRARIES "Libpng::Libpng"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(FREETYPE_INCLUDE_DIR FT2BUILD_INCLUDE_DIR FREETYPE_LIBRARY)
