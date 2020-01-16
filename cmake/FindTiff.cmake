#.rst:
# FindTiff
# --------
#
# Find the Tiff library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Tiff::Tiff``
#   The Tiff library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``TIFF_INCLUDE_DIRS``
#   where to find tiff.h, etc.
# ``TIFF_LIBRARIES``
#   the libraries to link against to use Tiff.
# ``Tiff_FOUND``
#   If false, do not try to use Tiff.

if(NOT TIFF_INCLUDE_DIR)
  find_path(TIFF_INCLUDE_DIR tiff.h)
endif()

if(NOT TIFF_LIBRARY)
  find_library(TIFF_LIBRARY NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}tiff${GR_THIRDPARTY_LIBRARY_SUFFIX} tiff)
endif()

find_path(TIFF_VERSION_DIR tiffvers.h)
if(TIFF_INCLUDE_DIR AND TIFF_VERSION_DIR)
  if(NOT TIFF_VERSION_STRING)
    file(READ ${TIFF_VERSION_DIR}/tiffvers.h TIFF_H_TEXT)
    string(REGEX REPLACE ".*#define TIFFLIB_VERSION_STR \"[ \tA-Za-z,]*([0-9]+.[0-9]+.[0-9]+).*\".*" "\\1"
                         TIFF_VERSION_STRING ${TIFF_H_TEXT}
    )
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Tiff
  VERSION_VAR
  TIFF_VERSION_STRING
  REQUIRED_VARS
  TIFF_LIBRARY
  TIFF_INCLUDE_DIR
  TIFF_VERSION_STRING
)

if(Tiff_FOUND)
  set(TIFF_INCLUDE_DIRS ${TIFF_INCLUDE_DIR})
  set(TIFF_LIBRARIES ${TIFF_LIBRARY})

  if(NOT TARGET Tiff::Tiff)
    add_library(Tiff::Tiff UNKNOWN IMPORTED)
    set_target_properties(
      Tiff::Tiff
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${TIFF_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${TIFF_LIBRARY}"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(TIFF_INCLUDE_DIR TIFF_LIBRARY)
