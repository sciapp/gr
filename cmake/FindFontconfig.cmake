#.rst:
# FindFontconfig
# ------------
#
# Find the Fontconfig library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Fontconfig::Fontconfig``
#   The Fontconfig library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``FONTCONFIG_INCLUDE_DIRS``
#   where to find fontconfig/fontconfig.h, etc.
# ``FONTCONFIG_LIBRARIES``
#   the libraries to link against to use Fontconfig.
# ``Fontconfig_FOUND``
#   If false, do not try to use Fontconfig.

if(NOT FONTCONFIG_INCLUDE_DIR)
  find_path(FONTCONFIG_INCLUDE_DIR fontconfig/fontconfig.h)
endif()

if(NOT FONTCONFIG_LIBRARY)
  find_library(FONTCONFIG_LIBRARY NAMES fontconfig)
endif()

if(FONTCONFIG_INCLUDE_DIR)
  if(NOT FONTCONFIG_VERSION_STRING)
    file(READ ${FONTCONFIG_INCLUDE_DIR}/fontconfig/fontconfig.h FONTCONFIG_H_TEXT)
    string(REGEX REPLACE ".*#define FC_MAJOR[ \t]*([0-9]+).*" "\\1." FC_MAJOR_STRING ${FONTCONFIG_H_TEXT})
    string(REGEX REPLACE ".*#define FC_MINOR[ \t]*([0-9]+).*" "\\1." FC_MINOR_STRING ${FONTCONFIG_H_TEXT})
    string(REGEX REPLACE ".*#define FC_REVISION[ \t]*([0-9]+).*" "\\1" FC_REVISION_STRING ${FONTCONFIG_H_TEXT})
    string(CONCAT FONTCONFIG_VERSION_STRING "${FC_MAJOR_STRING}" "${FC_MINOR_STRING}" "${FC_REVISION_STRING}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Fontconfig
  VERSION_VAR
  FONTCONFIG_VERSION_STRING
  REQUIRED_VARS
  FONTCONFIG_LIBRARY
  FONTCONFIG_INCLUDE_DIR
  FONTCONFIG_VERSION_STRING
)

if(Fontconfig_FOUND)
  set(FONTCONFIG_INCLUDE_DIRS ${FONTCONFIG_INCLUDE_DIR})
  set(FONTCONFIG_LIBRARIES ${FONTCONFIG_LIBRARY})

  if(NOT TARGET Fontconfig::Fontconfig)
    add_library(Fontconfig::Fontconfig UNKNOWN IMPORTED)
    set_target_properties(
      Fontconfig::Fontconfig
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FONTCONFIG_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${FONTCONFIG_LIBRARY}"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(FONTCONFIG_INCLUDE_DIR FONTCONFIG_LIBRARY)
