#ifndef GR_CASTS_HXX
#define GR_CASTS_HXX


#include <grm/dom_render/graphics_tree/element.hxx>
#include <grm/dom_render/graphics_tree/document.hxx>
#include "gks.h"
#include "gr.h"


/* ========================= string to int maps ===================================================================== */

inline std::map<std::string, double> symbol_to_meters_per_unit{
    {"m", 1.0},     {"dm", 0.1},    {"cm", 0.01},  {"mm", 0.001},        {"in", 0.0254},
    {"\"", 0.0254}, {"ft", 0.3048}, {"'", 0.0254}, {"pc", 0.0254 / 6.0}, {"pt", 0.0254 / 72.0},
};

inline std::map<std::string, int> colormap_string_to_int{
    {"default", 0},       {"temperature", 1}, {"grayscale", 2},   {"glowing", 3},    {"rainbowlike", 4},
    {"geologic", 5},      {"greenscale", 6},  {"cyanscale", 7},   {"bluescale", 8},  {"magentascale", 9},
    {"redscale", 10},     {"flame", 11},      {"brownscale", 12}, {"pilatus", 13},   {"autumn", 14},
    {"bone", 15},         {"cool", 16},       {"copper", 17},     {"gray", 18},      {"hot", 19},
    {"hsv", 20},          {"jet", 21},        {"pink", 22},       {"spectral", 23},  {"spring", 24},
    {"summer", 25},       {"winter", 26},     {"gist_earth", 27}, {"gist_heat", 28}, {"gist_ncar", 29},
    {"gist_rainbow", 30}, {"gist_stern", 31}, {"afmhot", 32},     {"brg", 33},       {"bwr", 34},
    {"coolwarm", 35},     {"cmrmap", 36},     {"cubehelix", 37},  {"gnuplot", 38},   {"gnuplot2", 39},
    {"ocean", 40},        {"rainbow", 41},    {"seismic", 42},    {"terrain", 43},   {"viridis", 44},
    {"inferno", 45},      {"plasma", 46},     {"magma", 47},      {"uniform", 48},
};
inline std::map<std::string, int> font_string_to_int{
    {"times_roman", 101},
    {"times_italic", 102},
    {"times_bold", 103},
    {"times_bolditalic", 104},
    {"helvetica", 105},
    {"helvetica_oblique", 106},
    {"helvetica_bold", 107},
    {"helvetica_boldoblique", 108},
    {"courier", 109},
    {"courier_oblique", 110},
    {"courier_bold", 111},
    {"courier_boldoblique", 112},
    {"symbol", 113},
    {"bookman_light", 114},
    {"bookman_lightitalic", 115},
    {"bookman_demi", 116},
    {"bookman_demiitalic", 117},
    {"newcenturyschlbk_roman", 118},
    {"newcenturyschlbk_italic", 119},
    {"newcenturyschlbk_bold", 120},
    {"newcenturyschlbk_bolditalic", 121},
    {"avantgarde_book", 122},
    {"avantgarde_bookoblique", 123},
    {"avantgarde_demi", 124},
    {"avantgarde_demioblique", 125},
    {"palantino_roman", 126},
    {"palantino_italic", 127},
    {"palantino_bold", 128},
    {"palantino_bolditalic", 129},
    {"zapfchancery_mediumitalic", 130},
    {"zapfdingbats", 131},
    {"computermodern", 232},
    {"dejavusans", 233},
};
inline std::map<std::string, int> font_precision_string_to_int{
    {"string", 0},
    {"char", 1},
    {"stroke", 2},
    {"outline", 3},
};
inline std::map<std::string, int> line_type_string_to_int{
    {"solid", 1},        {"dashed", 2},      {"dotted", 3},      {"dashed_dotted", 4},
    {"dash_2_dot", -1},  {"dash_3_dot", -2}, {"long_dash", -3},  {"long_short_dash", -4},
    {"spaced_dash", -5}, {"spaced_dot", -6}, {"double_dot", -7}, {"triple_dot", -8},
};
inline std::map<std::string, int> location_string_to_int{
    {"top_right", 1},
    {"top_left", 2},
    {"bottom_left", 3},
    {"bottom_right", 4},
    {"mid_right", 5},
    {"mid_left", 6},
    {"mid_right", 7},
    {"mid_bottom", 8},
    {"mid_top", 9},
    {"central", 10},
    {"outside_window_top_right", 11},
    {"outside_window_mid_right", 12},
    {"outside_window_bottom_right", 13},
};
inline std::map<std::string, int> x_axis_location_string_to_int{
    {"x", 0},
    {"twin_x", 1},
    {"top", 2},
    {"bottom", 3},
};
inline std::map<std::string, int> y_axis_location_string_to_int{
    {"y", 0},
    {"twin_y", 1},
    {"right", 2},
    {"left", 3},
};
inline std::map<std::string, int> marker_type_string_to_int{
    {"dot", 1},
    {"plus", 2},
    {"asterisk", 3},
    {"circle", 4},
    {"diagonal_cross", 5},
    {"solid_circle", -1},
    {"triangle_up", -2},
    {"solid_tri_up", -3},
    {"triangle_down", -4},
    {"solid_tri_down", -5},
    {"square", -6},
    {"solid_square", -7},
    {"bowtie", -8},
    {"solid_bowtie", -9},
    {"hglass", -10},
    {"solid_hglass", -11},
    {"diamond", -12},
    {"solid_diamond", -13},
    {"star", -14},
    {"solid_star", -15},
    {"tri_up_down", -16},
    {"solid_tri_right", -17},
    {"solid_tri_left", -18},
    {"hollow_plus", -19},
    {"solid_plus", -20},
    {"pentagon", -21},
    {"hexagon", -22},
    {"heptagon", -23},
    {"octagon", -24},
    {"star_4", -25},
    {"star_5", -26},
    {"star_6", -27},
    {"star_7", -28},
    {"star_8", -29},
    {"vline", -30},
    {"hline", -31},
    {"omark", -32},
};
inline std::map<std::string, int> scientific_format_string_to_int{
    {"textex", 2},
    {"mathtex", 3},
};
inline std::map<std::string, int> error_bar_style_string_to_int{
    {"line", 0},
    {"area", 1},
};
inline std::map<std::string, int> text_align_horizontal_string_to_int{
    {"normal", 0},
    {"left", 1},
    {"center", 2},
    {"right", 3},
};
inline std::map<std::string, int> text_align_vertical_string_to_int{
    {"normal", 0}, {"top", 1}, {"cap", 2}, {"half", 3}, {"base", 4}, {"bottom", 5},
};
inline std::map<std::string, int> algorithm_string_to_int{
    {"emission", GR_VOLUME_EMISSION},
    {"absorption", GR_VOLUME_ABSORPTION},
    {"mip", GR_VOLUME_MIP},
    {"maximum", GR_VOLUME_MIP},
};
inline std::map<std::string, int> color_model_string_to_int{
    {"rgb", 0},
    {"hsv", 1},
};
inline std::map<std::string, int> clip_region_string_to_int{
    {"rectangular", 0},
    {"elliptic", 1},
};
inline std::map<std::string, int> resample_method_string_to_int{
    {"default", GKS_K_RESAMPLE_DEFAULT},
    {"nearest", GKS_K_RESAMPLE_NEAREST},
    {"linear", GKS_K_RESAMPLE_LINEAR},
    {"lanczos", GKS_K_RESAMPLE_LANCZOS},
};
inline std::map<std::string, int> fill_style_string_to_int{
    {"hatch01", 1},      {"hatch02", 2},      {"hatch03", 3},      {"hatch04", 4},      {"hatch05", 5},
    {"hatch06", 6},      {"hatch07", 7},      {"hatch08", 8},      {"hatch09", 9},      {"hatch10", 10},
    {"hatch11", 11},     {"pattern001", 1},   {"pattern002", 2},   {"pattern003", 3},   {"pattern004", 4},
    {"pattern005", 5},   {"pattern006", 6},   {"pattern007", 7},   {"pattern008", 8},   {"pattern009", 9},
    {"pattern010", 10},  {"pattern011", 11},  {"pattern012", 12},  {"pattern013", 13},  {"pattern014", 14},
    {"pattern015", 15},  {"pattern016", 16},  {"pattern017", 17},  {"pattern018", 18},  {"pattern019", 19},
    {"pattern020", 20},  {"pattern021", 21},  {"pattern022", 22},  {"pattern023", 23},  {"pattern024", 24},
    {"pattern025", 25},  {"pattern026", 26},  {"pattern027", 27},  {"pattern028", 28},  {"pattern029", 29},
    {"pattern030", 30},  {"pattern031", 31},  {"pattern032", 32},  {"pattern033", 33},  {"pattern034", 34},
    {"pattern035", 35},  {"pattern036", 36},  {"pattern037", 37},  {"pattern038", 38},  {"pattern039", 39},
    {"pattern040", 40},  {"pattern041", 41},  {"pattern042", 42},  {"pattern043", 43},  {"pattern044", 44},
    {"pattern045", 45},  {"pattern046", 46},  {"pattern047", 47},  {"pattern048", 48},  {"pattern049", 49},
    {"pattern050", 50},  {"pattern040", 40},  {"pattern041", 41},  {"pattern042", 42},  {"pattern043", 43},
    {"pattern044", 44},  {"pattern045", 45},  {"pattern046", 46},  {"pattern047", 47},  {"pattern048", 48},
    {"pattern049", 49},  {"pattern050", 50},  {"pattern040", 40},  {"pattern041", 41},  {"pattern042", 42},
    {"pattern043", 43},  {"pattern044", 44},  {"pattern045", 45},  {"pattern046", 46},  {"pattern047", 47},
    {"pattern048", 48},  {"pattern049", 49},  {"pattern050", 50},  {"pattern051", 51},  {"pattern052", 52},
    {"pattern053", 53},  {"pattern054", 54},  {"pattern055", 55},  {"pattern056", 56},  {"pattern057", 57},
    {"pattern058", 58},  {"pattern059", 59},  {"pattern060", 60},  {"pattern061", 61},  {"pattern062", 62},
    {"pattern063", 63},  {"pattern064", 64},  {"pattern065", 65},  {"pattern066", 66},  {"pattern067", 67},
    {"pattern068", 68},  {"pattern069", 69},  {"pattern070", 70},  {"pattern071", 71},  {"pattern072", 72},
    {"pattern073", 73},  {"pattern074", 74},  {"pattern075", 75},  {"pattern076", 76},  {"pattern077", 77},
    {"pattern078", 78},  {"pattern079", 79},  {"pattern080", 80},  {"pattern081", 81},  {"pattern082", 82},
    {"pattern083", 83},  {"pattern084", 84},  {"pattern085", 85},  {"pattern086", 86},  {"pattern087", 87},
    {"pattern088", 88},  {"pattern089", 89},  {"pattern090", 90},  {"pattern091", 91},  {"pattern092", 92},
    {"pattern093", 93},  {"pattern094", 94},  {"pattern095", 95},  {"pattern096", 96},  {"pattern097", 97},
    {"pattern098", 98},  {"pattern099", 99},  {"pattern100", 100}, {"pattern101", 101}, {"pattern102", 102},
    {"pattern103", 103}, {"pattern104", 104}, {"pattern105", 105}, {"pattern106", 106}, {"pattern107", 107},
    {"pattern108", 108},
};
inline std::map<std::string, int> transformation_string_to_int{
    {"boolean", 0},     {"linear", 1},
    {"logarithmic", 2}, {"double_logarithmic", 3},
    {"cubic", 4},       {"histogram_equalized", 5},
};

/* ========================= functions ============================================================================== */

namespace GRM
{

/* ------------------------------- string to int ---------------------------------------------------------------------*/

GRM_EXPORT int algorithmStringToInt(const std::string &algorithm_str);
GRM_EXPORT int colormapStringToInt(const std::string &colormap_str);
GRM_EXPORT int fontStringToInt(const std::string &font_str);
GRM_EXPORT int fontPrecisionStringToInt(const std::string &font_precision_str);
GRM_EXPORT int lineTypeStringToInt(const std::string &line_type_str);
GRM_EXPORT int locationStringToInt(const std::string &location_str);
GRM_EXPORT int xAxisLocationStringToInt(const std::string &location_str);
GRM_EXPORT int yAxisLocationStringToInt(const std::string &location_str);
GRM_EXPORT int markerTypeStringToInt(const std::string &marker_type_str);
GRM_EXPORT int projectionTypeStringToInt(const std::string &projection_type_str);
GRM_EXPORT int colorModelStringToInt(const std::string &color_model_str);
GRM_EXPORT int scientificFormatStringToInt(const std::string &scientific_format_str);
GRM_EXPORT int textAlignHorizontalStringToInt(const std::string &text_align_horizontal_str);
GRM_EXPORT int textAlignVerticalStringToInt(const std::string &text_align_vertical_str);
GRM_EXPORT int textEncodingStringToInt(const std::string &text_encoding_str);
GRM_EXPORT int tickOrientationStringToInt(const std::string &tick_orientation_str);
GRM_EXPORT int errorBarStyleStringToInt(const std::string &error_bar_stylr_str);
GRM_EXPORT int clipRegionStringToInt(const std::string &error_bar_stylr_str);
GRM_EXPORT int resampleMethodStringToInt(const std::string &error_bar_stylr_str);
GRM_EXPORT int fillStyleStringToInt(const std::string &fill_style_str);
GRM_EXPORT int fillIntStyleStringToInt(const std::string &fill_int_style_str);
GRM_EXPORT int transformationStringToInt(const std::string &transformation_str);
GRM_EXPORT int labelOrientationStringToInt(const std::string &label_orientation_str);
GRM_EXPORT int worldCoordinatesStringToInt(const std::string &world_coordinates_str);

/* ------------------------------- int to string ---------------------------------------------------------------------*/

GRM_EXPORT std::string algorithmIntToString(int algorithm);
GRM_EXPORT std::string colormapIntToString(int colormap);
GRM_EXPORT std::string fontIntToString(int font);
GRM_EXPORT std::string fontPrecisionIntToString(int font_precision);
GRM_EXPORT std::string lineTypeIntToString(int line_type);
GRM_EXPORT std::string locationIntToString(int location);
GRM_EXPORT std::string xAxisLocationIntToString(int location);
GRM_EXPORT std::string yAxisLocationIntToString(int location);
GRM_EXPORT std::string markerTypeIntToString(int marker_type);
GRM_EXPORT std::string projectionTypeIntToString(int projection_type);
GRM_EXPORT std::string colorModelIntToString(int color_model);
GRM_EXPORT std::string scientificFormatIntToString(int scientific_format);
GRM_EXPORT std::string textAlignHorizontalIntToString(int text_align_horizontal);
GRM_EXPORT std::string textAlignVerticalIntToString(int text_align_vertical);
GRM_EXPORT std::string textEncodingIntToString(int text_encoding);
GRM_EXPORT std::string tickOrientationIntToString(int tick_orientation);
GRM_EXPORT std::string errorBarStyleIntToString(int error_bar_style);
GRM_EXPORT std::string clipRegionIntToString(int error_bar_style);
GRM_EXPORT std::string resampleMethodIntToString(int error_bar_style);
GRM_EXPORT std::string fillStyleIntToString(int fill_style);
GRM_EXPORT std::string fillIntStyleIntToString(int fill_int_style);
GRM_EXPORT std::string transformationIntToString(int transformation);
GRM_EXPORT std::string labelOrientationIntToString(int label_orientation);
GRM_EXPORT std::string worldCoordinatesIntToString(int world_coordinates);

/* ------------------------------- get functions ---------------------------------------------------------------------*/

GRM_EXPORT std::vector<std::string> getSizeUnits();
GRM_EXPORT std::vector<std::string> getColormaps();
GRM_EXPORT std::vector<std::string> getFonts();
GRM_EXPORT std::vector<std::string> getFontPrecisions();
GRM_EXPORT std::vector<std::string> getLineTypes();
GRM_EXPORT std::vector<std::string> getLocations();
GRM_EXPORT std::vector<std::string> getXAxisLocations();
GRM_EXPORT std::vector<std::string> getYAxisLocations();
GRM_EXPORT std::vector<std::string> getMarkerTypes();
GRM_EXPORT std::vector<std::string> getTextAlignHorizontal();
GRM_EXPORT std::vector<std::string> getTextAlignVertical();
GRM_EXPORT std::vector<std::string> getAlgorithm();
GRM_EXPORT std::vector<std::string> getColorModel();
GRM_EXPORT std::vector<std::string> getContextAttributes();
GRM_EXPORT std::vector<std::string> getFillStyles();
GRM_EXPORT std::vector<std::string> getFillIntStyles();
GRM_EXPORT std::vector<std::string> getTransformation();
} // namespace GRM
#endif // GR_CASTS_HXX
