cmake_minimum_required(VERSION 3.1...3.16)

project(
  grm_test_snoop
  DESCRIPTION "Test GRM against the GR.jl snoop example"
  LANGUAGES C
)

add_library(c_rand SHARED c_rand.c)
target_link_libraries(c_rand PRIVATE m)
target_compile_options(c_rand PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
set_target_properties(c_rand PROPERTIES C_STANDARD 90 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)

add_executable(snoop snoop.c)
target_link_libraries(snoop PRIVATE grm_shared)
target_link_libraries(snoop PRIVATE gr_shared)
target_link_libraries(snoop PRIVATE gks_shared)
target_link_libraries(snoop PRIVATE c_rand)
target_link_libraries(snoop PRIVATE m)
target_compile_options(snoop PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
set_target_properties(snoop PROPERTIES C_STANDARD 90 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
if(APPLE)
  set_target_properties(snoop PROPERTIES BUILD_RPATH "@loader_path/.")
else()
  set_target_properties(snoop PROPERTIES BUILD_RPATH "$ORIGIN/.")
endif()
