cmake_minimum_required(VERSION 3.1...3.16)

project(
  grm_test_public_api
  DESCRIPTION "Test the public api of GRM"
  LANGUAGES C CXX
)

set(EXECUTABLE_SOURCES
    bar_errorbar.c
    barplot.cxx
    custom_receiver.c
    custom_sender.c
    dom_render.cxx
    event_handling.c
    heatmap.c
    histogram.c
    hold_append.c
    imshow.c
    isosurface.c
    layout.c
    line_errorbar.c
    merge_args.c
    multi_plot.c
    zoom.c
    pie.c
    plot.c
    polar_histogram.c
    polar_heatmap.cxx
    quiver.c
    raw.c
    receiver.c
    scatter_errorbar.c
    sender.c
    size.c
    subplots.c
    surface.c
    volume.c
    marginalheatmap.c
)

foreach(executable_source ${EXECUTABLE_SOURCES})
  get_filename_component(executable "${executable_source}" NAME_WE)
  get_filename_component(EXT "${executable_source}" LAST_EXT)
  add_executable("${PROJECT_NAME}_${executable}" "${executable_source}")
  target_link_libraries("${PROJECT_NAME}_${executable}" PRIVATE grm_shared)
  target_link_libraries("${PROJECT_NAME}_${executable}" PRIVATE gr_shared)
  target_link_libraries("${PROJECT_NAME}_${executable}" PRIVATE gks_shared)
  target_link_libraries("${PROJECT_NAME}_${executable}" PRIVATE m)
  if("${ext}" STREQUAL ".cxx")
    target_link_libraries("${PROJECT_NAME}_${executable}" PRIVATE GRender)
    target_link_libraries("${PROJECT_NAME}_${executable}" PRIVATE gr_graphics_tree_interface)
  endif()
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

add_subdirectory(snoop grm_test_snoop)
