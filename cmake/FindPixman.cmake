#.rst:
# FindPixman
# ----------
#
# Find the Pixman library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Pixman::Pixman``
#   The Pixman library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``PIXMAN_LIBRARIES``
#   the libraries to link against to use Pixman.
# ``Pixman_FOUND``
#   If false, do not try to use Pixman.

if(NOT PIXMAN_LIBRARY)
  find_library(PIXMAN_LIBRARY NAMES pixman-1)
endif()

if(NOT PIXMAN_INCLUDE_DIR)
  find_path(PIXMAN_INCLUDE_DIR pixman-1/pixman-version.h)
endif()

if(PIXMAN_INCLUDE_DIR)
  if(NOT PIXMAN_VERSION_STRING)
    file(READ ${PIXMAN_INCLUDE_DIR}/pixman-1/pixman-version.h PIXMAN_H_TEXT)
    string(REGEX REPLACE ".*#define PIXMAN_VERSION_MAJOR[ \t]*([0-9]+).*" "\\1." PIXMAN_MAJOR_STRING ${PIXMAN_H_TEXT})
    string(REGEX REPLACE ".*#define PIXMAN_VERSION_MINOR[ \t]*([0-9]+).*" "\\1." PIXMAN_MINOR_STRING ${PIXMAN_H_TEXT})
    string(REGEX REPLACE ".*#define PIXMAN_VERSION_MICRO[ \t]*([0-9]+).*" "\\1" PIXMAN_MICRO_STRING ${PIXMAN_H_TEXT})
    string(CONCAT PIXMAN_VERSION_STRING "${PIXMAN_MAJOR_STRING}" "${PIXMAN_MINOR_STRING}" "${PIXMAN_MICRO_STRING}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Pixman
  VERSION_VAR
  PIXMAN_VERSION_STRING
  REQUIRED_VARS
  PIXMAN_LIBRARY
  PIXMAN_VERSION_STRING
)

if(Pixman_FOUND)
  set(PIXMAN_LIBRARIES ${PIXMAN_LIBRARY})

  if(NOT TARGET Pixman::Pixman)
    add_library(Pixman::Pixman UNKNOWN IMPORTED)
    set_target_properties(
      Pixman::Pixman
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${PIXMAN_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${PIXMAN_LIBRARY}"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(PIXMAN_LIBRARY)
