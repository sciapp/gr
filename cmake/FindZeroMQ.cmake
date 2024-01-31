#.rst:
# FindZeroMQ
# ----------
#
# Find the ZeroMQ messaging support library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``ZeroMQ::ZeroMQ``
#   The ZeroMQ library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``ZeroMQ_INCLUDE_DIR``
#   where to find zmq.h, etc.
# ``ZeroMQ_LIBRARIES``
#   the libraries to link against to use zmq.
# ``ZeroMQ_FOUND``
#   If false, do not try to use ZeroMQ.

if(NOT ZeroMQ_INCLUDE_DIR)
  find_path(ZeroMQ_INCLUDE_DIR zmq.h)
endif()

if(NOT ZeroMQ_LIBRARY)
  find_library(ZeroMQ_LIBRARY NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}zmq${GR_THIRDPARTY_LIBRARY_SUFFIX} zmq)
endif()

if(ZeroMQ_INCLUDE_DIR)
  if(NOT ZeroMQ_VERSION_STRING)
    file(READ ${ZeroMQ_INCLUDE_DIR}/zmq.h ZeroMQ_H_TEXT)
    string(REGEX REPLACE ".*#define ZMQ_VERSION_MAJOR[ \t]*([0-9]+).*" "\\1." ZMQ_MAJOR_STRING ${ZeroMQ_H_TEXT})
    string(REGEX REPLACE ".*#define ZMQ_VERSION_MINOR[ \t]*([0-9]+).*" "\\1." ZMQ_MINOR_STRING ${ZeroMQ_H_TEXT})
    string(REGEX REPLACE ".*#define ZMQ_VERSION_PATCH[ \t]*([0-9]+).*" "\\1" ZMQ_PATCH_STRING ${ZeroMQ_H_TEXT})
    string(CONCAT ZeroMQ_VERSION_STRING "${ZMQ_MAJOR_STRING}" "${ZMQ_MINOR_STRING}" "${ZMQ_PATCH_STRING}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  ZeroMQ
  VERSION_VAR
  ZeroMQ_VERSION_STRING
  REQUIRED_VARS
  ZeroMQ_LIBRARY
  ZeroMQ_INCLUDE_DIR
  ZeroMQ_VERSION_STRING
)

if(ZeroMQ_FOUND)
  set(ZeroMQ_INCLUDE_DIRS ${ZeroMQ_INCLUDE_DIR})
  set(ZeroMQ_LIBRARIES ${ZeroMQ_LIBRARY})

  if(NOT TARGET ZeroMQ::ZeroMQ)
    add_library(ZeroMQ::ZeroMQ STATIC IMPORTED)
    set_target_properties(
      ZeroMQ::ZeroMQ
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ZeroMQ_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                 IMPORTED_LOCATION "${ZeroMQ_LIBRARY}"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(ZeroMQ_INCLUDE_DIR ZeroMQ_LIBRARY)
