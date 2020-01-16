#.rst:
# FindQhull
# ---------
#
# Find the Qhull library.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``Qhull::Qhull``
#   The Qhull library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``QHULL_INCLUDE_DIRS``
#   where to find qhull_a.h, etc.
# ``QHULL_LIBRARIES``
#   the libraries to link against to use Qhull.
# ``Qhull_FOUND``
#   If false, do not try to use Qhull.

if(NOT QHULL_INCLUDE_DIR)
  find_path(QHULL_INCLUDE_DIR qhull/qhull_a.h)
endif()

if(NOT QHULL_LIBRARY)
  find_library(QHULL_LIBRARY NAMES ${GR_THIRDPARTY_LIBRARY_PREFIX}qhull${GR_THIRDPARTY_LIBRARY_SUFFIX} qhull)
endif()

if(NOT QHULL_VERSION_STRING
   AND QHULL_INCLUDE_DIR
   AND QHULL_LIBRARY
)
  if(CMAKE_CROSSCOMPILING)
    # Qhull version cannot be queried when cross compiling
    set(QHULL_VERSION_STRING "Unknown")
  else()
    file(
      WRITE "${CMAKE_CURRENT_BINARY_DIR}/qhull_get_version.c"
      "
        #include \"qhull/libqhull.h\"

        int main(void)
        {
            printf(\"%s\", qh_version2);
            return 0;
        }
        "
    )

    try_compile(
      COMPILE_SUCCESS "${CMAKE_CURRENT_BINARY_DIR}/qhull_get_version" "${CMAKE_CURRENT_BINARY_DIR}/qhull_get_version.c"
      LINK_LIBRARIES ${QHULL_LIBRARY} m
      COPY_FILE "${CMAKE_CURRENT_BINARY_DIR}/qhull_get_version/exe"
      CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${QHULL_INCLUDE_DIR}"
    )
    if(COMPILE_SUCCESS)
      execute_process(COMMAND "${CMAKE_CURRENT_BINARY_DIR}/qhull_get_version/exe" OUTPUT_VARIABLE QHULL_H_TEXT)
      string(REGEX REPLACE "[ \tA-Za-z]*([0-9]+.[0-9]+.[0-9]+).*" "\\1" QHULL_VERSION_STRING ${QHULL_H_TEXT})
    else()
      set(QHULL_VERSION_STRING "Unknown")
    endif()
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Qhull
  VERSION_VAR
  QHULL_VERSION_STRING
  REQUIRED_VARS
  QHULL_LIBRARY
  QHULL_INCLUDE_DIR
  QHULL_VERSION_STRING
)

if(Qhull_FOUND)
  set(QHULL_INCLUDE_DIRS ${QHULL_INCLUDE_DIR})
  set(QHULL_LIBRARIES ${QHULL_LIBRARY})

  if(NOT TARGET Qhull::Qhull)
    add_library(Qhull::Qhull UNKNOWN IMPORTED)
    set_target_properties(
      Qhull::Qhull
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${QHULL_INCLUDE_DIRS}"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${QHULL_LIBRARY}"
    )
  endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
  message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(QHULL_INCLUDE_DIR QHULL_LIBRARY)
