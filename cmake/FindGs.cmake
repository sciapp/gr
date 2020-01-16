#.rst:
# FindGs
# ------
#
# Find the Ghostscript postscript and pdf renderer library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Gs::Gs``
#   The Gs library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``GS_INCLUDE_DIRS``
#   where to find ghostscript/iapi.h, etc.
# ``GS_LIBRARIES``
#   the libraries to link against to use Gs.
# ``Gs_FOUND``
#   If false, do not try to use Gs.

if(NOT GS_INCLUDE_DIR)
  find_path(GS_INCLUDE_DIR ghostscript/iapi.h)
endif()

if(NOT GS_LIBRARY)
  find_library(GS_LIBRARY NAMES gs)
endif()

find_path(GS_VERSION_DIR ghostscript/gdevdsp.h)
if(GS_VERSION_DIR)
  if(NOT GS_VERSION_STRING)
    file(READ ${GS_VERSION_DIR}/ghostscript/gdevdsp.h GS_H_TEXT)
    string(REGEX REPLACE ".*#define DISPLAY_VERSION_MAJOR[ \t]*([0-9]+).*" "\\1." GS_MAJOR_STRING ${GS_H_TEXT})
    string(REGEX REPLACE ".*#define DISPLAY_VERSION_MINOR[ \t]*([0-9]+).*" "\\1" GS_MINOR_STRING ${GS_H_TEXT})
    string(CONCAT GS_VERSION_STRING "${GS_MAJOR_STRING}" "${GS_MINOR_STRING}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Gs
  VERSION_VAR
  GS_VERSION_STRING
  REQUIRED_VARS
  GS_LIBRARY
  GS_INCLUDE_DIR
  GS_VERSION_STRING
)

if(Gs_FOUND)
  set(GS_INCLUDE_DIRS ${GS_INCLUDE_DIR})
  set(GS_LIBRARIES ${GS_LIBRARY})

  if(NOT TARGET Gs::Gs)
    add_library(Gs::Gs UNKNOWN IMPORTED)
    set_target_properties(
      Gs::Gs
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GS_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${GS_LIBRARY}"
    )
    if(APPLE)
      find_package(Iconv REQUIRED)
      if(Iconv_FOUND)
        set_target_properties(Gs::Gs PROPERTIES INTERFACE_LINK_LIBRARIES "Iconv::Iconv")
      endif()
    endif()
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(GS_INCLUDE_DIR GS_LIBRARY)
