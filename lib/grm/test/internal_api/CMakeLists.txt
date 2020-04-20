cmake_minimum_required(VERSION 3.1...3.16)

# Import an already installed library
function(import_library name_prefix library_name header_filename)
  cmake_parse_arguments(
    PARSE_ARGV
    3
    import
    ""
    ""
    "PATHS"
  )
  set(lib_includes "")
  foreach(include ${import_PATHS})
    list(APPEND lib_includes "${include}")
    list(APPEND lib_includes "${include}/include")
  endforeach()
  set(lib_paths "")
  foreach(path ${import_PATHS})
    list(APPEND lib_paths "${path}")
    list(APPEND lib_paths "${path}/lib")
  endforeach()

  # Pass unique variable names as first parameter to `path_path` and `find_library` to
  # enable CMake caching
  find_path(${name_prefix}_include "${header_filename}" PATHS ${lib_includes})
  find_library(${name_prefix}_lib "${library_name}" PATHS ${lib_paths})
  if("${${name_prefix}_include}" STREQUAL "${name_prefix}_include-NOTFOUND" OR "${${name_prefix}_lib}" STREQUAL
                                                                               "${name_prefix}_lib-NOTFOUND"
  )
    message(FATAL_ERROR "Could not find the library ${library_name}.")
  endif()
  add_library(${name_prefix} SHARED IMPORTED GLOBAL)
  set_target_properties(${name_prefix} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${${name_prefix}_include}")
  set_target_properties(${name_prefix} PROPERTIES IMPORTED_LOCATION "${${name_prefix}_lib}")
endfunction()

# Set the default build type to `Debug`; important: this must be set before a call to `project`!
set(CMAKE_BUILD_TYPE
    Debug
    CACHE STRING ""
)
set(GRDIR
    "/usr/local/gr"
    CACHE STRING "Root directory of the GR installation"
)

project(
  grm_test_internal_api
  DESCRIPTION "Test the internal api of GRM"
  LANGUAGES C
)

set(EXECUTABLE_SOURCES)

set(GRM_SOURCES
    ../../args.c
    ../../dump.c
    ../../dynamic_args_array.c
    ../../error.c
    ../../event.c
    ../../interaction.c
    ../../json.c
    ../../memwriter.c
    ../../net.c
    ../../plot.c
    ../../util.c
    ../../datatype/string_array_map.c
    ../../datatype/string_list.c
    ../../datatype/string_map.c
    ../../datatype/uint_map.c
)

import_library(
  gks
  GKS
  gks.h
  PATHS
  "${GRDIR}"
)
import_library(
  gr
  GR
  gr.h
  PATHS
  "${GRDIR}"
)
import_library(
  gr3
  GR3
  gr3.h
  PATHS
  "${GRDIR}"
)

add_library(grm SHARED "${GRM_SOURCES}")
target_include_directories(grm PUBLIC ../../ ../../../gks)
target_link_libraries(grm PUBLIC gks)
target_link_libraries(grm PUBLIC gr)
target_link_libraries(grm PUBLIC gr3)
set_target_properties(grm PROPERTIES C_STANDARD 90 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)

foreach(executable_source ${EXECUTABLE_SOURCES})
  get_filename_component(executable "${executable_source}" NAME_WE)
  add_executable("${executable}" "${executable_source}")
  target_link_libraries("${executable}" PRIVATE grm)
  target_link_libraries("${executable}" PRIVATE m)
  set_target_properties("${executable}" PROPERTIES C_STANDARD 90 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
endforeach()
