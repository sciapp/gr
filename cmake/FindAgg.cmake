#.rst:
# FindAgg
# -------
#
# Find the Agg library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Agg::Agg``
#   The Agg library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``AGG_INCLUDE_DIRS``
#   where to find agg_basics.h, etc.
# ``AGG_LIBRARIES``
#   the libraries to link against to use Agg.
# ``Agg_FOUND``
#   If false, do not try to use Agg.

if(NOT AGG_INCLUDE_DIR)
  find_path(AGG_INCLUDE_DIR agg_basics.h PATH_SUFFIXES agg2)
endif()

if(NOT AGG_LIBRARY)
  find_library(AGG_LIBRARY NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}agg${GR_THIRDPARTY_LIBRARY_SUFFIX} agg)
endif()

if(AGG_INCLUDE_DIR)
  if(NOT AGG_VERSION_STRING)
    file(READ ${AGG_INCLUDE_DIR}/agg_basics.h AGG_BASICS_H_TEXT)
    string(REGEX REPLACE ".*Anti-Grain Geometry - Version[ \tA-Za-z,]*([0-9]+.[0-9]+).*" "\\1" AGG_VERSION_STRING
                         ${AGG_BASICS_H_TEXT}
    )
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Agg
  VERSION_VAR
  AGG_VERSION_STRING
  REQUIRED_VARS
  AGG_LIBRARY
  AGG_INCLUDE_DIR
  AGG_VERSION_STRING
)

if(Agg_FOUND)
  set(AGG_INCLUDE_DIRS ${AGG_INCLUDE_DIR})
  set(AGG_LIBRARIES ${AGG_LIBRARY})

  if(NOT TARGET Agg::Agg)
    add_library(Agg::Agg UNKNOWN IMPORTED)
    set_target_properties(
      Agg::Agg
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${AGG_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                 IMPORTED_LOCATION "${AGG_LIBRARY}"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(AGG_INCLUDE_DIR AGG_LIBRARY)
