#.rst:
# FindCairo
# ---------
#
# Find the Cairo library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Cairo::Cairo``
#   The Cairo library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``CAIRO_INCLUDE_DIRS``
#   where to find cairo.h, etc.
# ``CAIRO_LIBRARIES``
#   the libraries to link against to use Cairo.
# ``Cairo_FOUND``
#   If false, do not try to use Cairo.

find_package(Pixman)
find_package(Libpng)

if(NOT CAIRO_INCLUDE_DIR)
  find_path(CAIRO_INCLUDE_DIR cairo/cairo.h)
endif()

if(NOT CAIRO_LIBRARY)
  find_library(CAIRO_LIBRARY NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}cairo${GR_THIRDPARTY_LIBRARY_SUFFIX} cairo)
endif()

find_path(CAIRO_VERSION_DIR cairo/cairo-version.h)
if(CAIRO_VERSION_DIR)
  if(NOT CAIRO_VERSION_STRING)
    file(READ ${CAIRO_VERSION_DIR}/cairo/cairo-version.h CAIRO_H_TEXT)
    string(REGEX REPLACE ".*#define CAIRO_VERSION_MAJOR[ \t]*([0-9]+).*" "\\1." CAIRO_MAJOR_STRING ${CAIRO_H_TEXT})
    string(REGEX REPLACE ".*#define CAIRO_VERSION_MINOR[ \t]*([0-9]+).*" "\\1." CAIRO_MINOR_STRING ${CAIRO_H_TEXT})
    string(REGEX REPLACE ".*#define CAIRO_VERSION_MICRO[ \t]*([0-9]+).*" "\\1" CAIRO_MICRO_STRING ${CAIRO_H_TEXT})
    string(CONCAT CAIRO_VERSION_STRING "${CAIRO_MAJOR_STRING}" "${CAIRO_MINOR_STRING}" "${CAIRO_MICRO_STRING}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Cairo
  VERSION_VAR
  CAIRO_VERSION_STRING
  REQUIRED_VARS
  CAIRO_LIBRARY
  CAIRO_INCLUDE_DIR
  Pixman_FOUND
  Libpng_FOUND
  CAIRO_VERSION_STRING
)

if(Cairo_FOUND)
  set(CAIRO_INCLUDE_DIRS ${CAIRO_INCLUDE_DIR})
  set(CAIRO_LIBRARIES ${CAIRO_LIBRARY})

  if(NOT TARGET Cairo::Cairo)
    add_library(Cairo::Cairo UNKNOWN IMPORTED)
    set_target_properties(
      Cairo::Cairo
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CAIRO_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${CAIRO_LIBRARY}"
                 INTERFACE_LINK_LIBRARIES "Pixman::Pixman;Libpng::Libpng"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(CAIRO_INCLUDE_DIR CAIRO_LIBRARY)
