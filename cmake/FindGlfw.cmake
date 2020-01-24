#.rst:
# FindGlfw
# --------
#
# Find the GLFW library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Glfw::Glfw``
#   The Glfw library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``GLFW_INCLUDE_DIRS``
#   where to find GLFW/glfw3.h, etc.
# ``GLFW_LIBRARIES``
#   the libraries to link against to use Glfw.
# ``Glfw_FOUND``
#   If false, do not try to use Glfw.

if(NOT GLFW_INCLUDE_DIR)
  find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h)
endif()

if(NOT GLFW_LIBRARY)
  find_library(GLFW_LIBRARY NAMES glfw3 glfw)
endif()

if(GLFW_INCLUDE_DIR)
  if(NOT GLFW_VERSION_STRING)
    file(READ ${GLFW_INCLUDE_DIR}/GLFW/glfw3.h GLFW_H_TEXT)
    string(REGEX REPLACE ".*#define GLFW_VERSION_MAJOR[ \t]*([0-9]+).*" "\\1." GLFW_MAJOR_STRING ${GLFW_H_TEXT})
    string(REGEX REPLACE ".*#define GLFW_VERSION_MINOR[ \t]*([0-9]+).*" "\\1." GLFW_MINOR_STRING ${GLFW_H_TEXT})
    string(REGEX REPLACE ".*#define GLFW_VERSION_REVISION[ \t]*([0-9]+).*" "\\1" GLFW_REVISION_STRING ${GLFW_H_TEXT})
    string(CONCAT GLFW_VERSION_STRING "${GLFW_MAJOR_STRING}" "${GLFW_MINOR_STRING}" "${GLFW_REVISION_STRING}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Glfw
  VERSION_VAR
  GLFW_VERSION_STRING
  REQUIRED_VARS
  GLFW_LIBRARY
  GLFW_INCLUDE_DIR
  GLFW_VERSION_STRING
)

if(Glfw_FOUND)
  set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
  set(GLFW_LIBRARIES ${GLFW_LIBRARY})

  if(NOT TARGET Glfw::Glfw)
    add_library(Glfw::Glfw UNKNOWN IMPORTED)
    set_target_properties(
      Glfw::Glfw
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${GLFW_LIBRARY}"
    )
    if(APPLE)
      set_target_properties(
        Glfw::Glfw PROPERTIES INTERFACE_LINK_LIBRARIES
                              "-framework Cocoa -framework IOKit -framework CoreFoundation -framework CoreVideo"
      )
    endif()
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARY)
