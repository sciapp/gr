#.rst:
# FindQt5
# -------
#
# Find the Qt5 framework. This find module is for use with a minimal
# distribution of the Qt5 runtime.
#
# Imported targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``Qt5::Core``
#   The Qt5 Core library, if found.
#
# ``Qt5::Network``
#   The Qt5 Network library, if found.
#
# ``Qt5::Gui``
#   The Qt5 Gui library, if found.
#
# ``Qt5::Widgets``
#   The Qt5 Widgets library, if found.
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``Qt5_FOUND``
#   If false, do not try to use Qt5.
#
# ``Qt5Core_FOUND``
#   If false, do not try to use Qt5 Core.
#
# ``Qt5Network_FOUND``
#   If false, do not try to use Qt5 Network.
#
# ``Qt5Gui_FOUND``
#   If false, do not try to use Qt5 Gui.
#
# ``Qt5Widgets_FOUND``
#   If false, do not try to use Qt5 Widgets.


if(NOT Qt5_INCLUDE_DIR)
    set(Qt5_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../include)
endif()

if(NOT Qt5Core_LIBRARY)
    set(Qt5Core_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/../Qt5Core.dll)
endif()

if(NOT Qt5Network_LIBRARY)
    set(Qt5Network_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/../Qt5Network.dll)
endif()

if(NOT Qt5Gui_LIBRARY)
    set(Qt5Gui_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/../Qt5Gui.dll)
endif()

if(NOT Qt5Widgets_LIBRARY)
    set(Qt5Widgets_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/../Qt5Widgets.dll)
endif()

if(NOT Qt5_VERSION_STRING)
    file(READ ${Qt5_INCLUDE_DIR}/QtCore/qconfig.h QCONFIG_H_TEXT)
    string(REGEX REPLACE ".*#define QT_VERSION_STR \"([0-9]+.[0-9]+.[0-9]+)\".*" "\\1" Qt5_VERSION_STRING ${QCONFIG_H_TEXT})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qt5
        VERSION_VAR Qt5_VERSION_STRING
        REQUIRED_VARS Qt5Core_LIBRARY Qt5Network_LIBRARY Qt5Gui_LIBRARY Qt5Widgets_LIBRARY Qt5_INCLUDE_DIR Qt5_VERSION_STRING)

set(Qt5Core_FOUND "${Qt5_FOUND}")
set(Qt5Network_FOUND "${Qt5_FOUND}")
set(Qt5Gui_FOUND "${Qt5_FOUND}")
set(Qt5Widgets_FOUND "${Qt5_FOUND}")

if (Qt5_FOUND)
    set(Qt5Core_INCLUDE_DIRS "${Qt5_INCLUDE_DIR}")
    set(Qt5Gui_INCLUDE_DIRS "${Qt5_INCLUDE_DIR}")
    set(Qt5Widgets_INCLUDE_DIRS "${Qt5_INCLUDE_DIR}")

    if(NOT TARGET Qt5::Core)
        add_library(Qt5::Core UNKNOWN IMPORTED)
        set_target_properties(Qt5::Core PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${Qt5_INCLUDE_DIR};${Qt5_INCLUDE_DIR}/QtCore"
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${Qt5Core_LIBRARY}")
    endif()
    if(NOT TARGET Qt5::Network)
        add_library(Qt5::Network UNKNOWN IMPORTED)
        set_target_properties(Qt5::Network PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${Qt5_INCLUDE_DIR};${Qt5_INCLUDE_DIR}/QtNetwork"
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${Qt5Network_LIBRARY}"
                INTERFACE_LINK_LIBRARIES "Qt5::Core")
    endif()
    if(NOT TARGET Qt5::Gui)
        add_library(Qt5::Gui UNKNOWN IMPORTED)
        set_target_properties(Qt5::Gui PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${Qt5_INCLUDE_DIR};${Qt5_INCLUDE_DIR}/QtGui"
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${Qt5Gui_LIBRARY}"
                INTERFACE_LINK_LIBRARIES "Qt5::Core")
    endif()
    if(NOT TARGET Qt5::Widgets)
        add_library(Qt5::Widgets UNKNOWN IMPORTED)
        set_target_properties(Qt5::Widgets PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${Qt5_INCLUDE_DIR};${Qt5_INCLUDE_DIR}/QtWidgets"
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${Qt5Widgets_LIBRARY}"
                INTERFACE_LINK_LIBRARIES "Qt5::Gui")
    endif()
elseif(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
    message(FATAL_ERROR "${CMAKE_FIND_PACKAGE_NAME} was required but could not be found.")
endif()

mark_as_advanced(Qt5_INCLUDE_DIR Qt5Core_LIBRARY Qt5Network_LIBRARY Qt5Gui_LIBRARY Qt5Widgets_LIBRARY QCONFIG_H_TEXT Qt5_VERSION_STRING)