#.rst:
# FindExpat
# ---------
#
# Find the Expat XML Parser library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Expat::Expat``
#   The Expat library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``EXPAT_INCLUDE_DIRS``
#   where to find expat.h, etc.
# ``EXPAT_LIBRARIES``
#   the libraries to link against to use Expat.
# ``EXPAT_FOUND``
#   If false, do not try to use Expat.

if(NOT EXPAT_INCLUDE_DIR)
  find_path(EXPAT_INCLUDE_DIR expat.h)
endif()

if(NOT EXPAT_LIBRARY)
  find_library(EXPAT_LIBRARY NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}expat${GR_THIRDPARTY_LIBRARY_SUFFIX} expat libexpat)
endif()

if(EXPAT_INCLUDE_DIR)
  if(NOT EXPAT_VERSION_STRING)
    file(READ ${EXPAT_INCLUDE_DIR}/expat.h EXPAT_H_TEXT)
    string(REGEX REPLACE ".*#define XML_MAJOR_VERSION[ \t]*([0-9]+).*" "\\1" EXPAT_VERSION_MAJOR ${EXPAT_H_TEXT})
    string(REGEX REPLACE ".*#define XML_MINOR_VERSION[ \t]*([0-9]+).*" "\\1" EXPAT_VERSION_MINOR ${EXPAT_H_TEXT})
    string(REGEX REPLACE ".*#define XML_MICRO_VERSION[ \t]*([0-9]+).*" "\\1" EXPAT_VERSION_MICRO ${EXPAT_H_TEXT})
    string(CONCAT EXPAT_VERSION_STRING "${EXPAT_VERSION_MAJOR}.${EXPAT_VERSION_MINOR}.${EXPAT_VERSION_MICRO}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Expat
  VERSION_VAR
  EXPAT_VERSION_STRING
  REQUIRED_VARS
  EXPAT_LIBRARY
  EXPAT_INCLUDE_DIR
  EXPAT_VERSION_STRING
)

if(EXPAT_FOUND)
  set(EXPAT_INCLUDE_DIRS ${EXPAT_INCLUDE_DIR})
  set(EXPAT_LIBRARY ${EXPAT_LIBRARY})

  if(NOT TARGET Expat::Expat)
    add_library(Expat::Expat UNKNOWN IMPORTED)
    set_target_properties(
      Expat::Expat
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${EXPAT_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${EXPAT_LIBRARY}"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(EXPAT_INCLUDE_DIR EXPAT_LIBRARY)
