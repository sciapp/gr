#.rst:
# FindZeromq
# ----------
#
# Find the Zeromq messaging support library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Zeromq::Zeromq``
#   The Zeromq library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``ZEROMQ_INCLUDE_DIRS``
#   where to find zmq.h, etc.
# ``ZEROMQ_LIBRARIES``
#   the libraries to link against to use zmq.
# ``Zeromq_FOUND``
#   If false, do not try to use Zeromq.

if(NOT ZEROMQ_INCLUDE_DIR)
  find_path(ZEROMQ_INCLUDE_DIR zmq.h)
endif()

if(NOT ZEROMQ_LIBRARY)
  find_library(ZEROMQ_LIBRARY NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}zmq${GR_THIRDPARTY_LIBRARY_SUFFIX} zmq)
endif()

if(ZEROMQ_INCLUDE_DIR)
  if(NOT ZEROMQ_VERSION_STRING)
    file(READ ${ZEROMQ_INCLUDE_DIR}/zmq.h ZEROMQ_H_TEXT)
    string(REGEX REPLACE ".*#define ZMQ_VERSION_MAJOR[ \t]*([0-9]+).*" "\\1." ZMQ_MAJOR_STRING ${ZEROMQ_H_TEXT})
    string(REGEX REPLACE ".*#define ZMQ_VERSION_MINOR[ \t]*([0-9]+).*" "\\1." ZMQ_MINOR_STRING ${ZEROMQ_H_TEXT})
    string(REGEX REPLACE ".*#define ZMQ_VERSION_PATCH[ \t]*([0-9]+).*" "\\1" ZMQ_PATCH_STRING ${ZEROMQ_H_TEXT})
    string(CONCAT ZEROMQ_VERSION_STRING "${ZMQ_MAJOR_STRING}" "${ZMQ_MINOR_STRING}" "${ZMQ_PATCH_STRING}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Zeromq
  VERSION_VAR
  ZEROMQ_VERSION_STRING
  REQUIRED_VARS
  ZEROMQ_LIBRARY
  ZEROMQ_INCLUDE_DIR
  ZEROMQ_VERSION_STRING
)

if(Zeromq_FOUND)
  set(ZEROMQ_INCLUDE_DIRS ${ZEROMQ_INCLUDE_DIR})
  set(ZEROMQ_LIBRARIES ${ZEROMQ_LIBRARY})

  if(NOT TARGET Zeromq::Zeromq)
    add_library(Zeromq::Zeromq STATIC IMPORTED)
    set_target_properties(
      Zeromq::Zeromq
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ZEROMQ_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                 IMPORTED_LOCATION "${ZEROMQ_LIBRARY}"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(ZEROMQ_INCLUDE_DIR ZEROMQ_LIBRARY)
