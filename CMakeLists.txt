cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(GetVersionFromGit)
get_version_from_git(GR_VERSION GR_VERSION_FULL)

project(GR VERSION ${GR_VERSION} LANGUAGES C CXX )

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(GR_BUILD_DEMOS "Build demos for GR" OFF)
option(GR_INSTALL "Create installation target for GR" ON)
option(GR_MANUAL_MOC_AND_RCC "Manually run moc and rcc instead of relying on AUTOMOC and AUTORCC" OFF)
set(GR_DIRECTORY "/usr/local/gr" CACHE STRING "Default value for GRDIR")
option(GR_USE_BUNDLED_LIBRARIES "Use thirdparty libraries bundled with GR" ON)
if (GR_USE_BUNDLED_LIBRARIES)
    list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/3rdparty/build/")
    set(GR_THIRDPARTY_LIBRARY_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
    set(GR_THIRDPARTY_LIBRARY_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
    set(GR_THIRDPARTY_LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
    set(GR_THIRDPARTY_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

if(WIN32)
    set(GR_PLUGIN_SUFFIX ".dll")
else()
    set(GR_PLUGIN_SUFFIX ".so")
endif()

find_package(Threads REQUIRED)
find_package(Freetype)
find_package(Jpeg REQUIRED)
find_package(Libpng REQUIRED)
find_package(Zlib REQUIRED)
find_package(Qhull REQUIRED)
find_package(Tiff)
find_package(Ffmpeg)
find_package(Glfw)
find_package(Zeromq)
find_package(Cairo)
find_package(X11)
find_package(Fontconfig)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)
find_package(Gs)
find_package(Qt5 OPTIONAL_COMPONENTS Widgets Core Network)
find_package(Qt4)


if (APPLE)
    set(INSTALL_RPATH "${GR_DIRECTORY}/lib/;@loader_path/.")
else()
    set(INSTALL_RPATH "${GR_DIRECTORY}/lib/;$ORIGIN/.")
endif()


if(Qt4_FOUND OR (Qt5Widgets_FOUND AND Qt5Core_FOUND AND Qt5Network_FOUND))
    if (GR_MANUAL_MOC_AND_RCC)
        if(NOT QT_MOC_EXECUTABLE)
            find_program(QT_MOC_EXECUTABLE moc)
        endif()
        if(NOT QT_MOC_EXECUTABLE)
            message(FATAL_ERROR "Could not find moc but GR_MANUAL_MOC_AND_RCC is set.")
        endif()
        if(NOT QT_RCC_EXECUTABLE)
            find_program(QT_RCC_EXECUTABLE rcc)
        endif()
        if(NOT QT_RCC_EXECUTABLE)
            message(FATAL_ERROR "Could not find rcc but GR_MANUAL_MOC_AND_RCC is set.")
        endif()
    endif()
endif()
if(Qt4_FOUND)
    set(QT4_MOC_INCLUDE_FLAGS "")
    foreach(DIR IN LISTS QT_INCLUDE_DIR)
        set(QT4_MOC_INCLUDE_FLAGS ${QT4_MOC_INCLUDE_FLAGS} -I${DIR})
    endforeach()
endif()
if(Qt5Widgets_FOUND AND Qt5Core_FOUND AND Qt5Network_FOUND)
    set(QT5_MOC_INCLUDE_FLAGS "")
    foreach(DIR IN LISTS Qt5Core_INCLUDE_DIRS Qt5Gui_INCLUDE_DIRS Qt5Widgets_INCLUDE_DIRS)
        set(QT5_MOC_INCLUDE_FLAGS ${QT5_MOC_INCLUDE_FLAGS} -I${DIR})
    endforeach()
endif()

add_library(gks_static STATIC
        lib/gks/afm.c
        lib/gks/font.c
        lib/gks/mac.c
        lib/gks/socket.c
        lib/gks/cgm.c
        lib/gks/ft.c
        lib/gks/malloc.c
        lib/gks/util.c
        lib/gks/compress.c
        lib/gks/gks.c
        lib/gks/mf.c
        lib/gks/win.c
        lib/gks/gksforbnd.c
        lib/gks/pdf.c
        lib/gks/wiss.c
        lib/gks/dl.c
        lib/gks/plugin.c
        lib/gks/error.c
        lib/gks/io.c
        lib/gks/ps.c
        )
add_library(gks_shared SHARED
        lib/gks/afm.c
        lib/gks/font.c
        lib/gks/mac.c
        lib/gks/socket.c
        lib/gks/cgm.c
        lib/gks/ft.c
        lib/gks/malloc.c
        lib/gks/util.c
        lib/gks/compress.c
        lib/gks/gks.c
        lib/gks/mf.c
        lib/gks/win.c
        lib/gks/gksforbnd.c
        lib/gks/pdf.c
        lib/gks/wiss.c
        lib/gks/dl.c
        lib/gks/plugin.c
        lib/gks/error.c
        lib/gks/io.c
        lib/gks/ps.c
        )
foreach(LIBRARY gks_static gks_shared)
    target_link_libraries(${LIBRARY} PUBLIC Threads::Threads)
    if(FREETYPE_FOUND)
        target_link_libraries(${LIBRARY} PUBLIC Freetype::Freetype)
    else()
        target_compile_definitions(${LIBRARY} PUBLIC NO_FT)
    endif()
    target_link_libraries(${LIBRARY} PUBLIC Zlib::Zlib)
    if(UNIX)
        target_link_libraries(${LIBRARY} PUBLIC dl)
    elseif(WIN32)
        target_link_libraries(${LIBRARY} PUBLIC wsock32)
        target_link_libraries(${LIBRARY} PUBLIC msimg32)
        target_link_libraries(${LIBRARY} PUBLIC gdi32)
    endif()
    target_compile_options(${LIBRARY} PUBLIC -pthread)
    target_include_directories(${LIBRARY} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            )
    target_compile_definitions(${LIBRARY} PUBLIC
            GRDIR="${GR_DIRECTORY}"
            )
    set_target_properties(${LIBRARY} PROPERTIES
            C_STANDARD 90
            C_EXTENSIONS OFF
            C_STANDARD_REQUIRED ON
            POSITION_INDEPENDENT_CODE ON
            OUTPUT_NAME gks
            INSTALL_RPATH "${INSTALL_RPATH}"
            )
endforeach()
add_library(GR::GKS ALIAS gks_static)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/gr_version.h "#ifndef GR_VERSION\n#define GR_VERSION \"${GR_VERSION_FULL}\"\n#endif\n")
set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/gr_version.h
        PROPERTIES GENERATED TRUE
)


add_library(gr
        lib/gr/boundary.c
        lib/gr/contour.c
        lib/gr/contourf.c
        lib/gr/delaunay.c
        lib/gr/gr.c
        lib/gr/grforbnd.c
        lib/gr/gridit.c
        lib/gr/image.c
        lib/gr/import.c
        lib/gr/interp2.c
        lib/gr/io.c
        lib/gr/md5.c
        lib/gr/meta.c
        lib/gr/shade.c
        lib/gr/spline.c
        lib/gr/strlib.c
        lib/gr/text.c
        ${CMAKE_CURRENT_BINARY_DIR}/gr_version.h
        )
target_link_libraries(gr PUBLIC GR::GKS)
target_link_libraries(gr PUBLIC m)
target_link_libraries(gr PUBLIC Qhull::Qhull)
target_link_libraries(gr PUBLIC Jpeg::Jpeg)
target_link_libraries(gr PUBLIC Libpng::Libpng)
if(WIN32)
    target_link_libraries(gr PUBLIC ws2_32)
endif()
target_include_directories(gr PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
target_include_directories(gr PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        )
set_target_properties(gr PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
        INSTALL_RPATH "${INSTALL_RPATH}"
        )
add_library(GR::GR ALIAS gr)


add_library(gr3 SHARED
        lib/gr3/gr3.c
        lib/gr3/gr3_convenience.c
        lib/gr3/gr3_gr.c
        lib/gr3/gr3_html.c
        lib/gr3/gr3_jpeg.c
        lib/gr3/gr3_mc.c
        lib/gr3/gr3_png.c
        lib/gr3/gr3_povray.c
        lib/gr3/gr3_slices.c
        )
if(APPLE)
    target_sources(gr3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr3/gr3_cgl.c)
elseif(UNIX AND NOT APPLE)
    target_sources(gr3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr3/gr3_glx.c)
elseif(WIN32)
    target_sources(gr3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr3/gr3_win.c)
endif()
target_link_libraries(gr3 PUBLIC GR::GR)
target_link_libraries(gr3 PUBLIC Jpeg::Jpeg)
target_link_libraries(gr3 PUBLIC Libpng::Libpng)
if(APPLE OR WIN32)
    target_link_libraries(gr3 PUBLIC OpenGL::GL)
endif()
if(APPLE)
    # Apple has deprecated OpenGL in macOS 10.14
    target_compile_definitions(gr3 PRIVATE GL_SILENCE_DEPRECATION)
endif()
target_include_directories(gr3 PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/gr3/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
set_target_properties(gr3 PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
        INSTALL_RPATH "${INSTALL_RPATH}"
        )
add_library(GR::GR3 ALIAS gr3)


if(UNIX AND NOT APPLE)
    if(X11_FOUND)
    add_library(gr3platform SHARED lib/gr3/gr3_platform_glx.c)
    target_link_libraries(gr3platform PUBLIC GR::GR)
    target_link_libraries(gr3platform PUBLIC OpenGL::GL)
    target_link_libraries(gr3platform PUBLIC X11::X11)
    set_target_properties(gr3platform PROPERTIES
            C_STANDARD 90
            C_EXTENSIONS OFF
            C_STANDARD_REQUIRED ON
            POSITION_INDEPENDENT_CODE ON
            INSTALL_RPATH "${INSTALL_RPATH}"
            )
    else()
        message(WARNING "libGR3platform.so requires X11 and will not be built.")
    endif()
endif()


add_library(cairoplugin SHARED
        lib/gks/plugin/cairoplugin.c
        )
target_link_libraries(cairoplugin PUBLIC GR::GKS)
if(Cairo_FOUND)
    target_link_libraries(cairoplugin PRIVATE Cairo::Cairo)
    target_link_libraries(cairoplugin PRIVATE Jpeg::Jpeg)
else()
    target_compile_definitions(cairoplugin PRIVATE NO_CAIRO)
endif()
if(Tiff_FOUND)
    target_link_libraries(cairoplugin PRIVATE Tiff::Tiff)
else()
    target_compile_definitions(cairoplugin PRIVATE NO_TIFF)
endif()
# Cairo X11 support is disabled to allow users to generate images using Cairo on systems without X11 installed
target_compile_definitions(cairoplugin PRIVATE NO_X11)
set_target_properties(cairoplugin PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(videoplugin SHARED
        lib/gks/plugin/videoplugin.c
        lib/gks/plugin/vc.c
        lib/gks/plugin/gif.c
        )
target_link_libraries(videoplugin PUBLIC GR::GKS)
if(NOT Cairo_FOUND)
    target_compile_definitions(videoplugin PRIVATE NO_CAIRO)
endif()
if(Ffmpeg_FOUND)
    target_link_libraries(videoplugin PUBLIC Ffmpeg::Ffmpeg)
else()
    target_compile_definitions(videoplugin PRIVATE NO_AV)
endif()
set_target_properties(videoplugin PROPERTIES
        C_STANDARD 99
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(x11plugin SHARED
        lib/gks/plugin/x11plugin.c
        )
target_link_libraries(x11plugin PUBLIC GR::GKS)
if(NOT Freetype_FOUND)
    target_compile_definitions(x11plugin PRIVATE NO_FT)
endif()
if (X11_FOUND)
    if (NOT TARGET X11::X11)
        add_library(X11::X11 UNKNOWN IMPORTED)
        if(NOT Xft_FOUND)
            target_compile_definitions(x11plugin PRIVATE NO_XFT)
        else()
            target_link_libraries(x11plugin PUBLIC Fontconfig::Fontconfig)
        endif()
        set_target_properties(X11::X11 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${X11_INCLUDE_DIR}/"
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${X11_X11_LIB}"
            INTERFACE_LINK_LIBRARIES "${X11_Xft_LIB};${X11_Xt_LIB}")
    endif()
    target_link_libraries(x11plugin PUBLIC X11::X11)
else()
    target_compile_definitions(x11plugin PRIVATE NO_X11)
    target_compile_definitions(x11plugin PRIVATE NO_XFT)
endif()
set_target_properties(x11plugin PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(pgfplugin SHARED
        lib/gks/plugin/pgfplugin.c
        )
target_link_libraries(pgfplugin PUBLIC GR::GKS)
target_link_libraries(pgfplugin PUBLIC Libpng::Libpng)
set_target_properties(pgfplugin PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(htmplugin SHARED
        lib/gks/plugin/htmplugin.c
        )
target_link_libraries(htmplugin PUBLIC GR::GKS)
target_link_libraries(htmplugin PUBLIC Libpng::Libpng)
set_target_properties(htmplugin PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(glplugin SHARED
        lib/gks/plugin/glplugin.c
        )
target_link_libraries(glplugin PUBLIC GR::GKS)
target_link_libraries(glplugin PUBLIC OpenGL::GL)
if(Glfw_FOUND)
    target_link_libraries(glplugin PUBLIC Glfw::Glfw)
else()
    target_compile_definitions(glplugin PRIVATE NO_GLFW)
endif()
if(NOT Freetype_FOUND)
    target_compile_definitions(glplugin PRIVATE NO_FT)
endif()
if(APPLE)
    # Apple has deprecated OpenGL in macOS 10.14
    target_compile_definitions(glplugin PRIVATE GL_SILENCE_DEPRECATION)
endif()
set_target_properties(glplugin PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(zmqplugin SHARED
        lib/gks/plugin/zmqplugin.c
        )
target_link_libraries(zmqplugin PUBLIC GR::GKS)
if(Zeromq_FOUND)
    target_link_libraries(zmqplugin PUBLIC Zeromq::Zeromq)
else()
    target_compile_definitions(zmqplugin PRIVATE NO_ZMQ)
endif()
set_target_properties(zmqplugin PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(wmfplugin SHARED
        lib/gks/plugin/wmfplugin.c
        )
target_link_libraries(wmfplugin PUBLIC GR::GKS)
set_target_properties(wmfplugin PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(gsplugin SHARED
        lib/gks/plugin/gsplugin.c
        )
target_link_libraries(gsplugin PUBLIC GR::GKS)
if(X11_FOUND AND Gs_FOUND)
    target_link_libraries(gsplugin PUBLIC X11::X11)
endif()
if( NOT Gs_FOUND OR  NOT X11_FOUND)
   target_compile_definitions(gsplugin PRIVATE NO_GS)
else()
    target_link_libraries(gsplugin PUBLIC Gs::Gs)
endif()
set_target_properties(gsplugin PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(figplugin SHARED
        lib/gks/plugin/figplugin.c
        )
target_link_libraries(figplugin PUBLIC GR::GKS)
target_link_libraries(figplugin PUBLIC Libpng::Libpng)
set_target_properties(figplugin PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(gtkplugin SHARED
        lib/gks/plugin/gtkplugin.c
        )
target_link_libraries(gtkplugin PUBLIC GR::GKS)
set_target_properties(gtkplugin PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(svgplugin SHARED
        lib/gks/plugin/svgplugin.c
        )
target_link_libraries(svgplugin PUBLIC GR::GKS)
target_link_libraries(svgplugin PUBLIC Libpng::Libpng)
set_target_properties(svgplugin PROPERTIES
        C_STANDARD 90
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )


add_library(qt5plugin SHARED
        lib/gks/plugin/qt5plugin.cxx
        )
target_link_libraries(qt5plugin PUBLIC GR::GKS)
if(Qt5Widgets_FOUND)
    target_link_libraries(qt5plugin PUBLIC Qt5::Widgets)
else()
    target_compile_definitions(qt5plugin PRIVATE NO_QT5)
endif()
set_target_properties(qt5plugin PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        PREFIX ""
        SUFFIX "${GR_PLUGIN_SUFFIX}"
        )
add_library(qtplugin SHARED
        lib/gks/plugin/qtplugin.cxx
        )
target_link_libraries(qtplugin PUBLIC GR::GKS)


if(Qt4_FOUND)
    target_link_libraries(qtplugin PUBLIC Qt4::QtGui)
else()
    target_compile_definitions(qtplugin PRIVATE NO_QT)
endif()
set_target_properties(qtplugin PROPERTIES
    CXX_STANDARD 11
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
    PREFIX ""
    SUFFIX "${GR_PLUGIN_SUFFIX}"
    )

if(Qt4_FOUND)
    add_library(qt4gr SHARED lib/gr/qtgr/grwidget.cxx)
    target_link_libraries(qt4gr PUBLIC GR::GR)
    target_link_libraries(qt4gr PUBLIC Qt4::QtCore Qt4::QtGui Qt4::QtNetwork)
    if(MINGW)
        target_compile_options(qt4gr PRIVATE -fno-exceptions)
    endif()

    if (GR_MANUAL_MOC_AND_RCC)
        set_target_properties(qt4gr PROPERTIES
                AUTOMOC OFF
                AUTORCC OFF
                )
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_qt4_grwidget.cxx
                COMMAND ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GRDIR)\" ${QT4_MOC_INCLUDE_FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.h -o ${CMAKE_CURRENT_BINARY_DIR}/moc_qt4_grwidget.cxx
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.cxx)
        target_sources(qt5gr PUBLIC
                ${CMAKE_CURRENT_BINARY_DIR}/moc_qt4_grwidget.cxx)
    else()
        set_target_properties(qt4gr PROPERTIES
                AUTOMOC ON
                AUTORCC ON
                )
        target_sources(qt4gr PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.h)
    endif()
endif()

if(Qt5Widgets_FOUND AND Qt5Core_FOUND AND Qt5Network_FOUND)
    add_library(qt5gr SHARED lib/gr/qtgr/grwidget.cxx)
    target_link_libraries(qt5gr PUBLIC GR::GR)
    target_link_libraries(qt5gr PUBLIC Qt5::Widgets Qt5::Core Qt5::Network)
    if(MINGW)
        target_compile_options(qt5gr PRIVATE -fno-exceptions)
    endif()

    if (GR_MANUAL_MOC_AND_RCC)
        set_target_properties(qt5gr PROPERTIES
                AUTOMOC OFF
                AUTORCC OFF
                )
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_qt5_grwidget.cxx
                COMMAND ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GRDIR)\" ${QT5_MOC_INCLUDE_FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.h -o ${CMAKE_CURRENT_BINARY_DIR}/moc_qt5_grwidget.cxx
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.cxx)
        target_sources(qt5gr PUBLIC
                ${CMAKE_CURRENT_BINARY_DIR}/moc_qt5_grwidget.cxx)
    else()
        set_target_properties(qt5gr PROPERTIES
                AUTOMOC ON
                AUTORCC ON
                )
        target_sources(qt5gr PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.h)
    endif()
endif()

if(APPLE)
add_executable(GKSTerm MACOSX_BUNDLE
        lib/gks/quartz/GKSTerm.m
        lib/gks/quartz/main.m
        lib/gks/quartz/GKSView.m
        )
target_link_libraries(GKSTerm PUBLIC Zeromq::Zeromq)
target_link_libraries(GKSTerm PUBLIC GR::GKS
        "-framework CoreGraphics -framework CoreFoundation -framework CoreVideo -framework Cocoa")
file(READ "lib/gks/quartz/Info.plist" GKS_TERM_INFO_PLIST)
string(REPLACE "\${EXECUTABLE_NAME}" "GKSTerm" GKS_TERM_INFO_PLIST ${GKS_TERM_INFO_PLIST})
string(REPLACE "\${PRODUCT_NAME}" "GKSTerm" GKS_TERM_INFO_PLIST ${GKS_TERM_INFO_PLIST})
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" ${GKS_TERM_INFO_PLIST})
add_custom_command(TARGET GKSTerm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/Info.plist ${CMAKE_CURRENT_BINARY_DIR}/GKSTerm.app/Contents/Info.plist
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
        BYPRODUCTS GKSTerm.app/Contents/Info.plist
        )
add_custom_command(TARGET GKSTerm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/quartz/English.lproj ${CMAKE_CURRENT_BINARY_DIR}/GKSTerm.app/Contents/Resources/English.lproj
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/quartz/English.lproj
        BYPRODUCTS GKSTerm.app/Contents/Resources/English.lproj
        )
add_custom_command(TARGET GKSTerm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/quartz/GKSTerm.icns ${CMAKE_CURRENT_BINARY_DIR}/GKSTerm.app/Contents/Resources/GKSTerm.icns
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/quartz/GKSTerm.icns
        BYPRODUCTS GKSTerm.app/Contents/Resources/GKSTerm.icns
        )
endif()


if(Qt4_FOUND OR (Qt5Widgets_FOUND AND Qt5Core_FOUND AND Qt5Network_FOUND))
    add_executable(gksqt WIN32 MACOSX_BUNDLE
            lib/gks/qt/gksqt.cxx
            lib/gks/qt/gkswidget.cxx
            lib/gks/qt/gksserver.cxx)
    target_link_libraries(gksqt PUBLIC GR::GKS)
    if(Qt5Widgets_FOUND AND Qt5Core_FOUND AND Qt5Network_FOUND)
        target_link_libraries(gksqt PUBLIC Qt5::Widgets Qt5::Core Qt5::Network)
    elseif(Qt4_FOUND)
        target_link_libraries(gksqt PUBLIC Qt4::QtCore Qt4::QtGui Qt4::QtNetwork)
    endif()
    if(MINGW)
        target_compile_options(gksqt PRIVATE -fno-exceptions)
    endif()
    if (GR_MANUAL_MOC_AND_RCC)
        set_target_properties(gksqt PROPERTIES
                AUTOMOC OFF
                AUTORCC OFF
                )
        if(Qt5Widgets_FOUND AND Qt5Core_FOUND AND Qt5Network_FOUND)
            set(MOC_INCLUDE_FLAGS ${QT5_MOC_INCLUDE_FLAGS})
        else()
            set(MOC_INCLUDE_FLAGS ${QT4_MOC_INCLUDE_FLAGS})
        endif()
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_gkswidget.cxx
                COMMAND ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GRDIR)\" ${MOC_INCLUDE_FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gkswidget.h -o ${CMAKE_CURRENT_BINARY_DIR}/moc_gkswidget.cxx
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gkswidget.h)
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_gksserver.cxx
                COMMAND ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GRDIR)\" ${MOC_INCLUDE_FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksserver.h -o ${CMAKE_CURRENT_BINARY_DIR}/moc_gksserver.cxx
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksserver.h)
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qrc_gksqt.cxx
                COMMAND ${QT_RCC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksqt.qrc -o ${CMAKE_CURRENT_BINARY_DIR}/qrc_gksqt.cxx
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksqt.qrc)
        target_sources(gksqt PUBLIC
                ${CMAKE_CURRENT_BINARY_DIR}/moc_gksserver.cxx
                ${CMAKE_CURRENT_BINARY_DIR}/moc_gkswidget.cxx
                ${CMAKE_CURRENT_BINARY_DIR}/qrc_gksqt.cxx)
    else()
        set_target_properties(gksqt PROPERTIES
                AUTOMOC ON
                AUTORCC ON
                )
        target_sources(gksqt PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksserver.h
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gkswidget.h
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksqt.qrc)
    endif()
    if(APPLE)
        add_custom_command(TARGET gksqt POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/Info.plist ${CMAKE_CURRENT_BINARY_DIR}/gksqt.app/Contents/Info.plist
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/Info.plist
                BYPRODUCTS gksqt.app/Contents/Info.plist
                )
        add_custom_command(TARGET gksqt POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksqt.icns ${CMAKE_CURRENT_BINARY_DIR}/gksqt.app/Contents/Resources/gksqt.icns
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksqt.icns
                BYPRODUCTS gksqt.app/Contents/Resources/gksqt.icns
                )
    endif()
endif()


if(GR_INSTALL)
    include(GNUInstallDirs)
    install(TARGETS gr gks_static gks_shared gr3 cairoplugin figplugin glplugin gsplugin gtkplugin htmplugin pgfplugin svgplugin videoplugin wmfplugin x11plugin zmqplugin qtplugin qt5plugin
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            )
    if(TARGET gr3platform)
        install(TARGETS gr3platform
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                )
    endif()
    if(TARGET qt4gr)
        install(TARGETS qt4gr
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                )
    endif()
    if(TARGET qt5gr)
        install(TARGETS qt5gr
                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                )
    endif()
    if(TARGET GKSTerm)
        install(TARGETS GKSTerm
                BUNDLE DESTINATION Applications
                )
    endif()
    if(TARGET gksqt)
        install(TARGETS gksqt
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                BUNDLE DESTINATION Applications
                )
    endif()
    install(FILES lib/gr/gr.h lib/gks/gks.h lib/gr3/gr3.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/
            )
    install(DIRECTORY lib/gks/fonts
            DESTINATION ${CMAKE_INSTALL_PREFIX}/
            USE_SOURCE_PERMISSIONS
            )
endif()


if (GR_BUILD_DEMOS)
    add_executable(gksdemo
            lib/gks/demo.c
            )
    target_link_libraries(gksdemo PUBLIC GR::GKS)
    set_target_properties(gksdemo PROPERTIES
            C_STANDARD 90
            C_EXTENSIONS OFF
            C_STANDARD_REQUIRED ON
            CXX_STANDARD 11
            CXX_EXTENSIONS OFF
            CXX_STANDARD_REQUIRED ON
            )
    add_executable(grdemo
            lib/gr/demo.c
            )
    target_link_libraries(grdemo PUBLIC GR::GR)
    set_target_properties(grdemo PROPERTIES
            C_STANDARD 90
            C_EXTENSIONS OFF
            C_STANDARD_REQUIRED ON
            CXX_STANDARD 11
            CXX_EXTENSIONS OFF
            CXX_STANDARD_REQUIRED ON
            )
endif ()