#ifndef GRM_PLOT_INT_H_INCLUDED
#define GRM_PLOT_INT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include "args_int.h"
#include "datatype/uint_map_int.h"
#include "error_int.h"
#include "event_int.h"

#ifdef __cplusplus
}

#include <optional>

#include <grm/dom_render/context.hxx>
#include <grm/dom_render/graphics_tree/Element.hxx>

#include <grm/plot.h>
extern "C" {
#else
#include <grm/plot.h>
#endif


/* ######################### internal interface ##################################################################### */

/* ========================= global variables ======================================================================= */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ args ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

extern grm_args_t *active_plot_args;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ event handling ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

extern EventQueue *event_queue;


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot clear ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

extern const char *plot_clear_exclude_keys[];


/* ========================= macros ================================================================================= */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define ROOT_DEFAULT_APPEND_PLOTS 0
#define PLOT_DEFAULT_WIDTH 600.0
#define PLOT_DEFAULT_HEIGHT 450.0
#define PLOT_DEFAULT_KIND "line"
#define PLOT_DEFAULT_XGRID 1
#define PLOT_DEFAULT_YGRID 1
#define PLOT_DEFAULT_ZGRID 1
#define PLOT_DEFAULT_COLORBAR_CHAR_HEIGHT 0.016
#define PLOT_DEFAULT_COLORBAR_OFFSET 0.02
#define PLOT_3D_COLORBAR_OFFSET 0.05
#define PLOT_POLAR_COLORBAR_OFFSET 0.025
#define PLOT_DEFAULT_COLORBAR_LOCATION "right"
#define PLOT_DEFAULT_COLORBAR_WIDTH 0.03

/* ========================= datatypes ============================================================================== */

/* ------------------------- dump ----------------------------------------------------------------------------------- */

typedef enum
{
  DUMP_AUTO_DETECT = 0,
  DUMP_JSON_PLAIN = 1,
  DUMP_JSON_ESCAPE_DOUBLE_MINUS = 2,
  DUMP_JSON_BASE64 = 3,
  DUMP_BSON_BASE64 = 4,
} DumpEncoding;


/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ kind to func ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

typedef grm_error_t (*PlotFunc)(grm_args_t *args);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ========================= functions ============================================================================== */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ general ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

grm_error_t plotInitStaticVariables(void);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plot arguments ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

grm_error_t plotMergeArgs(grm_args_t *args, const grm_args_t *merge_args, const char **hierarchy_name_ptr,
                          UintMap *hierarchy_to_id, int hold_always);
grm_error_t plotInitArgStructure(grm_arg_t *arg, const char **hierarchy_name_ptr,
                                 unsigned int next_hierarchy_level_max_id);
grm_error_t plotInitArgsStructure(grm_args_t *args, const char **hierarchy_name_ptr,
                                  unsigned int next_hierarchy_level_max_id);
int plotCheckForRequest(const grm_args_t *args, grm_error_t *error);
void plotSetFlagDefaults(void);
void plotSetAttributeDefaults(grm_args_t *subplot_args);
void plotPrePlot(grm_args_t *plot_args);
void plotSetTextEncoding(void);
grm_error_t plotPreSubplot(grm_args_t *subplot_args);
int plotProcessSubplotArgs(grm_args_t *subplot_args);
void plotProcessColormap(grm_args_t *subplot_args);
void plotProcessFont(grm_args_t *subplot_args);
grm_error_t plotProcessGridArguments(const grm_args_t *args);
void plotProcessResampleMethod(grm_args_t *subplot_args);
void plotProcessWindow(grm_args_t *subplot_args);
grm_error_t plotStoreCoordinateRanges(grm_args_t *subplot_args);
void plotPostPlot(grm_args_t *plot_args);
void plotPostSubplot(grm_args_t *subplot_args);
grm_error_t plotGetArgsInHierarchy(grm_args_t *args, const char **hierarchy_name_start_ptr, const char *key,
                                   UintMap *hierarchy_to_id, const grm_args_t **found_args,
                                   const char ***found_hierarchy_ptr);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ plotting ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

grm_error_t plotLine(grm_args_t *subplot_args);
grm_error_t plotStairs(grm_args_t *subplot_args);
grm_error_t plotScatter(grm_args_t *subplot_args);
grm_error_t plotQuiver(grm_args_t *subplot_args);
grm_error_t plotStem(grm_args_t *subplot_args);
grm_error_t plotHistogram(grm_args_t *subplot_args);
grm_error_t plotBarplot(grm_args_t *subplot_args);
grm_error_t plotContour(grm_args_t *subplot_args);
grm_error_t plotContourf(grm_args_t *subplot_args);
grm_error_t plotHexbin(grm_args_t *subplot_args);
grm_error_t plotHeatmap(grm_args_t *subplot_args);
grm_error_t plotPolarHeatmap(grm_args_t *subplot_args);
grm_error_t plotMarginalHeatmap(grm_args_t *subplot_args);
grm_error_t plotWireframe(grm_args_t *subplot_args);
grm_error_t plotSurface(grm_args_t *subplot_args);
grm_error_t plotLine3(grm_args_t *subplot_args);
grm_error_t plotScatter3(grm_args_t *subplot_args);
grm_error_t plotImshow(grm_args_t *subplot_args);
grm_error_t plotIsosurface(grm_args_t *subplot_args);
grm_error_t plotVolume(grm_args_t *subplot_args);
grm_error_t plotPolarLine(grm_args_t *subplot_args);
grm_error_t plotPolarHistogram(grm_args_t *subplot_args);
grm_error_t plotPolarScatter(grm_args_t *subplot_args);
grm_error_t plotPie(grm_args_t *subplot_args);
grm_error_t plotTrisurface(grm_args_t *subplot_args);
grm_error_t plotTricontour(grm_args_t *subplot_args);
grm_error_t plotShade(grm_args_t *subplot_args);
grm_error_t plotRaw(grm_args_t *subplot_args);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ auxiliary drawing functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

grm_error_t plotDrawAxes(grm_args_t *args, unsigned int pass);
grm_error_t plotDrawPolarAxes(grm_args_t *args);
grm_error_t plotDrawLegend(grm_args_t *args);
grm_error_t plotDrawPieLegend(grm_args_t *args);
grm_error_t plotDrawColorbar(grm_args_t *args, double off, unsigned int colors);
grm_error_t plotDrawErrorBars(grm_args_t *series_args, unsigned int x_length);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~ util ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const char *getCompatibleFormat(const char *key, const char *given_format);
int getIdFromArgs(const grm_args_t *args, int *plot_id, int *subplot_id, int *series_id);
grm_args_t *getSubplotFromNdcPoint(double x, double y);
grm_args_t *getSubplotFromNdcPoints(unsigned int n, const double *x, const double *y);
grm_error_t countsPolarHistogram(grm_args_t *subplot_args);

int getFreeIdFromFigureElements();

#ifdef __cplusplus
}
#endif


/* ------------------------- dump ----------------------------------------------------------------------------------- */

#ifdef __cplusplus
void dumpContext(FILE *f, DumpEncoding dump_encoding,
                 const std::unordered_set<std::string> *context_keys_to_discard = nullptr);
char *dumpContextStr(DumpEncoding dump_encoding,
                     const std::unordered_set<std::string> *context_keys_to_discard = nullptr);

void dumpContextAsXmlComment(FILE *f, const std::unordered_set<std::string> *context_keys_to_discard = nullptr);
char *dumpContextAsXmlCommentStr(const std::unordered_set<std::string> *context_keys_to_discard = nullptr);
#endif

/* ------------------------- load ----------------------------------------------------------------------------------- */

#ifdef __cplusplus
void loadContextStr(GRM::Context &context, const std::string &context_str, DumpEncoding dump_encoding);
#endif


/* ------------------------- xml ------------------------------------------------------------------------------------ */

#ifdef __cplusplus
#ifndef NO_XERCES_C
grm_error_t validateGraphicsTree(bool include_private_attributes = false);
#endif
bool validateGraphicsTreeWithErrorMessages();
#endif


#if defined(__cplusplus) && !defined(NO_XERCES_C)
std::string getMergedSchemaFilepath();
#endif

#endif /* ifndef GRM_PLOT_INT_H_INCLUDED */
