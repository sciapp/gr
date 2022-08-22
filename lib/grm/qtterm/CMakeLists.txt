cmake_minimum_required(VERSION 3.6.0)
project(qtterm)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCE_FILES
    main.cpp
    main_window.cpp
    receiver_thread.cpp
    tooltip.cpp
    grm_args_t_wrapper.cpp
)

if(DEFINED ENV{GRDIR})
  set(GRDIR $ENV{GRDIR})
  message(STATUS "Using GRDIR=" $ENV{GRDIR})
else(NOT DEFINED ENV{GRDIR})
  set(GRDIR "/usr/local/gr")
  message(STATUS "Using default GRDIR=/usr/local/gr")
endif(DEFINED ENV{GRDIR})

find_package(
  Qt6
  COMPONENTS Widgets
  QUIET
)
if(Qt6_FOUND AND ${CMAKE_VERSION} VERSION_GREATER "3.16.0")
  message("USING Qt6")
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(USING_QT6 True)
else()
  message("USING Qt5")
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

set_target_properties(
  Qt::Gui
  PROPERTIES MAP_IMPORTED_CONFIG_DEBUG Release MAP_IMPORTED_CONFIG_MINSIZEREL Release MAP_IMPORTED_CONFIG_RELWITHDEBINFO
                                                                                      Release
)
set_target_properties(
  Qt::Core
  PROPERTIES MAP_IMPORTED_CONFIG_DEBUG Release MAP_IMPORTED_CONFIG_MINSIZEREL Release MAP_IMPORTED_CONFIG_RELWITHDEBINFO
                                                                                      Release
)
set_target_properties(
  Qt::Widgets
  PROPERTIES MAP_IMPORTED_CONFIG_DEBUG Release MAP_IMPORTED_CONFIG_MINSIZEREL Release MAP_IMPORTED_CONFIG_RELWITHDEBINFO
                                                                                      Release
)

find_library(GR GR PATHS ${GRDIR}/lib/)
find_library(GRM GRM PATHS ${GRDIR}/lib/)

set(ICON_FILE resources/qtterm.icns)
set(QRC_FILE resources/resources.qrc)
set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION resources)

add_executable(qtterm WIN32 MACOSX_BUNDLE ${ICON_FILE} ${SOURCE_FILES})

if(DEFINED USING_QT6)
  message("LINKING Qt6")
  target_link_libraries(qtterm PRIVATE Qt6::Widgets ${GR} ${GRM})
else()
  message("LINKING Qt5")
  target_link_libraries(qtterm PRIVATE Qt5::Widgets ${GR} ${GRM})
endif()

set_target_properties(
  qtterm
  PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/resources/info.template.plist
             MACOSX_BUNDLE_GUI_IDENTIFIER "de.fz-juelich.qtterm"
             MACOSX_BUNDLE_ICON_FILE "qtterm.icns"
)

target_include_directories(qtterm PRIVATE ${GRDIR}/include)
