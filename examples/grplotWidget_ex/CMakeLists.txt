cmake_minimum_required(VERSION 3.16...4.0 FATAL_ERROR)

project(gr_grplotWidget_example LANGUAGES CXX)

set(GRDIR
    "/usr/local/gr"
    CACHE STRING "Default value for GRDIR"
)
list(APPEND CMAKE_PREFIX_PATH "${GRDIR}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(
  Qt6
  COMPONENTS Core Widgets
  REQUIRED
)
find_package(GR NO_MODULE REQUIRED)

add_executable(main main.cpp main_window.cpp)
target_link_libraries(main PRIVATE GR::grplotWidget)
