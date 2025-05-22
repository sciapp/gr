function(zero_pad width number padded_number)
  string(LENGTH "${number}" number_length)
  math(EXPR pad_length "${width} - ${number_length}")
  set(pad "")
  if(pad_length GREATER 0)
    string(REPEAT "0" ${pad_length} pad)
  endif()
  set(${padded_number}
      "${pad}${number}"
      PARENT_SCOPE
  )
endfunction()

set(GR_COLORMAPS
    uniform
    temperature
    grayscale
    glowing
    rainbowlike
    geologic
    greenscale
    cyanscale
    bluescale
    magentascale
    redscale
    flame
    brownscale
    pilatus
    autumn
    bone
    cool
    copper
    gray
    hot
    hsv
    jet
    pink
    spectral
    spring
    summer
    winter
    gist_earth
    gist_heat
    gist_ncar
    gist_rainbow
    gist_stern
    afmhot
    brg
    bwr
    coolwarm
    cmrmap
    cubehelix
    gnuplot
    gnuplot2
    ocean
    rainbow
    seismic
    terrain
    viridis
    inferno
    plasma
    magma
)
set(GR_FONTS
    times_roman
    times_italic
    times_bold
    times_bolditalic
    helvetica
    helvetica_oblique
    helvetica_bold
    helvetica_boldoblique
    courier
    courier_oblique
    courier_bold
    courier_boldoblique
    symbol
    bookman_light
    bookman_lightitalic
    bookman_demi
    bookman_demiitalic
    newcenturyschlbk_roman
    newcenturyschlbk_italic
    newcenturyschlbk_bold
    newcenturyschlbk_bolditalic
    avantgarde_book
    avantgarde_bookoblique
    avantgarde_demi
    avantgarde_demioblique
    palantino_roman
    palantino_italic
    palantino_bold
    palantino_bolditalic
    zapfchancery_mediumitalic
    zapfdingbats
    computermodern
    dejavusans
    stix_two_math
)
set(GR_FONT_INDICES
    101
    102
    103
    104
    105
    106
    107
    108
    109
    110
    111
    112
    113
    114
    115
    116
    117
    118
    119
    120
    121
    122
    123
    124
    125
    126
    127
    128
    129
    130
    131
    232
    233
    234
)
set(GR_FONT_PRECISIONS string char stroke outline)
set(GR_LINE_TYPES
    solid
    dashed
    dotted
    dashed_dotted
    dash_2_dot
    dash_3_dot
    long_dash
    long_short_dash
    spaced_dash
    spaced_dot
    double_dot
    triple_dot
)
set(GR_LINE_TYPE_INDICES
    1
    2
    3
    4
    -1
    -2
    -3
    -4
    -5
    -6
    -7
    -8
)
set(GR_MARKER_TYPES
    dot
    plus
    asterisk
    circle
    diagonal_cross
    solid_circle
    triangle_up
    solid_tri_up
    triangle_down
    solid_tri_down
    square
    solid_square
    bowtie
    solid_bowtie
    hglass
    solid_hglass
    diamond
    solid_diamond
    star
    solid_star
    tri_up_down
    solid_tri_right
    solid_tri_left
    hollow_plus
    solid_plus
    pentagon
    hexagon
    heptagon
    octagon
    star_4
    star_5
    star_6
    star_7
    star_8
    vline
    hline
    omark
)
set(GR_MARKER_TYPE_INDICES
    1
    2
    3
    4
    5
    -1
    -2
    -3
    -4
    -5
    -6
    -7
    -8
    -9
    -10
    -11
    -12
    -13
    -14
    -15
    -16
    -17
    -18
    -19
    -20
    -21
    -22
    -23
    -24
    -25
    -26
    -27
    -28
    -29
    -30
    -31
    -32
)
set(GRPLOT_QRC "<RCC>\n    <qresource prefix=\"/\">\n")
foreach(colormap IN LISTS GR_COLORMAPS)
  string(APPEND GRPLOT_QRC "        <file>preview_images/colormaps/${colormap}.png</file>\n")
endforeach()
foreach(font IN LISTS GR_FONTS)
  string(APPEND GRPLOT_QRC "        <file>preview_images/fonts/${font}.png</file>\n")
endforeach()
foreach(font_precision IN LISTS GR_FONT_PRECISIONS)
  string(APPEND GRPLOT_QRC "        <file>preview_images/font_precisions/${font_precision}.png</file>\n")
endforeach()
foreach(line_type IN LISTS GR_LINE_TYPES)
  string(APPEND GRPLOT_QRC "        <file>preview_images/line_types/${line_type}.png</file>\n")
endforeach()
foreach(marker_type IN LISTS GR_MARKER_TYPES)
  string(APPEND GRPLOT_QRC "        <file>preview_images/marker_types/${marker_type}.png</file>\n")
endforeach()
foreach(hatch_index RANGE 1 11)
  zero_pad(2 ${hatch_index} hatch_index_str)
  string(APPEND GRPLOT_QRC "        <file>preview_images/fill_styles/hatch${hatch_index_str}.png</file>\n")
endforeach()
foreach(pattern_index RANGE 1 108)
  zero_pad(3 ${pattern_index} pattern_index_str)
  string(APPEND GRPLOT_QRC "        <file>preview_images/fill_styles/pattern${pattern_index_str}.png</file>\n")
endforeach()
string(APPEND GRPLOT_QRC "   </qresource>\n</RCC>")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/grplot.qrc" "${GRPLOT_QRC}")

set(GEN_PREVIEWS_SRC
    "\
#ifdef __unix__\n\
#define _XOPEN_SOURCE 500\n\
#define _POSIX_C_SOURCE 200112L\n\
#endif\n\
\n\
#ifdef _WIN32\n\
#include <direct.h>\n\
#define mkdir(dir, mode) (_mkdir)(dir)\n\
#define PATH_SEP '\\\\'\n\
#else\n\
#include <sys/stat.h>\n\
#include <unistd.h>\n\
#define mkdir(dir, mode) (mkdir)(dir, mode)\n\
#define PATH_SEP '/'\n\
#endif\n\
\n\
#include <gks.h>\n\
#include <gr.h>\n\
#include <stdio.h>\n\
#include <stdlib.h>\n\
\n\
#define MAX_FILENAME_LENGTH 80\n\
#define COLORMAP_WIDTH 2000\n\
#define COLORMAP_HEIGHT 200\n\
#define FONT_WIDTH 1800\n\
#define FONT_HEIGHT 200\n\
#define FONT_PRECISION_WIDTH 1350\n\
#define FONT_PRECISION_HEIGHT 200\n\
#define LINE_TYPE_WIDTH 1000\n\
#define LINE_TYPE_HEIGHT 100\n\
#define MARKER_TYPE_WIDTH 200\n\
#define MARKER_TYPE_HEIGHT 200\n\
#define MARKER_SIZE 50\n\
#define MARKER_BORDER_COLORMAP 32\n\
#define MARKER_BORDER_COLOR 1100\n\
#define FILL_STYLE_WIDTH 16\n\
#define FILL_STYLE_HEIGHT 16\n\
#define FILL_STYLE_MAX_PATTERN 108\n\
#define FILL_STYLE_MAX_HATCH 11\n\
\n\
typedef struct\n\
{\n\
  const char *name;\n\
  int id;\n\
} entry_t;\n\
\n\
static const entry_t COLORMAPS[] = {\n\
"
)
set(colormap_index 0)
foreach(colormap IN LISTS GR_COLORMAPS)
  string(APPEND GEN_PREVIEWS_SRC "  {\"${colormap}\", ${colormap_index}},\n")
  math(EXPR colormap_index "${colormap_index} + 1")
endforeach()
string(
  APPEND
  GEN_PREVIEWS_SRC
  "\
  {NULL, 0},\n\
};\n\
\n\
static entry_t FONTS[] = {\n\
"
)
foreach(
  font_index
  font
  IN
  ZIP_LISTS
  GR_FONT_INDICES
  GR_FONTS
)
  string(APPEND GEN_PREVIEWS_SRC "  {\"${font}\", ${font_index}},\n")
endforeach()
string(
  APPEND
  GEN_PREVIEWS_SRC
  "\
  {NULL, 0},\n\
};\n\
\n\
static entry_t FONT_PRECISIONS[] = {\n\
"
)
set(font_precision_index 0)
foreach(font_precision IN LISTS GR_FONT_PRECISIONS)
  string(APPEND GEN_PREVIEWS_SRC "  {\"${font_precision}\", ${font_precision_index}},\n")
  math(EXPR font_precision_index "${font_precision_index} + 1")
endforeach()
string(
  APPEND
  GEN_PREVIEWS_SRC
  "\
  {NULL, 0},\n\
};\n\
\n\
static entry_t LINE_TYPES[] = {\n\
"
)
foreach(
  line_type_index
  line_type
  IN
  ZIP_LISTS
  GR_LINE_TYPE_INDICES
  GR_LINE_TYPES
)
  string(APPEND GEN_PREVIEWS_SRC "  {\"${line_type}\", ${line_type_index}},\n")
endforeach()
string(
  APPEND
  GEN_PREVIEWS_SRC
  "\
  {NULL, 0},\n\
};\n\
\n\
static entry_t MARKER_TYPES[] = {\n\
"
)
foreach(
  marker_type_index
  marker_type
  IN
  ZIP_LISTS
  GR_MARKER_TYPE_INDICES
  GR_MARKER_TYPES
)
  string(APPEND GEN_PREVIEWS_SRC "  {\"${marker_type}\", ${marker_type_index}},\n")
endforeach()
string(
  APPEND
  GEN_PREVIEWS_SRC
  "\
  {NULL, 0},\n\
};\n\
\n\
static void create_colormaps(void)\n\
{\n\
  const entry_t *colormap = COLORMAPS;\n\
  int color_indices[256];\n\
  int i;\n\
  char out_pathname[MAX_FILENAME_LENGTH];\n\
\n\
  for (i = 0; i < 256; ++i)\n\
    {\n\
      color_indices[i] = 1000 + i;\n\
    }\n\
\n\
  snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%ccolormaps\", PATH_SEP);\n\
  mkdir(out_pathname, 0755);\n\
  gr_setwindow(0.0, 1.0, 0.0, 1.0);\n\
  gr_setviewport(0.0, 1.0, 0.0, 1.0 * COLORMAP_HEIGHT / COLORMAP_WIDTH);\n\
  while (colormap->name != NULL)\n\
    {\n\
      snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%ccolormaps%c%s.png\", PATH_SEP, PATH_SEP,\n\
               colormap->name);\n\
      gr_beginprint(out_pathname);\n\
      gr_setwswindow(0.0, 1.0, 0.0, 1.0 * COLORMAP_HEIGHT / COLORMAP_WIDTH);\n\
      gr_setwsviewport(0.0, COLORMAP_WIDTH / 600.0 * 0.0254, 0.0, COLORMAP_HEIGHT / 600.0 * 0.0254);\n\
      gr_setcolormap(colormap->id);\n\
      gr_cellarray(0.0, 1.0, 0.0, 1.0, 256, 1, 1, 1, 256, 1, color_indices);\n\
      gr_endprint();\n\
      ++colormap;\n\
    }\n\
}\n\
\n\
static void create_fonts(void)\n\
{\n\
  const entry_t *font = FONTS;\n\
  char out_pathname[MAX_FILENAME_LENGTH];\n\
\n\
  snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%cfonts\", PATH_SEP);\n\
  mkdir(out_pathname, 0755);\n\
  gr_setwindow(0.0, 1.0, 0.0, 1.0);\n\
  gr_setviewport(0.0, 1.0, 0.0, 1.0 * FONT_HEIGHT / FONT_WIDTH);\n\
  gr_settextalign(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_BOTTOM);\n\
  gr_setcharheight(0.6 * FONT_HEIGHT / FONT_WIDTH);\n\
  while (font->name != NULL)\n\
    {\n\
      int font_prec;\n\
      switch (font->id)\n\
        {\n\
        case 113:\n\
        case 131:\n\
          font_prec = GKS_K_TEXT_PRECISION_STRING;\n\
          break;\n\
        default:\n\
          font_prec = GKS_K_TEXT_PRECISION_OUTLINE;\n\
          break;\n\
        }\n\
      snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%cfonts%c%s.png\", PATH_SEP, PATH_SEP, font->name);\n\
      gr_beginprint(out_pathname);\n\
      gr_setwswindow(0.0, 1.0, 0.0, 1.0 * FONT_HEIGHT / FONT_WIDTH);\n\
      gr_setwsviewport(0.0, FONT_WIDTH / 600.0 * 0.0254, 0.0, FONT_HEIGHT / 600.0 * 0.0254);\n\
      gr_settextfontprec(font->id, font_prec);\n\
      gr_text(0.0, 0.1 * FONT_HEIGHT / FONT_WIDTH, \"AaBbCcDdEeFfGg\");\n\
      gr_endprint();\n\
      ++font;\n\
    }\n\
}\n\
\n\
static void create_font_precisions(void)\n\
{\n\
  const entry_t *precision = FONT_PRECISIONS;\n\
  const entry_t *font = FONTS;\n\
  char out_pathname[MAX_FILENAME_LENGTH];\n\
\n\
  snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%cfont_precisions\", PATH_SEP);\n\
  mkdir(out_pathname, 0755);\n\
  gr_setwindow(0.0, 1.0, 0.0, 1.0);\n\
  gr_setviewport(0.0, 1.0, 0.0, 1.0 * FONT_PRECISION_HEIGHT / FONT_PRECISION_WIDTH);\n\
  gr_settextalign(GKS_K_TEXT_HALIGN_LEFT, GKS_K_TEXT_VALIGN_BOTTOM);\n\
  gr_setcharheight(0.7 * FONT_PRECISION_HEIGHT / FONT_PRECISION_WIDTH);\n\
  while (precision->name != NULL)\n\
    {\n\
      snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%cfont_precisions%c%s.png\", PATH_SEP, PATH_SEP,\n\
               precision->name);\n\
      gr_beginprint(out_pathname);\n\
      gr_setwswindow(0.0, 1.0, 0.0, 1.0 * FONT_PRECISION_HEIGHT / FONT_PRECISION_WIDTH);\n\
      gr_setwsviewport(0.0, FONT_PRECISION_WIDTH / 600.0 * 0.0254, 0.0, FONT_PRECISION_HEIGHT / 600.0 * 0.0254);\n\
      gr_settextfontprec(font->id, precision->id);\n\
      gr_text(0.0, 0.0, (char *)font->name);\n\
      gr_endprint();\n\
      ++precision;\n\
    }\n\
}\n\
\n\
static void create_line_types(void)\n\
{\n\
  const entry_t *line_type = LINE_TYPES;\n\
  const double x[2] = {0.0, 1.0};\n\
  const double y[2] = {0.5, 0.5};\n\
  char out_pathname[MAX_FILENAME_LENGTH];\n\
\n\
  snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%cline_types\", PATH_SEP);\n\
  mkdir(out_pathname, 0755);\n\
  gr_setwindow(0.0, 1.0, 0.0, 1.0);\n\
  gr_setviewport(0.0, 1.0, 0.0, 1.0 * LINE_TYPE_HEIGHT / LINE_TYPE_WIDTH);\n\
  gr_setlinewidth(50);\n\
  while (line_type->name != NULL)\n\
    {\n\
      snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%cline_types%c%s.png\", PATH_SEP, PATH_SEP,\n\
               line_type->name);\n\
      gr_beginprint(out_pathname);\n\
      gr_setwswindow(0.0, 1.0, 0.0, 1.0 * LINE_TYPE_HEIGHT / LINE_TYPE_WIDTH);\n\
      gr_setwsviewport(0.0, LINE_TYPE_WIDTH / 600.0 * 0.0254, 0.0, LINE_TYPE_HEIGHT / 600.0 * 0.0254);\n\
      gr_setlinetype(line_type->id);\n\
      gr_polyline(2, (double *)x, (double *)y);\n\
      gr_endprint();\n\
      ++line_type;\n\
    }\n\
}\n\
\n\
static void create_marker_types(void)\n\
{\n\
  const entry_t *marker_type = MARKER_TYPES;\n\
  const double x = 0.5, y = 0.5;\n\
  char out_pathname[MAX_FILENAME_LENGTH];\n\
\n\
  snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%cmarker_types\", PATH_SEP);\n\
  mkdir(out_pathname, 0755);\n\
  gr_setwindow(0.0, 1.0, 0.0, 1.0);\n\
  gr_setviewport(0.0, 1.0, 0.0, 1.0 * MARKER_TYPE_HEIGHT / MARKER_TYPE_WIDTH);\n\
  gr_setmarkersize(MARKER_SIZE);\n\
  gr_setlinewidth(MARKER_SIZE);\n\
  gr_setborderwidth(0.4 * MARKER_SIZE);\n\
  gr_setcolormap(MARKER_BORDER_COLORMAP);\n\
  gr_setbordercolorind(MARKER_BORDER_COLOR);\n\
  while (marker_type->name != NULL)\n\
    {\n\
      snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%cmarker_types%c%s.png\", PATH_SEP, PATH_SEP,\n\
               marker_type->name);\n\
      gr_beginprint(out_pathname);\n\
      gr_setwswindow(0.0, 1.0, 0.0, 1.0 * MARKER_TYPE_HEIGHT / MARKER_TYPE_WIDTH);\n\
      gr_setwsviewport(0.0, MARKER_TYPE_WIDTH / 600.0 * 0.0254, 0.0, MARKER_TYPE_HEIGHT / 600.0 * 0.0254);\n\
      gr_setmarkertype(marker_type->id);\n\
      gr_polymarker(1, (double *)&x, (double *)&y);\n\
      gr_endprint();\n\
      ++marker_type;\n\
    }\n\
}\n\
\n\
static void create_fill_styles(void)\n\
{\n\
  const double x[4] = {0.0, 1.0, 1.0, 0.0};\n\
  const double y[4] = {0.0, 0.0, 1.0, 1.0};\n\
  char out_pathname[MAX_FILENAME_LENGTH];\n\
  int i;\n\
\n\
  snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%cfill_styles\", PATH_SEP);\n\
  mkdir(out_pathname, 0755);\n\
  gr_setwindow(0.0, 1.0, 0.0, 1.0);\n\
  gr_setviewport(0.0, 1.0, 0.0, 1.0 * MARKER_TYPE_HEIGHT / MARKER_TYPE_WIDTH);\n\
  gr_setfillintstyle(GKS_K_INTSTYLE_PATTERN);\n\
  for (i = 1; i <= FILL_STYLE_MAX_PATTERN; ++i)\n\
    {\n\
      snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%cfill_styles%cpattern%03d.png\", PATH_SEP, PATH_SEP,\n\
               i);\n\
      gr_beginprint(out_pathname);\n\
      gr_setwswindow(0.0, 1.0, 0.0, 1.0 * FILL_STYLE_HEIGHT / FILL_STYLE_WIDTH);\n\
      gr_setwsviewport(0.0, FILL_STYLE_WIDTH / 600.0 * 0.0254, 0.0, FILL_STYLE_HEIGHT / 600.0 * 0.0254);\n\
      gr_setfillintstyle(GKS_K_INTSTYLE_SOLID);\n\
      gr_setfillcolorind(0);\n\
      gr_fillarea(4, (double *)x, (double *)y);\n\
      gr_setfillintstyle(GKS_K_INTSTYLE_PATTERN);\n\
      gr_setfillcolorind(1);\n\
      gr_setfillstyle(i);\n\
      gr_fillarea(4, (double *)x, (double *)y);\n\
      gr_endprint();\n\
    }\n\
  gr_setfillintstyle(GKS_K_INTSTYLE_HATCH);\n\
  for (i = 1; i <= FILL_STYLE_MAX_HATCH; ++i)\n\
    {\n\
      snprintf(out_pathname, MAX_FILENAME_LENGTH, \"preview_images%cfill_styles%chatch%02d.png\", PATH_SEP, PATH_SEP, i);\n\
      gr_beginprint(out_pathname);\n\
      gr_setwswindow(0.0, 1.0, 0.0, 1.0 * FILL_STYLE_HEIGHT / FILL_STYLE_WIDTH);\n\
      gr_setwsviewport(0.0, FILL_STYLE_WIDTH / 600.0 * 0.0254, 0.0, FILL_STYLE_HEIGHT / 600.0 * 0.0254);\n\
      gr_setfillintstyle(GKS_K_INTSTYLE_SOLID);\n\
      gr_setfillcolorind(0);\n\
      gr_fillarea(4, (double *)x, (double *)y);\n\
      gr_setfillintstyle(GKS_K_INTSTYLE_HATCH);\n\
      gr_setfillcolorind(1);\n\
      gr_setfillstyle(i);\n\
      gr_fillarea(4, (double *)x, (double *)y);\n\
      gr_endprint();\n\
    }\n\
}\n\
\n\
int main(int argc, char **argv)\n\
{\n\
#ifdef _WIN32\n\
  putenv(\"GKS_WSTYPE=nul\");\n\
#else\n\
  setenv(\"GKS_WSTYPE\", \"nul\", 1);\n\
#endif\n\
\n\
  mkdir(\"preview_images\", 0755);\n\
\n\
  create_colormaps();\n\
  create_fonts();\n\
  create_font_precisions();\n\
  create_line_types();\n\
  create_marker_types();\n\
  create_fill_styles();\n\
\n\
  return 0;\n\
}\n\
"
)

if(NOT CMAKE_CROSSCOMPILING)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gen_preview_images.c" "${GEN_PREVIEWS_SRC}")
  add_executable(gen_preview_images EXCLUDE_FROM_ALL ${CMAKE_CURRENT_BINARY_DIR}/gen_preview_images.c)
  target_link_libraries(gen_preview_images PRIVATE GR::GKS GR::GR)
  set(GR_GEN_PREVIEW_IMAGES_EXECUTABLE $<TARGET_FILE:gen_preview_images>)
else()
  if(GR_HOST_DIRECTORY)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen_preview_images")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gen_preview_images/gen_preview_images.c" "${GEN_PREVIEWS_SRC}")
    file(
      WRITE "${CMAKE_CURRENT_BINARY_DIR}/gen_preview_images/CMakeLists.txt"
      "\
cmake_minimum_required(VERSION 3.1...4.0 FATAL_ERROR)\n\
\n\
project(\n\
gen_preview_images\n\
LANGUAGES C\n\
)\n\
\n\
set(GR_DIRECTORY\n\
\"/usr/local/gr\"\n\
CACHE STRING \"GR installation prefix\"\n\
)\n\
\n\
add_executable(gen_preview_images gen_preview_images.c)\n\
find_package(GR CONFIG REQUIRED PATHS \${GR_DIRECTORY} NO_DEFAULT_PATH)\n\
target_link_libraries(gen_preview_images PRIVATE GR::GKS GR::GR)\n\
set_target_properties(\n\
gen_preview_images\n\
PROPERTIES\n\
  C_STANDARD 90\n\
  C_STANDARD_REQUIRED ON\n\
  C_EXTENSIONS OFF\n\
)\n\
"
    )
    set(GR_GEN_PREVIEW_IMAGES_EXECUTABLE
        "${CMAKE_CURRENT_BINARY_DIR}/host/gen_preview_images${CMAKE_HOST_EXECUTABLE_SUFFIX}"
    )
    add_custom_target(gen_preview_images DEPENDS ${GR_GEN_PREVIEW_IMAGES_EXECUTABLE})
    add_custom_command(
      OUTPUT ${GR_GEN_PREVIEW_IMAGES_EXECUTABLE}
      COMMAND
        "${CMAKE_COMMAND}" -S "${CMAKE_CURRENT_BINARY_DIR}/gen_preview_images" -B
        "${CMAKE_CURRENT_BINARY_DIR}/gen_preview_images/build" -DCMAKE_BUILD_TYPE="Release"
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE="${CMAKE_CURRENT_BINARY_DIR}/host"
        -DGR_DIRECTORY="${GR_HOST_DIRECTORY}"
      COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}/gen_preview_images/build" --config Release
      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/gen_preview_images/gen_preview_images.c"
    )
  else()
    message(
      WARNING
        "GR_HOST_DIRECTORY is not set. GRPlot preview images will be taken from the source code tree and could differ \
from the version of GRPlot that you are building."
    )
  endif()
endif()

set(GR_COLORMAP_IMAGES "${GR_COLORMAPS}")
list(TRANSFORM GR_COLORMAP_IMAGES PREPEND "preview_images/colormaps/")
list(TRANSFORM GR_COLORMAP_IMAGES APPEND ".png")
set(GR_FONT_IMAGES "${GR_FONTS}")
list(TRANSFORM GR_FONT_IMAGES PREPEND "preview_images/fonts/")
list(TRANSFORM GR_FONT_IMAGES APPEND ".png")
set(GR_FONT_PRECISION_IMAGES "${GR_FONT_PRECISIONS}")
list(TRANSFORM GR_FONT_PRECISION_IMAGES PREPEND "preview_images/font_precisions/")
list(TRANSFORM GR_FONT_PRECISION_IMAGES APPEND ".png")
set(GR_LINE_TYPE_IMAGES "${GR_LINE_TYPES}")
list(TRANSFORM GR_LINE_TYPE_IMAGES PREPEND "preview_images/line_types/")
list(TRANSFORM GR_LINE_TYPE_IMAGES APPEND ".png")
set(GR_MARKER_TYPE_IMAGES "${GR_MARKER_TYPES}")
list(TRANSFORM GR_MARKER_TYPE_IMAGES PREPEND "preview_images/marker_types/")
list(TRANSFORM GR_MARKER_TYPE_IMAGES APPEND ".png")
set(GR_FILL_STYLE_IMAGES "")
foreach(hatch_index RANGE 1 11)
  zero_pad(2 ${hatch_index} hatch_index_str)
  list(APPEND GR_FILL_STYLE_IMAGES "hatch${hatch_index_str}")
endforeach()
foreach(pattern_index RANGE 1 108)
  zero_pad(3 ${pattern_index} pattern_index_str)
  list(APPEND GR_FILL_STYLE_IMAGES "pattern${pattern_index_str}")
endforeach()
list(TRANSFORM GR_FILL_STYLE_IMAGES PREPEND "preview_images/fill_styles/")
list(TRANSFORM GR_FILL_STYLE_IMAGES APPEND ".png")

add_custom_target(
  grplot_preview_images
  DEPENDS ${GR_COLORMAP_IMAGES}
          ${GR_FONT_IMAGES}
          ${GR_FONT_PRECISION_IMAGES}
          ${GR_LINE_TYPE_IMAGES}
          ${GR_MARKER_TYPE_IMAGES}
          ${GR_FILL_STYLE_IMAGES}
)
if(NOT CMAKE_CROSSCOMPILING OR GR_HOST_DIRECTORY)
  add_custom_command(
    OUTPUT ${GR_COLORMAP_IMAGES}
           ${GR_FONT_IMAGES}
           ${GR_FONT_PRECISION_IMAGES}
           ${GR_LINE_TYPE_IMAGES}
           ${GR_MARKER_TYPE_IMAGES}
           ${GR_FILL_STYLE_IMAGES}
    COMMAND "${CMAKE_COMMAND}" -E env GKS_FONTPATH="${CMAKE_CURRENT_LIST_DIR}/../lib/gks"
            ${GR_GEN_PREVIEW_IMAGES_EXECUTABLE}
    DEPENDS ${GR_GEN_PREVIEW_IMAGES_EXECUTABLE}
  )
else()
  add_custom_command(
    OUTPUT ${GR_COLORMAP_IMAGES}
           ${GR_FONT_IMAGES}
           ${GR_FONT_PRECISION_IMAGES}
           ${GR_LINE_TYPE_IMAGES}
           ${GR_MARKER_TYPE_IMAGES}
           ${GR_FILL_STYLE_IMAGES}
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/../lib/grm/grplot/preview_images"
            "${CMAKE_CURRENT_BINARY_DIR}/preview_images"
  )
endif()
