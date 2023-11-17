#.rst:
# FindXercesC
# -----------
#
# Find the Xerces-C++ XML Parser library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``XercesC::XercesC``
#   The Xerces-C++ library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``XERCES_C_INCLUDE_DIRS``
#   where to find Xerces-C++ header files, etc.
# ``XERCES_C_LIBRARIES``
#   the libraries to link against to use Xerces-C++.
# ``XERCES_C_FOUND``
#   If false, do not try to use Xerces-C++.

if(NOT XERCES_C_INCLUDE_DIR)
  find_path(XERCES_C_UTIL_DIR XercesVersion.hpp PATH_SUFFIXES xercesc/util)
  if(XERCES_C_UTIL_DIR)
    get_filename_component(XERCES_C_INCLUDE_DIR "${XERCES_C_UTIL_DIR}/../.." ABSOLUTE)
  endif()
endif()

if(NOT XERCES_C_LIBRARY)
  find_library(
    XERCES_C_LIBRARY NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}xerces-c${GR_THIRDPARTY_LIBRARY_SUFFIX} xerces-c libxerces-c
  )
endif()

if(XERCES_C_INCLUDE_DIR)
  if(NOT XERCES_C_VERSION_STRING)
    file(READ ${XERCES_C_INCLUDE_DIR}/xercesc/util/XercesVersion.hpp XERCES_VERSION_H_TEXT)
    string(REGEX REPLACE ".*#define XERCES_VERSION_MAJOR[ \t]*([0-9]+).*" "\\1" XERCES_C_VERSION_MAJOR
                         ${XERCES_VERSION_H_TEXT}
    )
    string(REGEX REPLACE ".*#define XERCES_VERSION_MINOR[ \t]*([0-9]+).*" "\\1" XERCES_C_VERSION_MINOR
                         ${XERCES_VERSION_H_TEXT}
    )
    string(REGEX REPLACE ".*#define XERCES_VERSION_REVISION[ \t]*([0-9]+).*" "\\1" XERCES_C_VERSION_REVISION
                         ${XERCES_VERSION_H_TEXT}
    )
    string(CONCAT XERCES_C_VERSION_STRING
                  "${XERCES_C_VERSION_MAJOR}.${XERCES_C_VERSION_MINOR}.${XERCES_C_VERSION_REVISION}"
    )
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  XERCES_C
  VERSION_VAR
  XERCES_C_VERSION_STRING
  REQUIRED_VARS
  XERCES_C_LIBRARY
  XERCES_C_INCLUDE_DIR
  XERCES_C_VERSION_STRING
)

if(XERCES_C_FOUND)
  set(XERCES_C_INCLUDE_DIRS ${XERCES_C_INCLUDE_DIR})
  set(XERCES_C_LIBRARY ${XERCES_C_LIBRARY})

  if(NOT TARGET XercesC::XercesC)
    add_library(XercesC::XercesC UNKNOWN IMPORTED)
    set_target_properties(
      XercesC::XercesC
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${XERCES_C_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                 IMPORTED_LOCATION "${XERCES_C_LIBRARY}"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(XERCES_C_INCLUDE_DIRS XERCES_C_LIBRARY)
