cmake_minimum_required(VERSION 3.23)
project(GREditor)
set(CMAKE_PREFIX_PATH /usr/local/qt6/lib/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(DEFINED ENV{GRDIR})
  set(GRDIR $ENV{GRDIR})
  message(STATUS "Using GRDIR=" $ENV{GRDIR})
else(NOT DEFINED ENV{GRDIR})
  set(GRDIR "/usr/local/gr")
  message(STATUS "Using default GRDIR=/usr/local/gr")
endif(DEFINED ENV{GRDIR})

find_library(GR GR PATHS ${GRDIR}/lib/)
find_library(GRM GRM PATHS ${GRDIR}/lib/)

find_package(
  Qt6
  COMPONENTS Core Gui Widgets PrintSupport
  REQUIRED
)

add_executable(
  GREditor
  Bounding_logic.cpp
  Bounding_object.cpp
  CustomTreeWidgetItem.cpp
  main.cpp
  main_window.cxx
  TreeWidget.cpp
)

target_link_libraries(
  GREditor
  Qt::Core
  Qt::Gui
  Qt::Widgets
  Qt6::PrintSupport
  ${GR}
  ${GRM}
  -fsanitize=address
)

target_include_directories(GREditor PRIVATE ${GRDIR}/include)
