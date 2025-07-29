cmake_minimum_required(VERSION 3.18...4.0 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GetVersionFromGit)
get_version_from_git(GR_VERSION GR_VERSION_FULL)

project(
  GR
  VERSION ${GR_VERSION}
  LANGUAGES C CXX
)

include(GNUInstallDirs)
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-Werror=implicit" ERROR_IMPLICIT_SUPPORTED)
if(ERROR_IMPLICIT_SUPPORTED)
  set(COMPILER_OPTION_ERROR_IMPLICIT
      "-Werror=implicit"
      CACHE INTERNAL "Compiler flag for generating errors on implicit declarations"
  )
else()
  set(COMPILER_OPTION_ERROR_IMPLICIT
      ""
      CACHE INTERNAL "Compiler flag for generating errors on implicit declarations"
  )
endif()

if(UNIX)
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX
        "/usr/local/gr"
        CACHE PATH "GR install prefix" FORCE
    )
  endif()
endif()
# Force to install libraries to `lib` (avoid `lib64` on RHEL distributions)
set(CMAKE_INSTALL_LIBDIR "lib")
set(GR_DIRECTORY
    "${CMAKE_INSTALL_PREFIX}"
    CACHE STRING "Default value for GRDIR"
)
option(GR_BUILD_DEMOS "Build demos for GR" OFF)
option(GR_BUILD_GKSM "Build GKS metafile reader for GR" OFF)
option(GR_INSTALL "Create installation target for GR" ON)
option(GR_USE_BUNDLED_LIBRARIES "Use thirdparty libraries bundled with GR" OFF)
option(GR_MANUAL_MOC_AND_RCC "Manually run moc and rcc instead of relying on AUTOMOC and AUTORCC" OFF)
option(GR_FIND_QT5_BY_VARIABLES "Find Qt5 based on the variables Qt5_INCLUDE_DIR and Qt5_LIBRARY_DIR" OFF)
option(GR_PREFER_XCODEBUILD "Prefer xcodebuild to build macOS applications if available" ON)

if(EMSCRIPTEN)
  set(GR_DIRECTORY
      "/"
      CACHE STRING "Emscripten requires `/` as GRDIR" FORCE
  )
  set(GR_INSTALL
      OFF
      CACHE BOOL "Install is not supported on Emscripten" FORCE
  )
  set(CMAKE_C_FLAGS_DEBUG
      "--no-heap-copy -g3 -O0 -s ASSERTIONS=2 -s SAFE_HEAP=0 -s STACK_OVERFLOW_CHECK=2 -sNO_DISABLE_EXCEPTION_CATCHING=1 -sWASM=0"
  )
  set(CMAKE_C_FLAGS_RELEASE "--no-heap-copy -O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
endif()

if(GR_USE_BUNDLED_LIBRARIES)
  list(PREPEND CMAKE_FIND_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/3rdparty/build/")
  set(GR_THIRDPARTY_LIBRARY_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
  set(GR_THIRDPARTY_LIBRARY_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
  set(GR_THIRDPARTY_LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
  set(GR_THIRDPARTY_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

if(GR_FIND_QT5_BY_VARIABLES)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/optional/qt5")
endif()

if(WIN32)
  set(GR_PLUGIN_SUFFIX ".dll")
else()
  set(GR_PLUGIN_SUFFIX ".so")
endif()

if(APPLE)
  set(DEFAULT_SHARED_LIBRARY_SUFFIX ".dylib")
else()
  set(DEFAULT_SHARED_LIBRARY_SUFFIX "${GR_PLUGIN_SUFFIX}")
endif()

set(GR_SHARED_LIBRARY_SUFFIX
    "${DEFAULT_SHARED_LIBRARY_SUFFIX}"
    CACHE STRING "File suffix for shared libraries"
)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG
    TRUE
    CACHE BOOL ""
)

# Find the following packages always in system locations even if `GR_USE_BUNDLED_LIBRARIES` is set, because they are not
# located in 3rdparty
if(NOT EMSCRIPTEN)
  find_package(X11)
  find_package(Fontconfig)
  find_package(OpenGL OPTIONAL_COMPONENTS OpenGL)
  if(${CMAKE_VERSION} VERSION_GREATER "3.16.0")
    find_package(
      Qt6
      OPTIONAL_COMPONENTS
        Widgets
        Core
        Network
        Gui
        PrintSupport
    )
  endif()
  find_package(
    Qt5
    OPTIONAL_COMPONENTS
      Widgets
      Core
      Network
      Gui
      PrintSupport
  )
  find_package(Qt4)
endif()

# Find the following packages only in 3rdparty, if `GR_USE_BUNDLED_LIBRARIES` is set
if(GR_USE_BUNDLED_LIBRARIES)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
endif()
find_package(Freetype)
if(TARGET freetype AND NOT TARGET Freetype::Freetype)
  add_library(Freetype::Freetype ALIAS freetype)
endif()
find_package(Jpeg REQUIRED)
find_package(Libpng REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Qhull REQUIRED)
if(TARGET Qhull::qhullstatic_r AND NOT TARGET Qhull::qhull_r)
  add_library(Qhull::qhull_r ALIAS Qhull::qhullstatic_r)
endif()
if(NOT EMSCRIPTEN)
  find_package(Tiff)
  find_package(Ffmpeg 57.48.100)
  find_package(glfw3)
  if(TARGET glfw AND NOT TARGET Glfw::Glfw)
    add_library(Glfw::Glfw ALIAS glfw)
  endif()
  find_package(ZeroMQ)
  if(NOT TARGET ZeroMQ::ZeroMQ)
    if(TARGET libzmq)
      add_library(ZeroMQ::ZeroMQ ALIAS libzmq)
    elseif(TARGET libzmq-static)
      add_library(ZeroMQ::ZeroMQ ALIAS libzmq-static)
      get_target_property(ZeroMQ_LIBRARY libzmq-static LOCATION)
    endif()
  endif()
  find_package(Cairo)
  find_package(Agg)
  find_package(Gs)
endif()
find_package(XercesC)

if(APPLE)
  set(INSTALL_RPATH "${GR_DIRECTORY}/lib/;@loader_path/.")
else()
  set(INSTALL_RPATH "${GR_DIRECTORY}/lib/;$ORIGIN/.")
endif()

if(Qt4_FOUND
   OR (Qt5Widgets_FOUND
       AND Qt5Core_FOUND
       AND Qt5Network_FOUND
      )
   OR (Qt6Widgets_FOUND
       AND Qt6Core_FOUND
       AND Qt6Network_FOUND
      )
)
  if(GR_MANUAL_MOC_AND_RCC)
    if(NOT QT_MOC_EXECUTABLE)
      find_program(QT_MOC_EXECUTABLE moc CMAKE_FIND_ROOT_PATH_BOTH)
    endif()
    if(NOT QT_MOC_EXECUTABLE)
      message(FATAL_ERROR "Could not find moc but GR_MANUAL_MOC_AND_RCC is set.")
    endif()
    if(NOT QT_RCC_EXECUTABLE)
      find_program(QT_RCC_EXECUTABLE rcc CMAKE_FIND_ROOT_PATH_BOTH)
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
  if(NOT DEFINED Qt4_LIBRARY_DIR)
    get_filename_component(Qt4_LIBRARY_DIR "${QT_LIBRARY_DIR}/../.." ABSOLUTE)
  endif()
endif()
if(Qt5Widgets_FOUND
   AND Qt5Core_FOUND
   AND Qt5Network_FOUND
)
  set(QT5_MOC_INCLUDE_FLAGS "")
  foreach(DIR IN LISTS Qt5Core_INCLUDE_DIRS Qt5Gui_INCLUDE_DIRS Qt5Widgets_INCLUDE_DIRS)
    set(QT5_MOC_INCLUDE_FLAGS ${QT5_MOC_INCLUDE_FLAGS} -I${DIR})
  endforeach()
  if(NOT DEFINED Qt5_LIBRARY_DIR)
    get_filename_component(Qt5_LIBRARY_DIR "${Qt5_DIR}/../.." ABSOLUTE)
  endif()
endif()
if(Qt6Widgets_FOUND
   AND Qt6Core_FOUND
   AND Qt6Network_FOUND
)
  set(QT6_MOC_INCLUDE_FLAGS "")
  foreach(DIR IN LISTS Qt6Core_INCLUDE_DIRS Qt6Gui_INCLUDE_DIRS Qt6Widgets_INCLUDE_DIRS)
    set(QT6_MOC_INCLUDE_FLAGS ${QT6_MOC_INCLUDE_FLAGS} -I${DIR})
  endforeach()
  if(NOT DEFINED Qt6_LIBRARY_DIR)
    get_filename_component(Qt6_LIBRARY_DIR "${Qt6_DIR}/../.." ABSOLUTE)
  endif()
endif()

if(X11_FOUND)
  # older versions of FindX11.cmake set variables but do not create targets
  if(NOT TARGET X11::X11
     AND X11_INCLUDE_DIR
     AND X11_X11_LIB
     AND X11_Xft_LIB
  )
    add_library(X11::X11 UNKNOWN IMPORTED)
    set_target_properties(
      X11::X11
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${X11_INCLUDE_DIR}/"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${X11_X11_LIB}"
                 INTERFACE_LINK_LIBRARIES "${X11_Xft_LIB}"
    )
  endif()
  if(NOT TARGET X11::Xt
     AND X11_INCLUDE_DIR
     AND X11_Xt_LIB
     AND TARGET X11::X11
  )
    add_library(X11::Xt UNKNOWN IMPORTED)
    set_target_properties(
      X11::Xt
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${X11_INCLUDE_DIR}/"
                 IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                 IMPORTED_LOCATION "${X11_Xt_LIB}"
                 INTERFACE_LINK_LIBRARIES "X11::X11"
    )
  endif()
endif()

set(GKS_SOURCES
    lib/gks/afm.c
    lib/gks/font.c
    lib/gks/socket.c
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
    lib/gks/resample.c
)
if(EMSCRIPTEN)
  list(APPEND GKS_SOURCES js/jsplugin.c)
  list(REMOVE_ITEM GKS_SOURCES lib/gks/compress.c)
  list(REMOVE_ITEM GKS_SOURCES lib/gks/dl.c)
  list(REMOVE_ITEM GKS_SOURCES lib/gks/gksforbnd.c)
  list(REMOVE_ITEM GKS_SOURCES lib/gks/mf.c)
  list(REMOVE_ITEM GKS_SOURCES lib/gks/pdf.c)
  list(REMOVE_ITEM GKS_SOURCES lib/gks/plugin.c)
  list(REMOVE_ITEM GKS_SOURCES lib/gks/ps.c)
  list(REMOVE_ITEM GKS_SOURCES lib/gks/resample.c)
  list(REMOVE_ITEM GKS_SOURCES lib/gks/socket.c)
  list(REMOVE_ITEM GKS_SOURCES lib/gks/win.c)
endif()

add_library(gks_static STATIC ${GKS_SOURCES})
list(APPEND GKS_LIBRARIES gks_static)
if(NOT EMSCRIPTEN)
  add_library(gks_shared SHARED ${GKS_SOURCES})
  list(APPEND GKS_LIBRARIES gks_shared)
endif()

foreach(LIBRARY IN LISTS GKS_LIBRARIES)
  if(LIBRARY MATCHES "static")
    set(GKS_LINK_MODE PUBLIC)
  else()
    set(GKS_LINK_MODE PRIVATE)
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(${LIBRARY} PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()
  if(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    if(NOT EMSCRIPTEN)
      target_link_libraries(${LIBRARY} ${GKS_LINK_MODE} pthread)
    endif()
    target_link_libraries(${LIBRARY} ${GKS_LINK_MODE} m)
  endif()
  if(Freetype_FOUND)
    target_link_libraries(${LIBRARY} ${GKS_LINK_MODE} Freetype::Freetype)
  else()
    target_compile_definitions(
      ${LIBRARY}
      ${GKS_LINK_MODE}
      NO_FT
    )
  endif()
  target_link_libraries(${LIBRARY} ${GKS_LINK_MODE} ZLIB::ZLIB)
  target_compile_definitions(
    ${LIBRARY}
    ${GKS_LINK_MODE}
    HAVE_ZLIB
  )
  if(UNIX)
    target_link_libraries(${LIBRARY} ${GKS_LINK_MODE} dl)
  elseif(WIN32)
    target_link_libraries(${LIBRARY} ${GKS_LINK_MODE} ws2_32)
    target_link_libraries(${LIBRARY} ${GKS_LINK_MODE} msimg32)
    target_link_libraries(${LIBRARY} ${GKS_LINK_MODE} gdi32)
  endif()
  if(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    if(NOT EMSCRIPTEN)
      target_compile_options(${LIBRARY} ${GKS_LINK_MODE} -pthread)
    endif()
  endif()
  target_compile_options(${LIBRARY} PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  target_include_directories(
    ${LIBRARY} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
  target_compile_definitions(${LIBRARY} PRIVATE GRDIR="${GR_DIRECTORY}")
  set_target_properties(
    ${LIBRARY}
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               POSITION_INDEPENDENT_CODE ON
               PREFIX "lib"
               IMPORT_PREFIX "lib"
               OUTPUT_NAME GKS
               INSTALL_RPATH "${INSTALL_RPATH}"
  )
endforeach()
unset(GKS_LINK_MODE)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set_target_properties(gks_static PROPERTIES OUTPUT_NAME GKSstatic)
endif()
if(NOT EMSCRIPTEN)
  set_target_properties(gks_shared PROPERTIES SUFFIX "${GR_SHARED_LIBRARY_SUFFIX}" EXPORT_NAME GKS)
endif()
add_library(GR::GKS ALIAS gks_static)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/gr_version.h
     "#ifndef GR_VERSION\n#define GR_VERSION \"${GR_VERSION_FULL}\"\n#endif\n"
)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/gr_version.h PROPERTIES GENERATED TRUE)

set(GR_SOURCES
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
    lib/gr/stream.c
    lib/gr/md5.c
    lib/gr/shade.c
    lib/gr/spline.c
    lib/gr/strlib.c
    lib/gr/text.c
    lib/gr/mathtex2.c
    lib/gr/mathtex2_kerning.c
    lib/gr/mathtex2.tab.c
    lib/gr/threadpool.c
    ${CMAKE_CURRENT_BINARY_DIR}/gr_version.h
)

add_library(gr_static STATIC ${GR_SOURCES})
list(APPEND GR_LIBRARIES gr_static)
if(NOT EMSCRIPTEN)
  add_library(gr_shared SHARED ${GR_SOURCES})
  list(APPEND GR_LIBRARIES gr_shared)
endif()

foreach(LIBRARY IN LISTS GR_LIBRARIES)
  if(LIBRARY MATCHES "static")
    set(GR_LINK_MODE PUBLIC)
  else()
    set(GR_LINK_MODE PRIVATE)
  endif()
  if(NOT LIBRARY MATCHES "static")
    target_link_libraries(${LIBRARY} ${GR_LINK_MODE} gks_shared)
  else()
    target_link_libraries(${LIBRARY} ${GR_LINK_MODE} gks_static)
  endif()
  if(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    if(NOT EMSCRIPTEN)
      target_link_libraries(${LIBRARY} ${GR_LINK_MODE} pthread)
    endif()
    target_link_libraries(${LIBRARY} ${GR_LINK_MODE} m)
  endif()
  target_link_libraries(${LIBRARY} ${GR_LINK_MODE} Qhull::qhull_r)
  target_link_libraries(${LIBRARY} ${GR_LINK_MODE} Jpeg::Jpeg)
  target_link_libraries(${LIBRARY} ${GR_LINK_MODE} Libpng::Libpng)
  if(WIN32)
    target_link_libraries(${LIBRARY} ${GR_LINK_MODE} ws2_32)
  endif()
  if(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    if(NOT EMSCRIPTEN)
      target_compile_options(${LIBRARY} ${GR_LINK_MODE} -pthread)
    endif()
  endif()
  target_include_directories(
    ${LIBRARY} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
  target_include_directories(${LIBRARY} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(${LIBRARY} PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()
  target_compile_definitions(${LIBRARY} PRIVATE GRDIR="${GR_DIRECTORY}")
  target_compile_options(${LIBRARY} PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    ${LIBRARY}
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               POSITION_INDEPENDENT_CODE ON
               INSTALL_RPATH "${INSTALL_RPATH}"
               PREFIX "lib"
               IMPORT_PREFIX "lib"
               OUTPUT_NAME GR
  )
endforeach()
unset(GR_LINK_MODE)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set_target_properties(gr_static PROPERTIES OUTPUT_NAME GRstatic)
endif()
if(NOT EMSCRIPTEN)
  set_target_properties(gr_shared PROPERTIES SUFFIX "${GR_SHARED_LIBRARY_SUFFIX}" EXPORT_NAME GR)
  add_library(GR::GR ALIAS gr_shared)
endif()

set(GR3_SOURCES
    lib/gr3/gr3.c
    lib/gr3/gr3_convenience.c
    lib/gr3/gr3_gr.c
    lib/gr3/gr3_html.c
    lib/gr3/gr3_jpeg.c
    lib/gr3/gr3_mc.c
    lib/gr3/gr3_png.c
    lib/gr3/gr3_povray.c
    lib/gr3/gr3_slices.c
    lib/gr3/gr3_sr.c
)

add_library(gr3_static STATIC ${GR3_SOURCES})
list(APPEND GR3_LIBRARIES gr3_static)
if(NOT EMSCRIPTEN)
  add_library(gr3_shared SHARED ${GR3_SOURCES})
  list(APPEND GR3_LIBRARIES gr3_shared)
endif()
foreach(LIBRARY IN LISTS GR3_LIBRARIES)
  if(LIBRARY MATCHES "static")
    set(GR3_LINK_MODE PUBLIC)
  else()
    set(GR3_LINK_MODE PRIVATE)
  endif()
  if(APPLE)
    target_sources(${LIBRARY} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr3/gr3_cgl.c)
  elseif(UNIX AND NOT APPLE)
    target_sources(${LIBRARY} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr3/gr3_glx.c)
  elseif(WIN32)
    target_sources(${LIBRARY} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr3/gr3_win.c)
  endif()
  if(LIBRARY MATCHES "static")
    target_link_libraries(${LIBRARY} ${GR3_LINK_MODE} gr_static)
    target_link_libraries(${LIBRARY} ${GR3_LINK_MODE} gks_static)
  else()
    target_link_libraries(${LIBRARY} ${GR3_LINK_MODE} gr_shared)
    target_link_libraries(${LIBRARY} ${GR3_LINK_MODE} gks_shared)
  endif()
  target_link_libraries(${LIBRARY} ${GR3_LINK_MODE} Jpeg::Jpeg)
  target_link_libraries(${LIBRARY} ${GR3_LINK_MODE} Libpng::Libpng)
  if(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    if(NOT EMSCRIPTEN)
      target_link_libraries(${LIBRARY} ${GR3_LINK_MODE} pthread)
    endif()
    target_link_libraries(${LIBRARY} ${GR3_LINK_MODE} m)
  endif()
  if((APPLE OR WIN32) AND OpenGL_FOUND)
    target_link_libraries(${LIBRARY} ${GR3_LINK_MODE} OpenGL::GL)
  endif()
  if(APPLE)
    # Apple has deprecated OpenGL in macOS 10.14
    target_compile_definitions(${LIBRARY} PRIVATE GL_SILENCE_DEPRECATION)
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(${LIBRARY} PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()
  if(EMSCRIPTEN)
    target_compile_definitions(${LIBRARY} PRIVATE NO_THREADS)
  endif()
  target_compile_definitions(${LIBRARY} PRIVATE GRDIR="${GR_DIRECTORY}")
  if(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    if(NOT EMSCRIPTEN)
      target_compile_options(${LIBRARY} ${GR3_LINK_MODE} -pthread)
    endif()
  endif()
  target_compile_options(${LIBRARY} PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  target_include_directories(
    ${LIBRARY} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/gr3/>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
  set_target_properties(
    ${LIBRARY}
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               POSITION_INDEPENDENT_CODE ON
               INSTALL_RPATH "${INSTALL_RPATH}"
               PREFIX "lib"
               IMPORT_PREFIX "lib"
               OUTPUT_NAME GR3
  )
endforeach()
unset(GR3_LINK_MODE)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set_target_properties(gr3_static PROPERTIES OUTPUT_NAME GR3static)
endif()
if(NOT EMSCRIPTEN)
  set_target_properties(gr3_shared PROPERTIES SUFFIX "${GR_SHARED_LIBRARY_SUFFIX}" EXPORT_NAME GR3)
  add_library(GR::GR3 ALIAS gr3_shared)
endif()

if(UNIX AND NOT APPLE)
  if(TARGET X11::X11
     AND TARGET OpenGL::GLX
     AND TARGET OpenGL::GL
  )
    add_library(gr3platform SHARED lib/gr3/gr3_platform_glx.c)
    target_link_libraries(gr3platform PUBLIC GR::GR)
    target_link_libraries(gr3platform PUBLIC X11::X11)
    target_link_libraries(gr3platform PUBLIC OpenGL::GLX)
    target_link_libraries(gr3platform PUBLIC OpenGL::GL)
    target_compile_options(gr3platform PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
    set_target_properties(
      gr3platform
      PROPERTIES C_STANDARD 90
                 C_EXTENSIONS OFF
                 C_STANDARD_REQUIRED ON
                 POSITION_INDEPENDENT_CODE ON
                 INSTALL_RPATH "${INSTALL_RPATH}"
                 OUTPUT_NAME GR3platform
    )
  else()
    target_compile_definitions(gr3_static PUBLIC -DNO_GL)
    if(NOT EMSCRIPTEN)
      target_compile_definitions(gr3_shared PUBLIC -DNO_GL)
    endif()
    message(WARNING "libGR3platform.so requires X11 and will not be built.")
  endif()
elseif(NOT OpenGL_FOUND)
  target_compile_definitions(gr3_static PUBLIC -DNO_GL)
  if(NOT EMSCRIPTEN)
    target_compile_definitions(gr3_shared PUBLIC -DNO_GL)
  endif()
endif()

set(GRM_SOURCES
    lib/grm/src/grm/args.c
    lib/grm/src/grm/backtrace.c
    lib/grm/src/grm/base64.c
    lib/grm/src/grm/bson.c
    lib/grm/src/grm/dump.c
    lib/grm/src/grm/dynamic_args_array.c
    lib/grm/src/grm/error.c
    lib/grm/src/grm/event.c
    lib/grm/src/grm/interaction.cxx
    lib/grm/src/grm/json.c
    lib/grm/src/grm/layout_c.cxx
    lib/grm/src/grm/layout.cxx
    lib/grm/src/grm/layout_error.cxx
    lib/grm/src/grm/logging.c
    lib/grm/src/grm/memwriter.c
    lib/grm/src/grm/net.c
    lib/grm/src/grm/plot.cxx
    lib/grm/src/grm/util.c
    lib/grm/src/grm/import.cxx
    lib/grm/src/grm/utilcpp.cxx
    lib/grm/src/grm/datatype/double_map.c
    lib/grm/src/grm/datatype/size_t_list.c
    lib/grm/src/grm/datatype/string_array_map.c
    lib/grm/src/grm/datatype/string_list.c
    lib/grm/src/grm/datatype/string_map.c
    lib/grm/src/grm/datatype/uint_map.c
    lib/grm/src/grm/dom_render/context.cxx
    lib/grm/src/grm/dom_render/render.cxx
    lib/grm/src/grm/dom_render/Drawable.cxx
    lib/grm/src/grm/dom_render/graphics_tree/Comment.cxx
    lib/grm/src/grm/dom_render/graphics_tree/Document.cxx
    lib/grm/src/grm/dom_render/graphics_tree/Element.cxx
    lib/grm/src/grm/dom_render/graphics_tree/Node.cxx
    lib/grm/src/grm/dom_render/graphics_tree/Value.cxx
    lib/grm/src/grm/dom_render/graphics_tree/util.cxx
    lib/grm/src/grm/dom_render/ManageCustomColorIndex.cxx
    lib/grm/src/grm/dom_render/ManageGRContextIds.cxx
    lib/grm/src/grm/dom_render/ManageZIndex.cxx
)

add_library(grm_static STATIC ${GRM_SOURCES})
list(APPEND GRM_LIBRARIES grm_static)
if(NOT EMSCRIPTEN)
  add_library(grm_shared SHARED ${GRM_SOURCES})
  list(APPEND GRM_LIBRARIES grm_shared)
  # TODO: Build this only in a debug build / for testing
  add_library(grm_shared_internal SHARED ${GRM_SOURCES})
  list(APPEND GRM_LIBRARIES grm_shared_internal)
endif()

foreach(LIBRARY IN LISTS GRM_LIBRARIES)
  if(LIBRARY MATCHES "static")
    set(GRM_LINK_MODE PUBLIC)
  else()
    set(GRM_LINK_MODE PRIVATE)
  endif()
  if(NOT LIBRARY MATCHES "internal")
    set_target_properties(${LIBRARY} PROPERTIES C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden)
  endif()
  if(LIBRARY MATCHES "static")
    target_link_libraries(${LIBRARY} ${GRM_LINK_MODE} gr_static)
    target_link_libraries(${LIBRARY} ${GRM_LINK_MODE} gr3_static)
    target_link_libraries(${LIBRARY} ${GRM_LINK_MODE} gks_static)
  else()
    target_link_libraries(${LIBRARY} ${GRM_LINK_MODE} gr_shared)
    target_link_libraries(${LIBRARY} ${GRM_LINK_MODE} gr3_shared)
    target_link_libraries(${LIBRARY} ${GRM_LINK_MODE} gks_shared)
  endif()
  if(TARGET XercesC::XercesC)
    target_link_libraries(${LIBRARY} ${GRM_LINK_MODE} XercesC::XercesC)
  else()
    target_compile_definitions(${LIBRARY} PRIVATE NO_XERCES_C)
  endif()
  if(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
    target_link_libraries(${LIBRARY} ${GRM_LINK_MODE} m)
  endif()
  if(WIN32)
    target_link_libraries(${LIBRARY} ${GRM_LINK_MODE} ws2_32)
  endif()
  target_include_directories(
    ${LIBRARY}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/include/>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/src/> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
  target_compile_definitions(${LIBRARY} PRIVATE BUILDING_GR)
  target_compile_definitions(${LIBRARY} PRIVATE GRDIR="${GR_DIRECTORY}")
  target_compile_options(${LIBRARY} PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(${LIBRARY} PRIVATE /permissive-)
  endif()
  set_target_properties(
    ${LIBRARY}
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               CXX_STANDARD 17
               CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               POSITION_INDEPENDENT_CODE ON
               INSTALL_RPATH "${INSTALL_RPATH}"
               PREFIX "lib"
               IMPORT_PREFIX "lib"
               OUTPUT_NAME GRM
  )
endforeach()
unset(GRM_LINK_MODE)
if(NOT EMSCRIPTEN)
  set_target_properties(grm_shared_internal PROPERTIES C_VISIBILITY_PRESET default)
  set_target_properties(grm_shared_internal PROPERTIES OUTPUT_NAME GRM_int)
  target_include_directories(grm_shared_internal PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/>)
  if(WIN32)
    target_compile_definitions(grm_static PRIVATE GR_STATIC_LIB)
    target_compile_definitions(grm_shared PRIVATE BUILDING_DLL)
    target_compile_definitions(grm_shared_internal PRIVATE BUILDING_DLL)
  endif()
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set_target_properties(grm_static PROPERTIES OUTPUT_NAME GRMstatic)
  endif()
  set_target_properties(grm_shared PROPERTIES SUFFIX "${GR_SHARED_LIBRARY_SUFFIX}" EXPORT_NAME GRM)
  set_target_properties(grm_shared_internal PROPERTIES SUFFIX "${GR_SHARED_LIBRARY_SUFFIX}")
  add_library(GR::GRM ALIAS grm_shared)
endif()

if(NOT EMSCRIPTEN)
  set(GR_REPORT "GKS plugins:\n")

  add_library(cairoplugin SHARED lib/gks/plugin/cairoplugin.c)
  target_link_libraries(cairoplugin PUBLIC gks_static)
  if(Cairo_FOUND)
    target_link_libraries(cairoplugin PRIVATE Cairo::Cairo)
    target_link_libraries(cairoplugin PRIVATE Jpeg::Jpeg)
    string(APPEND GR_REPORT "- cairoplugin:\n")
    string(APPEND GR_REPORT "\tPNG output: Yes\n")
    string(APPEND GR_REPORT "\tBMP output: Yes\n")
    string(APPEND GR_REPORT "\tJPEG output: Yes\n")

    if(Tiff_FOUND)
      target_link_libraries(cairoplugin PRIVATE Tiff::Tiff)
      string(APPEND GR_REPORT "\tTiff output: Yes\n")
    else()
      string(APPEND GR_REPORT "\tTiff output: No (libtiff not found)\n")
      target_compile_definitions(cairoplugin PRIVATE NO_TIFF)
    endif()
    target_compile_options(cairoplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
    string(APPEND GR_REPORT "\tSixel output: Yes\n")
    string(APPEND GR_REPORT "\tMemory output: Yes\n")
    string(APPEND GR_REPORT "\tX11 output: No (not available in CMake build)\n")
  else()
    target_compile_definitions(cairoplugin PRIVATE NO_CAIRO)
    string(APPEND GR_REPORT "- cairoplugin: No (Cairo not found)\n")
  endif()
  # Cairo X11 support is disabled to allow users to generate images using Cairo on systems without X11 installed
  target_compile_definitions(cairoplugin PRIVATE NO_X11)
  set_target_properties(
    cairoplugin
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )

  add_library(aggplugin SHARED lib/gks/plugin/aggplugin.cxx)
  target_link_libraries(aggplugin PUBLIC gks_static)
  if(Agg_FOUND)
    target_link_libraries(aggplugin PRIVATE Agg::Agg)
    target_link_libraries(aggplugin PRIVATE Jpeg::Jpeg)
    target_link_libraries(aggplugin PRIVATE Libpng::Libpng)
    string(APPEND GR_REPORT "- aggplugin: Yes\n")
    target_compile_options(aggplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      # Do not define `min` and `max` as macros because this can have unintended side effects (like error messages when
      # using `std::min`)
      target_compile_definitions(aggplugin PRIVATE NOMINMAX)
    endif()
  else()
    target_compile_definitions(aggplugin PRIVATE NO_AGG)
    string(APPEND GR_REPORT "- aggplugin: No (Agg not found)\n")
  endif()
  set_target_properties(
    aggplugin
    PROPERTIES CXX_STANDARD 11
               CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )

  add_library(videoplugin SHARED lib/gks/plugin/gif.c lib/gks/plugin/vc.c lib/gks/plugin/videoplugin.c)
  target_link_libraries(videoplugin PUBLIC gks_static)
  if(NOT Cairo_FOUND)
    target_compile_definitions(videoplugin PRIVATE NO_CAIRO)
    target_compile_definitions(videoplugin PRIVATE NO_AV)
    string(APPEND GR_REPORT "- videoplugin: No (Cairo not found)\n")
  elseif(NOT Ffmpeg_FOUND)
    target_compile_definitions(videoplugin PRIVATE NO_CAIRO)
    target_compile_definitions(videoplugin PRIVATE NO_AV)
    string(APPEND GR_REPORT "- videoplugin: No (ffmpeg / ogg / theora / vpx / openh264 not found)\n")
  else()
    string(APPEND GR_REPORT "- videoplugin:\n")
    string(APPEND GR_REPORT "\tMP4 output: Yes\n")
    string(APPEND GR_REPORT "\tMOV output: Yes\n")
    string(APPEND GR_REPORT "\tWEBM output: Yes\n")
    string(APPEND GR_REPORT "\tOGG output: Yes\n")
    string(APPEND GR_REPORT "\tGIF output: Yes\n")
    string(APPEND GR_REPORT "\tAPNG output: Yes\n")
    target_link_libraries(videoplugin PUBLIC Ffmpeg::Ffmpeg)
  endif()
  target_compile_options(videoplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    videoplugin
    PROPERTIES C_STANDARD 99
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               LINKER_LANGUAGE CXX
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )

  add_library(pgfplugin SHARED lib/gks/plugin/pgfplugin.c)
  target_link_libraries(pgfplugin PUBLIC gks_static)
  target_link_libraries(pgfplugin PUBLIC Libpng::Libpng)
  target_compile_options(pgfplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    pgfplugin
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )
  string(APPEND GR_REPORT "- pgfplugin:\n")
  string(APPEND GR_REPORT "\tTeX output: Yes\n")

  add_library(wmfplugin SHARED lib/gks/plugin/wmfplugin.c)
  target_link_libraries(wmfplugin PUBLIC gks_static)
  target_compile_options(wmfplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    wmfplugin
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )
  string(APPEND GR_REPORT "- wmfplugin:\n")
  string(APPEND GR_REPORT "\tWMF output: Yes\n")

  add_library(gsplugin SHARED lib/gks/plugin/gsplugin.c)
  target_link_libraries(gsplugin PUBLIC gks_static)
  if(NOT TARGET X11::Xt)
    string(APPEND GR_REPORT "- gsplugin: No (X11 / Xt not found)\n")
    target_compile_definitions(gsplugin PRIVATE NO_GS)
  elseif(NOT Gs_FOUND)
    string(APPEND GR_REPORT "- gsplugin: No (ghostscript not found)\n")
    target_compile_definitions(gsplugin PRIVATE NO_GS)
  else()
    target_link_libraries(gsplugin PUBLIC X11::Xt)
    target_link_libraries(gsplugin PUBLIC Gs::Gs)
    string(APPEND GR_REPORT "- gsplugin:\n")
    string(APPEND GR_REPORT "\tPNG outout: Yes\n")
    string(APPEND GR_REPORT "\tBMP outout: Yes\n")
    string(APPEND GR_REPORT "\tJPEG outout: Yes\n")
    string(APPEND GR_REPORT "\tTiff outout: Yes\n")
  endif()
  target_compile_options(gsplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    gsplugin
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )

  add_library(svgplugin SHARED lib/gks/plugin/svgplugin.c)
  target_link_libraries(svgplugin PUBLIC gks_static)
  target_link_libraries(svgplugin PUBLIC Libpng::Libpng)
  target_compile_options(svgplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    svgplugin
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )
  string(APPEND GR_REPORT "- svgplugin:\n")
  string(APPEND GR_REPORT "\tSVG output: Yes\n")

  add_library(glplugin SHARED lib/gks/plugin/glplugin.c)
  target_link_libraries(glplugin PUBLIC gks_static)
  if(TARGET Glfw::Glfw AND OpenGL_FOUND)
    string(APPEND GR_REPORT "- glplugin: Yes\n")
    target_link_libraries(glplugin PUBLIC Glfw::Glfw)
    target_link_libraries(glplugin PUBLIC OpenGL::GL)
  else()
    string(APPEND GR_REPORT "- glplugin: No (GLFW / OpenGL not found)\n")
    target_compile_definitions(glplugin PRIVATE NO_GLFW)
  endif()
  if(NOT Freetype_FOUND)
    target_compile_definitions(glplugin PRIVATE NO_FT)
  endif()
  if(APPLE)
    # Apple has deprecated OpenGL in macOS 10.14
    target_compile_definitions(glplugin PRIVATE GL_SILENCE_DEPRECATION)
  endif()
  target_compile_options(glplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    glplugin
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )

  add_library(zmqplugin SHARED lib/gks/plugin/zmqplugin.c)
  target_link_libraries(zmqplugin PUBLIC gks_static)
  if(ZeroMQ_FOUND)
    string(APPEND GR_REPORT "- zmqplugin: Yes\n")
    target_link_libraries(zmqplugin PUBLIC ZeroMQ::ZeroMQ)
  else()
    string(APPEND GR_REPORT "- zmqplugin: No (ZeroMQ not found)\n")
    target_compile_definitions(zmqplugin PRIVATE NO_ZMQ)
  endif()
  target_compile_options(zmqplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    zmqplugin
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )

  add_library(x11plugin SHARED lib/gks/plugin/x11plugin.c)
  target_link_libraries(x11plugin PUBLIC gks_static)
  if(NOT Freetype_FOUND)
    target_compile_definitions(x11plugin PRIVATE NO_FT)
  endif()
  if(TARGET X11::Xt)
    if(Xft_FOUND AND Fontconfig_FOUND)
      target_link_libraries(x11plugin PUBLIC Fontconfig::Fontconfig)
    else()
      target_compile_definitions(x11plugin PRIVATE NO_XFT)
    endif()
    target_link_libraries(x11plugin PUBLIC X11::Xt)
    string(APPEND GR_REPORT "- x11plugin: Yes\n")
  else()
    target_compile_definitions(x11plugin PRIVATE NO_X11)
    target_compile_definitions(x11plugin PRIVATE NO_XFT)
    string(APPEND GR_REPORT "- x11plugin: No (X11 / Xft / Xt not found)\n")
  endif()
  target_compile_options(x11plugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    x11plugin
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               CXX_STANDARD 11
               CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )

  add_library(qt5plugin SHARED lib/gks/plugin/qt5plugin.cxx)
  target_link_libraries(qt5plugin PUBLIC gks_static)
  if(Qt5Widgets_FOUND)
    string(APPEND GR_REPORT "- qt5plugin: Yes\n")
    target_link_libraries(qt5plugin PUBLIC Qt5::Widgets)
  else()
    string(APPEND GR_REPORT "- qt5plugin: No (Qt5 not found)\n")
    target_compile_definitions(qt5plugin PRIVATE NO_QT5)
  endif()
  target_compile_options(qt5plugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    qt5plugin
    PROPERTIES CXX_STANDARD 11
               CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )

  add_library(qt6plugin SHARED lib/gks/plugin/qt6plugin.cxx)
  target_link_libraries(qt6plugin PUBLIC gks_static)
  if(Qt6Widgets_FOUND AND ${CMAKE_VERSION} VERSION_GREATER "3.16.0")
    string(APPEND GR_REPORT "- qt6plugin: Yes\n")
    target_link_libraries(qt6plugin PUBLIC Qt6::Widgets)
    set_target_properties(
      qt6plugin
      PROPERTIES CXX_STANDARD 17
                 CXX_EXTENSIONS OFF
                 CXX_STANDARD_REQUIRED ON
                 PREFIX ""
                 SUFFIX "${GR_PLUGIN_SUFFIX}"
    )
  else()
    if(${CMAKE_VERSION} VERSION_LESS "3.16.0")
      string(APPEND GR_REPORT "- qt6plugin: No (CMake version < 3.16.0)\n")
    else()
      string(APPEND GR_REPORT "- qt6plugin: No (Qt6 not found)\n")
    endif()
    set_target_properties(
      qt6plugin
      PROPERTIES CXX_STANDARD 11
                 CXX_EXTENSIONS OFF
                 CXX_STANDARD_REQUIRED ON
                 PREFIX ""
                 SUFFIX "${GR_PLUGIN_SUFFIX}"
    )
    target_compile_definitions(qt6plugin PRIVATE NO_QT6)
  endif()
  target_compile_options(qt6plugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})

  add_library(qtplugin SHARED lib/gks/plugin/qtplugin.cxx)
  target_link_libraries(qtplugin PUBLIC gks_static)
  if(Qt4_FOUND)
    string(APPEND GR_REPORT "- qtplugin: Yes\n")
    target_link_libraries(qtplugin PUBLIC Qt4::QtGui)
  else()
    string(APPEND GR_REPORT "- qtplugin: No (Qt4 not found)\n")
    target_compile_definitions(qtplugin PRIVATE NO_QT)
  endif()
  target_compile_options(qtplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    qtplugin
    PROPERTIES CXX_STANDARD 11
               CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )

  add_library(gtkplugin SHARED lib/gks/plugin/gtkplugin.c)
  target_link_libraries(gtkplugin PUBLIC gks_static)
  set_target_properties(
    gtkplugin
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )
  target_compile_options(gtkplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  string(APPEND GR_REPORT "- gtkplugin: No (not implemented yet)\n")
endif()

if(APPLE)
  add_library(quartzplugin SHARED lib/gks/plugin/quartzplugin.m)
  target_link_libraries(quartzplugin PUBLIC gks_static)
  if(ZeroMQ_FOUND)
    target_link_libraries(quartzplugin PUBLIC ZeroMQ::ZeroMQ)
    target_link_libraries(quartzplugin PUBLIC "-framework Foundation -framework ApplicationServices -framework AppKit")
    target_compile_definitions(quartzplugin PRIVATE GRDIR="${GR_DIRECTORY}")
  else()
    target_compile_definitions(quartzplugin PRIVATE NO_GKSTERM)
  endif()
  target_compile_options(quartzplugin PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(
    quartzplugin
    PROPERTIES C_STANDARD 90
               C_EXTENSIONS OFF
               C_STANDARD_REQUIRED ON
               PREFIX ""
               SUFFIX "${GR_PLUGIN_SUFFIX}"
  )
endif()

if(Qt4_FOUND)
  add_library(qt4gr SHARED lib/gr/qtgr/grwidget.cxx)
  target_link_libraries(qt4gr PUBLIC GR::GR)
  target_link_libraries(qt4gr PUBLIC Qt4::QtCore Qt4::QtGui)
  if(MINGW AND CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
    target_compile_options(qt4gr PRIVATE -fno-exceptions)
  endif()
  target_compile_options(qt4gr PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(qt4gr PROPERTIES SUFFIX "${GR_SHARED_LIBRARY_SUFFIX}" INSTALL_RPATH "${INSTALL_RPATH}")
  if(GR_MANUAL_MOC_AND_RCC)
    set_target_properties(qt4gr PROPERTIES AUTOMOC OFF AUTORCC OFF)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_qt4_grwidget.cxx
      COMMAND ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${QT4_MOC_INCLUDE_FLAGS}
              ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.h -o ${CMAKE_CURRENT_BINARY_DIR}/moc_qt4_grwidget.cxx
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.cxx
    )
    target_sources(qt4gr PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/moc_qt4_grwidget.cxx)
  else()
    set_target_properties(qt4gr PROPERTIES AUTOMOC ON AUTORCC ON)
    target_sources(qt4gr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.h)
  endif()
  add_library(GR::qt4gr ALIAS qt4gr)
endif()

if(Qt5Widgets_FOUND AND Qt5Core_FOUND)
  add_library(qt5gr SHARED lib/gr/qtgr/grwidget.cxx)
  target_link_libraries(qt5gr PUBLIC GR::GR)
  target_link_libraries(qt5gr PUBLIC Qt5::Core Qt5::Widgets)
  if(MINGW AND CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
    target_compile_options(qt5gr PRIVATE -fno-exceptions)
  endif()
  target_compile_options(qt5gr PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(qt5gr PROPERTIES SUFFIX "${GR_SHARED_LIBRARY_SUFFIX}" INSTALL_RPATH "${INSTALL_RPATH}")
  if(GR_MANUAL_MOC_AND_RCC)
    set_target_properties(qt5gr PROPERTIES AUTOMOC OFF AUTORCC OFF)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_qt5_grwidget.cxx
      COMMAND ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${QT5_MOC_INCLUDE_FLAGS}
              ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.h -o ${CMAKE_CURRENT_BINARY_DIR}/moc_qt5_grwidget.cxx
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.cxx
    )
    target_sources(qt5gr PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/moc_qt5_grwidget.cxx)
  else()
    set_target_properties(qt5gr PROPERTIES AUTOMOC ON AUTORCC ON)
    target_sources(qt5gr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.h)
  endif()
  add_library(GR::qt5gr ALIAS qt5gr)
endif()

if(Qt6Widgets_FOUND AND Qt6Core_FOUND)
  add_library(qt6gr SHARED lib/gr/qtgr/grwidget.cxx)
  target_link_libraries(qt6gr PUBLIC GR::GR)
  target_link_libraries(qt6gr PUBLIC Qt6::Core Qt6::Widgets)
  if(MINGW AND CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
    target_compile_options(qt6gr PRIVATE -fno-exceptions)
  endif()
  target_compile_options(qt6gr PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(qt6gr PROPERTIES SUFFIX "${GR_SHARED_LIBRARY_SUFFIX}" INSTALL_RPATH "${INSTALL_RPATH}")
  if(GR_MANUAL_MOC_AND_RCC)
    set_target_properties(qt6gr PROPERTIES AUTOMOC OFF AUTORCC OFF)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_qt6_grwidget.cxx
      COMMAND ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${QT6_MOC_INCLUDE_FLAGS}
              ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.h -o ${CMAKE_CURRENT_BINARY_DIR}/moc_qt6_grwidget.cxx
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.cxx
    )
    target_sources(qt6gr PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/moc_qt6_grwidget.cxx)
  else()
    set_target_properties(qt6gr PROPERTIES AUTOMOC ON AUTORCC ON)
    target_sources(qt6gr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/gr/qtgr/grwidget.h)
  endif()
  add_library(GR::qt6gr ALIAS qt6gr)
endif()

if(Qt6Widgets_FOUND
   AND Qt6Core_FOUND
   AND (NOT MINGW OR NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
)
  add_library(
    grplotWidget SHARED
    lib/grm/grplot/CollapsibleSection.cpp
    lib/grm/grplot/gredit/AddElementWidget.cpp
    lib/grm/grplot/gredit/BoundingLogic.cpp
    lib/grm/grplot/gredit/BoundingObject.cpp
    lib/grm/grplot/gredit/ColorPickerRGB.cpp
    lib/grm/grplot/gredit/CustomTreeWidgetItem.cpp
    lib/grm/grplot/gredit/EditElementWidget.cpp
    lib/grm/grplot/gredit/PreviewTextWidget.cpp
    lib/grm/grplot/gredit/TableWidget.cpp
    lib/grm/grplot/gredit/TreeWidget.cpp
    lib/grm/grplot/grplotWidget.cxx
    lib/grm/grplot/qtterm/ArgsWrapper.cpp
    lib/grm/grplot/qtterm/Receiver.cpp
    lib/grm/grplot/util.cxx
  )
  target_link_libraries(grplotWidget PUBLIC GR::GRM)
  target_link_libraries(grplotWidget PUBLIC gks_shared)
  target_link_libraries(grplotWidget PUBLIC GR::GR)
  target_link_libraries(grplotWidget PUBLIC Qt6::Core Qt6::Widgets)
  target_link_libraries(grplotWidget PUBLIC GR::qt6gr)
  target_include_directories(grplotWidget INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/grplotWidget>)
  target_compile_definitions(grplotWidget PRIVATE GRDIR="${GR_DIRECTORY}")
  if(NOT TARGET XercesC::XercesC)
    target_compile_definitions(grplotWidget PRIVATE NO_XERCES_C)
  endif()
  target_compile_options(grplotWidget PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(grplotWidget PROPERTIES SUFFIX "${GR_SHARED_LIBRARY_SUFFIX}" INSTALL_RPATH "${INSTALL_RPATH}")
  if(GR_MANUAL_MOC_AND_RCC)
    set_target_properties(grplotWidget PROPERTIES AUTOMOC OFF AUTORCC OFF)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotWidget.cxx
      COMMAND
        ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${QT6_MOC_INCLUDE_FLAGS}
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotWidget.hxx -o ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotWidget.cxx
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotWidget.cxx
    )
    target_sources(grplotWidget PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotWidget.cxx)
  else()
    set_target_properties(grplotWidget PROPERTIES AUTOMOC ON AUTORCC ON)
    target_sources(grplotWidget PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotWidget.hxx)
  endif()
  add_library(GR::grplotWidget ALIAS grplotWidget)
endif()

string(APPEND GR_REPORT "\nGKS applications:\n")

if(APPLE)
  if(ZeroMQ_FOUND)
    if(GR_PREFER_XCODEBUILD)
      find_program(XCODE_BUILD xcodebuild CMAKE_FIND_ROOT_PATH_BOTH)
      if(XCODE_BUILD)
        execute_process(COMMAND ${XCODE_BUILD} -version RESULT_VARIABLE XCODE_BUILD_CHECK_RESULT)
        if(NOT XCODE_BUILD_CHECK_RESULT EQUAL 0)
          set(XCODE_BUILD "XCODE_BUILD-NOTFOUND")
        endif()
      endif()
    endif()
    if(GR_PREFER_XCODEBUILD AND XCODE_BUILD)
      set(GKSTERM_SOURCES
          lib/gks/quartz/GKSTerm.h
          lib/gks/quartz/GKSTerm.icns
          lib/gks/quartz/GKSTerm.m
          lib/gks/quartz/GKSTerm_Prefix.pch
          lib/gks/quartz/GKSView.h
          lib/gks/quartz/GKSView.m
          lib/gks/quartz/Info.plist
          lib/gks/quartz/MacOSXBundleInfo.plist.in
          lib/gks/quartz/main.m
          lib/gks/quartz/English.lproj/InfoPlist.strings
          lib/gks/quartz/English.lproj/ExtendSavePanel.nib/designable.nib
          lib/gks/quartz/English.lproj/ExtendSavePanel.nib/keyedobjects.nib
          lib/gks/quartz/English.lproj/MainMenu.nib/designable.nib
          lib/gks/quartz/English.lproj/MainMenu.nib/keyedobjects.nib
          lib/gks/quartz/GKSTerm.xcodeproj/project.pbxproj
          lib/gks/quartz/GKSTerm.xcodeproj/project.xcworkspace
          lib/gks/quartz/GKSTerm.xcodeproj/project.xcworkspace/contents.xcworkspacedata
      )
      add_custom_target(
        GKSTerm ALL
        COMMAND ${XCODE_BUILD} -arch ${CMAKE_SYSTEM_PROCESSOR} -project GKSTerm.xcodeproj
                CONFIGURATION_BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}/GKSTerm
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/GKSTerm-build/quartz
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/GKSTerm-build/quartz/GKSTerm.xcodeproj ZeroMQ::ZeroMQ gks_static
      )
      # Disable autoformat to not break the `sed` command
      # cmake-format: off
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/GKSTerm-build/quartz/GKSTerm.xcodeproj
        COMMAND ${CMAKE_COMMAND} -E copy_directory lib/gks ${CMAKE_CURRENT_BINARY_DIR}/GKSTerm-build
        COMMAND sed -e s%../../../3rdparty/build/lib/libzmq.a%${ZeroMQ_LIBRARY}% -e s%../../../3rdparty/build/include/zmq.h%${ZeroMQ_INCLUDE_DIR}/zmq.h% ${CMAKE_SOURCE_DIR}/lib/gks/quartz/GKSTerm.xcodeproj/project.pbxproj >${CMAKE_CURRENT_BINARY_DIR}/GKSTerm-build/quartz/GKSTerm.xcodeproj/project.pbxproj
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${GKS_SOURCES} ${GKSTERM_SOURCES}
      )
      # cmake-format: on
      string(APPEND GR_REPORT "- GKSTerm: Yes (Xcode build)\n")
    else()
      add_executable(GKSTerm MACOSX_BUNDLE lib/gks/quartz/GKSTerm.m lib/gks/quartz/GKSView.m lib/gks/quartz/main.m)
      target_link_libraries(GKSTerm PUBLIC ZeroMQ::ZeroMQ)
      target_link_libraries(
        GKSTerm PUBLIC gks_static
                       "-framework CoreGraphics -framework CoreFoundation -framework CoreVideo -framework Cocoa"
      )

      add_custom_command(
        TARGET GKSTerm
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/quartz/English.lproj
                ${CMAKE_CURRENT_BINARY_DIR}/GKSTerm.app/Contents/Resources/English.lproj
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/GKSTerm.app/Contents/Resources/English.lproj
      )
      target_sources(GKSTerm PRIVATE lib/gks/quartz/GKSTerm.icns)
      set_source_files_properties(lib/gks/quartz/GKSTerm.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources/)
      target_compile_options(GKSTerm PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
      set_target_properties(
        GKSTerm
        PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "GKSTerm"
                   MACOSX_BUNDLE_GUI_IDENTIFIER "de.fz-juelich.GKSTerm"
                   MACOSX_BUNDLE_ICON_FILE "GKSTerm"
                   MACOSX_BUNDLE_BUNDLE_VERSION "${GR_VERSION}"
                   MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/quartz/MacOSXBundleInfo.plist.in"
      )
      set(GKSTerm_MACOSX_BUNDLE_SIGNATURE "GKST")
      set(GKSTerm_MACOSX_MAIN_NIB_FILE "MainMenu")
      string(APPEND GR_REPORT "- GKSTerm: Yes (Build without Xcode)\n")
    endif()
  else()
    string(APPEND GR_REPORT "- GKSTerm: No (ZeroMQ not found)\n")
  endif()
endif()

if(Qt4_FOUND
   OR (Qt5Widgets_FOUND
       AND Qt5Core_FOUND
       AND Qt5Network_FOUND
      )
   OR (Qt6Widgets_FOUND
       AND Qt6Core_FOUND
       AND Qt6Network_FOUND
      )
)
  add_executable(gksqt WIN32 MACOSX_BUNDLE lib/gks/qt/gksqt.cxx lib/gks/qt/gksserver.cxx lib/gks/qt/gkswidget.cxx)
  target_link_libraries(gksqt PUBLIC gks_static)
  if(Qt6Widgets_FOUND
     AND Qt6Core_FOUND
     AND Qt6Network_FOUND
  )
    target_link_libraries(gksqt PUBLIC Qt6::Widgets Qt6::Core Qt6::Network)
    set(gksqt_INSTALL_RPATH "${INSTALL_RPATH};${Qt6_LIBRARY_DIR}")
  elseif(
    Qt5Widgets_FOUND
    AND Qt5Core_FOUND
    AND Qt5Network_FOUND
  )
    target_link_libraries(gksqt PUBLIC Qt5::Widgets Qt5::Core Qt5::Network)
    set(gksqt_INSTALL_RPATH "${INSTALL_RPATH};${Qt5_LIBRARY_DIR}")
  elseif(Qt4_FOUND)
    target_link_libraries(gksqt PUBLIC Qt4::QtCore Qt4::QtGui Qt4::QtNetwork)
    set(gksqt_INSTALL_RPATH "${INSTALL_RPATH};${Qt4_LIBRARY_DIR}")
  endif()
  set_target_properties(
    gksqt PROPERTIES CXX_STANDARD 11 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON INSTALL_RPATH "${gksqt_INSTALL_RPATH}"
  )
  if(MINGW AND CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
    target_compile_options(gksqt PRIVATE -fno-exceptions)
  endif()
  target_compile_options(gksqt PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  if(GR_MANUAL_MOC_AND_RCC)
    set_target_properties(gksqt PROPERTIES AUTOMOC OFF AUTORCC OFF)
    if(Qt6Widgets_FOUND
       AND Qt6Core_FOUND
       AND Qt6Network_FOUND
    )
      set(MOC_INCLUDE_FLAGS ${QT6_MOC_INCLUDE_FLAGS})
    elseif(
      Qt5Widgets_FOUND
      AND Qt5Core_FOUND
      AND Qt5Network_FOUND
    )
      set(MOC_INCLUDE_FLAGS ${QT5_MOC_INCLUDE_FLAGS})
    else()
      set(MOC_INCLUDE_FLAGS ${QT4_MOC_INCLUDE_FLAGS})
    endif()
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_gkswidget.cxx
      COMMAND ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
              ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gkswidget.h -o ${CMAKE_CURRENT_BINARY_DIR}/moc_gkswidget.cxx
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gkswidget.h
    )
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_gksserver.cxx
      COMMAND ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
              ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksserver.h -o ${CMAKE_CURRENT_BINARY_DIR}/moc_gksserver.cxx
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksserver.h
    )
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qrc_gksqt.cxx
      COMMAND ${QT_RCC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksqt.qrc -o
              ${CMAKE_CURRENT_BINARY_DIR}/qrc_gksqt.cxx
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksqt.qrc
    )
    target_sources(
      gksqt PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/moc_gksserver.cxx ${CMAKE_CURRENT_BINARY_DIR}/moc_gkswidget.cxx
                    ${CMAKE_CURRENT_BINARY_DIR}/qrc_gksqt.cxx
    )
  else()
    set_target_properties(gksqt PROPERTIES AUTOMOC ON AUTORCC ON)
    target_sources(
      gksqt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksserver.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gkswidget.h ${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/qt/gksqt.qrc
    )
  endif()
  if(APPLE)
    target_sources(gksqt PRIVATE lib/gks/qt/gksqt.icns)
    set_source_files_properties(lib/gks/qt/gksqt.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources/)
    set_target_properties(
      gksqt
      PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "gksqt"
                 MACOSX_BUNDLE_GUI_IDENTIFIER "de.fz-juelich.gksqt"
                 MACOSX_BUNDLE_ICON_FILE "gksqt"
                 MACOSX_BUNDLE_BUNDLE_VERSION "${GR_VERSION}"
    )
    set(gksqt_MACOSX_BUNDLE_SIGNATURE "gksqt")
    set(gksqt_MACOSX_MAIN_NIB_FILE "")
  endif()
  string(APPEND GR_REPORT "- gksqt: Yes\n")
else()
  string(APPEND GR_REPORT "- gksqt: No (Qt4 / Qt5 / Qt6 not found)\n")
endif()

if((Qt6Widgets_FOUND
    AND Qt6Core_FOUND
    AND Qt6Gui_FOUND
   )
   OR (Qt5Widgets_FOUND
       AND Qt5Core_FOUND
       AND Qt5Gui_FOUND
      )
)
  if(NOT MINGW OR NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
    include(GenPreviewImages)

    add_executable(
      grplot WIN32 MACOSX_BUNDLE
      "${CMAKE_CURRENT_BINARY_DIR}/grplot.qrc"
      lib/grm/grplot/CollapsibleSection.cpp
      lib/grm/grplot/gredit/AddElementWidget.cpp
      lib/grm/grplot/gredit/BoundingLogic.cpp
      lib/grm/grplot/gredit/BoundingObject.cpp
      lib/grm/grplot/gredit/ColorPickerRGB.cpp
      lib/grm/grplot/gredit/CustomTreeWidgetItem.cpp
      lib/grm/grplot/gredit/EditElementWidget.cpp
      lib/grm/grplot/gredit/PreviewTextWidget.cpp
      lib/grm/grplot/gredit/TableWidget.cpp
      lib/grm/grplot/gredit/TreeWidget.cpp
      lib/grm/grplot/grplot.cxx
      lib/grm/grplot/grplotDockWidget.cpp
      lib/grm/grplot/grplotMainwindow.cxx
      lib/grm/grplot/grplotWidget.cxx
      lib/grm/grplot/qtterm/ArgsWrapper.cpp
      lib/grm/grplot/qtterm/Receiver.cpp
      lib/grm/grplot/util.cxx
    )
    if(Qt6Widgets_FOUND AND Qt6Core_FOUND)
      target_link_libraries(
        grplot
        PRIVATE GR::qt6gr
                Qt6::Widgets
                Qt6::Core
                Qt6::Gui
                grm_shared
                gr_shared
                gks_shared
      )
      set(grplot_INSTALL_RPATH "${INSTALL_RPATH};${Qt6_LIBRARY_DIR}")
    else()
      target_link_libraries(
        grplot
        PRIVATE GR::qt5gr
                Qt5::Widgets
                Qt5::Core
                Qt5::Gui
                grm_shared
                gr_shared
                gks_shared
      )
      set(grplot_INSTALL_RPATH "${INSTALL_RPATH};${Qt5_LIBRARY_DIR}")
    endif()
    target_compile_definitions(grplot PRIVATE GRDIR="${GR_DIRECTORY}")
    if(NOT TARGET XercesC::XercesC)
      target_compile_definitions(grplot PRIVATE NO_XERCES_C)
    endif()
    set_target_properties(
      grplot PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON INSTALL_RPATH
                                                                                    "${grplot_INSTALL_RPATH}"
    )
    if(WIN32)
      if(MINGW)
        target_link_options(grplot PRIVATE -mconsole)
      else()
        target_link_options(grplot PRIVATE /SUBSYSTEM:CONSOLE)
      endif()
    endif()
    if(GR_MANUAL_MOC_AND_RCC)
      set_target_properties(grplot PROPERTIES AUTOMOC OFF AUTORCC OFF)
      if(Qt6Widgets_FOUND AND Qt6Core_FOUND)
        set(MOC_INCLUDE_FLAGS ${QT6_MOC_INCLUDE_FLAGS})
      else()
        set(MOC_INCLUDE_FLAGS ${QT5_MOC_INCLUDE_FLAGS})
      endif()
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotWidget.cxx
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotWidget.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotWidget.cxx
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotWidget.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotMainwindow.cxx
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotMainwindow.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotMainwindow.cxx
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotMainwindow.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_ArgsWrapper.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/qtterm/ArgsWrapper.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_ArgsWrapper.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/qtterm/ArgsWrapper.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_Receiver.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/qtterm/Receiver.hxx -o ${CMAKE_CURRENT_BINARY_DIR}/moc_Receiver.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/qtterm/Receiver.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_BoundingLogic.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/BoundingLogic.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_BoundingLogic.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/BoundingLogic.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_BoundingObject.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/BoundingObject.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_BoundingObject.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/BoundingObject.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_CustomTreeWidgetItem.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/CustomTreeWidgetItem.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_CustomTreeWidgetItem.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/CustomTreeWidgetItem.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_TreeWidget.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/TreeWidget.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_TreeWidget.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/TreeWidget.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_AddElementWidget.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/AddElementWidget.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_AddElementWidget.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/AddElementWidget.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_EditElementWidget.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/EditElementWidget.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_EditElementWidget.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/EditElementWidget.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_TableWidget.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/TableWidget.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_TableWidget.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/TableWidget.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_CollapsibleSection.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/CollapsibleSection.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_CollapsibleSection.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/CollapsibleSection.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_ColorPickerRGB.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/ColorPickerRGB.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_ColorPickerRGB.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/ColorPickerRGB.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_PreviewTextWidget.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/PreviewTextWidget.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_PreviewTextWidget.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/PreviewTextWidget.hxx
      )
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotDockWidget.cpp
        COMMAND
          ${QT_MOC_EXECUTABLE} -DGRDIR=\"$(GR_DIRECTORY)\" ${MOC_INCLUDE_FLAGS}
          ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotDockWidget.hxx -o
          ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotDockWidget.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotDockWidget.hxx
      )
      target_sources(
        grplot
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotDockWidget.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotMainwindow.cxx
                ${CMAKE_CURRENT_BINARY_DIR}/moc_grplotWidget.cxx
                ${CMAKE_CURRENT_BINARY_DIR}/moc_AddElementWidget.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_BoundingLogic.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_BoundingObject.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_CustomTreeWidgetItem.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_ColorPickerRGB.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_EditElementWidget.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_PreviewTextWidget.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_TableWidget.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_TreeWidget.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_ArgsWrapper.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_Receiver.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/moc_CollapsibleSection.cpp
      )
    else()
      set_target_properties(grplot PROPERTIES AUTOMOC ON AUTORCC ON)
      target_sources(
        grplot
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotDockWidget.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotMainwindow.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/grplotWidget.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/AddElementWidget.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/BoundingLogic.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/BoundingObject.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/CustomTreeWidgetItem.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/ColorPickerRGB.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/EditElementWidget.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/PreviewTextWidget.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/TableWidget.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/gredit/TreeWidget.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/qtterm/ArgsWrapper.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/qtterm/Receiver.hxx
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/grm/grplot/CollapsibleSection.hxx
      )
    endif()
    if(APPLE)
      set_target_properties(grplot PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "de.fz-juelich.grplot")
    endif()
    if(WIN32)
      target_compile_definitions(grplot PRIVATE GR_STATIC_LIB)
    endif()
    target_compile_options(grplot PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  else()
    add_executable(grplot lib/grm/grplot/grplot.cxx)
  endif()
  string(APPEND GR_REPORT "- grplot: Yes\n")
else()
  string(APPEND GR_REPORT "- grplot: No (Qt6 and Qt5 not found)\n")
endif()

string(APPEND GR_REPORT "\nGRM integrations:\n")
if(TARGET XercesC::XercesC)
  string(APPEND GR_REPORT "- Xerces-C++: Yes\n")
else()
  string(APPEND GR_REPORT "- Xerces-C++: No\n")
endif()

if(EMSCRIPTEN)
  add_custom_target(gr_js ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/gr.js)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gr.js
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/js/api.js ${CMAKE_CURRENT_SOURCE_DIR}/js/jsterm.js
            ${CMAKE_CURRENT_SOURCE_DIR}/js/module.js libGR
    COMMAND
      ${CMAKE_COMMAND} -E cat ${CMAKE_CURRENT_SOURCE_DIR}/js/module.js ${CMAKE_CURRENT_BINARY_DIR}/libGR.js
      ${CMAKE_CURRENT_SOURCE_DIR}/js/api.js ${CMAKE_CURRENT_SOURCE_DIR}/js/jsterm.js > ${CMAKE_CURRENT_BINARY_DIR}/gr.js
    VERBATIM
  )
  add_executable(
    libGR $<TARGET_OBJECTS:gks_static> $<TARGET_OBJECTS:gr3_static> $<TARGET_OBJECTS:gr_static>
          $<TARGET_OBJECTS:grm_static>
  )
  foreach(LIBRARY gks_static gr_static gr3_static grm_static)
    get_target_property(CURRENT_LINK_LIBRARIES ${LIBRARY} INTERFACE_LINK_LIBRARIES)
    list(APPEND LIBGR_JS_DEPENDENCIES ${CURRENT_LINK_LIBRARIES})
  endforeach()
  list(REMOVE_DUPLICATES LIBGR_JS_DEPENDENCIES)
  list(
    REMOVE_ITEM
    LIBGR_JS_DEPENDENCIES
    gks_static
    gr_static
    gr3_static
    grm_static
    GR::GKS
    GR::GR
    GR::GR3
    GR::GRM
  )
  target_link_libraries(libGR PRIVATE ${LIBGR_JS_DEPENDENCIES})
  target_link_options(libGR PRIVATE "SHELL:-s USE_ICU=1")
  target_link_options(libGR PRIVATE "SHELL:-s DYNCALLS=1")
  target_link_options(libGR PRIVATE "SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=0")
  target_link_options(libGR PRIVATE "SHELL:-s LEGACY_GL_EMULATION=1")
  target_link_options(libGR PRIVATE "SHELL:-s NO_EXIT_RUNTIME=0")
  target_link_options(libGR PRIVATE "SHELL:-s WASM=0")
  target_link_options(libGR PRIVATE "SHELL:-s TOTAL_MEMORY=33554432")
  target_link_options(libGR PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1")
  target_link_options(libGR PRIVATE "SHELL:-s RESERVED_FUNCTION_POINTERS=4")
  target_link_options(libGR PRIVATE "SHELL:-s ALLOW_TABLE_GROWTH=1")
  set(EXPORTED_RUNTIME_METHODS
      UTF8ToString
      addFunction
      ccall
      cwrap
      getValue
      lengthBytesUTF8
      removeFunction
      setValue
      stringToUTF8
  )
  list(JOIN EXPORTED_RUNTIME_METHODS "\', \'" EXPORTED_RUNTIME_METHODS)
  string(PREPEND EXPORTED_RUNTIME_METHODS "[\'")
  string(APPEND EXPORTED_RUNTIME_METHODS "\']")
  target_link_options(libGR PRIVATE "SHELL:-s \"EXPORTED_RUNTIME_METHODS=${EXPORTED_RUNTIME_METHODS}\"")
  set(EXPORTED_FUNCTIONS
      _free
      _gks_get_dash_list
      _gr_activatews
      _gr_adjustrange
      _gr_axes
      _gr_axes3d
      _gr_begingraphics
      _gr_beginprint
      _gr_beginprintext
      _gr_beginselection
      _gr_cellarray
      _gr_clearws
      _gr_closegks
      _gr_closeseg
      _gr_closews
      _gr_colorbar
      _gr_contour
      _gr_copysegws
      _gr_createseg
      _gr_deactivatews
      _gr_destroycontext
      _gr_drawarc
      _gr_drawarrow
      _gr_drawgraphics
      _gr_drawimage
      _gr_drawpath
      _gr_drawrect
      _gr_emergencyclosegks
      _gr_endgraphics
      _gr_endprint
      _gr_endselection
      _gr_fillarc
      _gr_fillarea
      _gr_fillrect
      _gr_grid
      _gr_gridit
      _gr_herrorbars
      _gr_hexbin
      _gr_hsvtorgb
      _gr_importgraphics
      _gr_inqbbox
      _gr_inqbordercolorind
      _gr_inqborderwidth
      _gr_inqclipxform
      _gr_inqcolor
      _gr_inqcolorfromrgb
      _gr_inqcolormap
      _gr_inqdspsize
      _gr_inqlinecolorind
      _gr_inqlinetype
      _gr_inqlinewidth
      _gr_inqmarkercolorind
      _gr_inqmarkertype
      _gr_inqregenflags
      _gr_inqscale
      _gr_inqspace
      _gr_inqtext
      _gr_inqtextext
      _gr_inqviewport
      _gr_inqwindow
      _gr_mathtex
      _gr_moveselection
      _gr_ndctowc
      _gr_opengks
      _gr_openws
      _gr_panzoom
      _gr_path
      _gr_polyline
      _gr_polyline3d
      _gr_polymarker
      _gr_precision
      _gr_readimage
      _gr_redrawsegws
      _gr_resizeselection
      _gr_restorestate
      _gr_savestate
      _gr_selectclipxform
      _gr_selectcontext
      _gr_selntran
      _gr_selntran
      _gr_setarrowstyle
      _gr_setbordercolorind
      _gr_setborderwidth
      _gr_setcharexpan
      _gr_setcharheight
      _gr_setcharspace
      _gr_setcharup
      _gr_setclip
      _gr_setcolormap
      _gr_setcolorrep
      _gr_setcoordxform
      _gr_setfillcolorind
      _gr_setfillintstyle
      _gr_setfillstyle
      _gr_setlinecolorind
      _gr_setlinetype
      _gr_setlinewidth
      _gr_setmarkercolorind
      _gr_setmarkersize
      _gr_setmarkertype
      _gr_setregenflags
      _gr_setscale
      _gr_setscientificformat
      _gr_setsegtran
      _gr_setshadow
      _gr_setspace
      _gr_settextalign
      _gr_settextcolorind
      _gr_settextencoding
      _gr_settextfontprec
      _gr_settextpath
      _gr_settransparency
      _gr_setviewport
      _gr_setwindow
      _gr_setwsviewport
      _gr_setwswindow
      _gr_shade
      _gr_shadelines
      _gr_shadepoints
      _gr_spline
      _gr_surface
      _gr_text
      _gr_textext
      _gr_tick
      _gr_titles3d
      _gr_updategks
      _gr_updatews
      _gr_uselinespec
      _gr_validaterange
      _gr_verrorbars
      _gr_wctondc
      _grm_args_delete
      _grm_args_new
      _grm_args_push
      _grm_dump
      _grm_dump_json
      _grm_dump_json_str
      _grm_get_box
      _grm_get_stdout
      _grm_get_tooltip
      _grm_input
      _grm_is3d
      _grm_load_from_str
      _grm_merge
      _grm_merge_named
      _grm_plot
      _grm_read
      _grm_register
      _grm_switch
      _grm_unregister
      _malloc
  )
  list(JOIN EXPORTED_FUNCTIONS "\', \'" EXPORTED_FUNCTIONS)
  string(PREPEND EXPORTED_FUNCTIONS "[\'")
  string(APPEND EXPORTED_FUNCTIONS "\']")
  target_link_options(libGR PRIVATE "SHELL:-s \"EXPORTED_FUNCTIONS=${EXPORTED_FUNCTIONS}\"")
  target_link_options(libGR PRIVATE --js-library "${CMAKE_CURRENT_SOURCE_DIR}/js/library.js")
  foreach(
    FONT
    gksfont.dat
    CMUSerif-Math.ttf
    DejaVuSans.ttf
    STIXTwoMath.ttf
    mathtex2_offsets.bin
  )
    add_custom_command(
      TARGET libGR
      PRE_LINK
      COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/gks/fonts/${FONT}"
              "${CMAKE_CURRENT_BINARY_DIR}/fonts/${FONT}"
      BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/fonts/${FONT}"
    )
    target_link_options(libGR PRIVATE "SHELL:--embed-file fonts/${FONT}")
  endforeach()
endif()

if(GR_BUILD_DEMOS)
  add_executable(gksdemo lib/gks/demo.c)
  target_link_libraries(gksdemo PUBLIC gks_static)
  target_compile_options(gksdemo PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(gksdemo PROPERTIES C_STANDARD 90 C_EXTENSIONS OFF C_STANDARD_REQUIRED ON)

  add_executable(grdemo lib/gr/demo.c)
  target_link_libraries(grdemo PUBLIC GR::GR)
  target_link_libraries(grdemo PUBLIC GR::GKS)
  target_compile_options(grdemo PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(grdemo PROPERTIES C_STANDARD 90 C_EXTENSIONS OFF C_STANDARD_REQUIRED ON)

  add_subdirectory(lib/grm/test/public_api/grm grm_test_public_api)
  add_subdirectory(lib/grm/test/internal_api/grm grm_test_internal_api)
endif()

if(GR_BUILD_GKSM)
  add_executable(gksm lib/gks/gksm.c)
  target_link_libraries(gksm PUBLIC gks_static)
  target_compile_options(gksm PRIVATE ${COMPILER_OPTION_ERROR_IMPLICIT})
  set_target_properties(gksm PROPERTIES C_STANDARD 90 C_EXTENSIONS OFF C_STANDARD_REQUIRED ON)
endif()

if(GR_INSTALL)
  install(FILES LICENSE.md DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/GR)
  install(
    FILES lib/grm/src/grm/dom_render/graphics_tree/schema.xsd
    DESTINATION ${CMAKE_INSTALL_DATADIR}/xml/GRM
    RENAME grm_graphics_tree_schema.xsd
  )
  install(
    FILES lib/grm/src/grm/dom_render/graphics_tree/private_schema.xsd
    DESTINATION ${CMAKE_INSTALL_DATADIR}/xml/GRM
    RENAME grm_graphics_tree_private_schema.xsd
  )
  include(CMakePackageConfigHelpers)
  configure_package_config_file(
    "cmake/Config.cmake.in" "GRConfig.cmake" INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/GR"
  )
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/GRConfigVersion.cmake
    VERSION ${GR_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GRConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/GRConfigVersion.cmake"
          DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/GR"
  )
  install(
    TARGETS gks_shared gr_shared gr3_shared grm_shared
    EXPORT GRTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  if(TARGET qt4gr)
    install(
      TARGETS qt4gr
      EXPORT GRTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  endif()
  if(TARGET qt5gr)
    install(
      TARGETS qt5gr
      EXPORT GRTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  endif()
  if(TARGET qt6gr)
    install(
      TARGETS qt6gr
      EXPORT GRTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  endif()
  if(TARGET grplotWidget)
    install(
      TARGETS grplotWidget
      EXPORT GRTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  endif()
  install(
    EXPORT GRTargets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/GR"
    NAMESPACE GR::
  )
  foreach(PACKAGE gr gr3 gks grm)
    configure_file("${PACKAGE}.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}.pc" @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig/")
  endforeach()
  install(
    TARGETS gks_static
            gr_static
            gr3_static
            grm_static
            aggplugin
            cairoplugin
            glplugin
            gsplugin
            gtkplugin
            pgfplugin
            svgplugin
            videoplugin
            wmfplugin
            x11plugin
            zmqplugin
            qtplugin
            qt5plugin
            qt6plugin
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  if(TARGET gr3platform)
    install(
      TARGETS gr3platform
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  endif()
  if(TARGET qt4gr)
    install(
      TARGETS qt4gr
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  endif()
  if(TARGET qt5gr)
    install(
      TARGETS qt5gr
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  endif()
  if(TARGET qt6gr)
    install(
      TARGETS qt6gr
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  endif()
  if(TARGET quartzplugin)
    install(
      TARGETS quartzplugin
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  endif()
  if(TARGET GKSTerm)
    if(XCODE_BUILD)
      install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/GKSTerm/GKSTerm.app
        DESTINATION Applications
        USE_SOURCE_PERMISSIONS
      )
    else()
      install(TARGETS GKSTerm BUNDLE DESTINATION Applications)
    endif()
  endif()
  if(TARGET gksqt)
    install(
      TARGETS gksqt
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      BUNDLE DESTINATION Applications
    )
  endif()
  if(TARGET grplotWidget)
    install(
      TARGETS grplotWidget
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      BUNDLE DESTINATION Applications
    )
  endif()
  if(TARGET grplot)
    install(
      TARGETS grplot
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      BUNDLE DESTINATION Applications
    )
    install(
      FILES lib/grm/grplot/README.md
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/grplot
      RENAME grplot.man.md
    )
  endif()
  install(FILES lib/gr/gr.h lib/gks/gks.h lib/gr3/gr3.h lib/grm/include/grm.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/)
  install(DIRECTORY lib/grm/include/grm DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/)
  if(TARGET qt4gr
     OR TARGET qt5gr
     OR TARGET qt6gr
  )
    install(FILES lib/gr/qtgr/grwidget.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/)
  endif()
  if(TARGET grplotWidget)
    install(FILES lib/grm/grplot/grplotWidget.hxx lib/grm/grplot/util.hxx
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/grplotWidget
    )
    file(GLOB GREDIT_HEADERS "lib/grm/grplot/gredit/*.h")
    install(FILES ${GREDIT_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/grplotWidget/gredit)
    file(GLOB QTTERM_HEADERS "lib/grm/grplot/qtterm/*.h")
    install(FILES ${QTTERM_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/grplotWidget/qtterm)
  endif()
  install(
    DIRECTORY lib/gks/fonts
    DESTINATION ${CMAKE_INSTALL_PREFIX}/
    USE_SOURCE_PERMISSIONS
  )
endif()

message(${GR_REPORT})
