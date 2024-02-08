cmake_minimum_required(VERSION 3.1...3.16)

project(
  grm_test_internal_api
  DESCRIPTION "Test the internal api of GRM"
  LANGUAGES C CXX
)

set(EXECUTABLE_SOURCES args_automatic_array_conversion.c get_compatible_format.c datatype/string_array_map.c
                       escape_minus.cxx
)

foreach(executable_source ${EXECUTABLE_SOURCES})
  get_filename_component(executable "${executable_source}" NAME_WE)
  add_executable("${PROJECT_NAME}_${executable}" "${executable_source}")
  target_include_directories("${PROJECT_NAME}_${executable}" PRIVATE ".")
  target_link_libraries("${PROJECT_NAME}_${executable}" PRIVATE grm_shared_internal)
  target_link_libraries("${PROJECT_NAME}_${executable}" PRIVATE m)
  target_compile_definitions("${PROJECT_NAME}_${executable}" PRIVATE BUILDING_GR)
  target_compile_options("${PROJECT_NAME}_${executable}" PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    "${PROJECT_NAME}_${executable}"
    PROPERTIES C_STANDARD 90
               C_STANDARD_REQUIRED ON
               C_EXTENSIONS OFF
               CXX_STANDARD 17
               CXX_STANDARD_REQUIRED ON
               CXX_EXTENSIONS OFF
  )
endforeach()
