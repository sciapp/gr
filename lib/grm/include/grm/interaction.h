#ifndef GRM_INTERACTION_H_INCLUDED
#define GRM_INTERACTION_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include "args.h"
#include "util.h"


/* ######################### public interface ####################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- user interaction ----------------------------------------------------------------------- */

typedef struct
{
  double x;
  double y;
  int x_px;
  int y_px;
  char *x_label;
  char *y_label;
  char *label;
} grm_tooltip_info_t;

typedef struct
{
  int n;
  double x;
  double *y;
  int x_px;
  int y_px;
  char *x_label;
  char **y_labels;
} grm_accumulated_tooltip_info_t;

/* ========================= functions ============================================================================== */

/* ------------------------- user interaction ----------------------------------------------------------------------- */

GRM_EXPORT int grm_input(const grm_args_t *input_args);
GRM_EXPORT int grm_get_box(const int x1, const int y1, const int x2, const int y2, const int keep_aspect_ratio, int *x,
                           int *y, int *w, int *h);
GRM_EXPORT int grm_is3d(const int x, const int y);
GRM_EXPORT grm_tooltip_info_t *grm_get_tooltip(int mouse_x, int mouse_y);
GRM_EXPORT grm_tooltip_info_t **grm_get_tooltips_x(int mouse_x, int mouse_y, unsigned int *array_length);
GRM_EXPORT grm_accumulated_tooltip_info_t *grm_get_accumulated_tooltip_x(int mouse_x, int mouse_y);
GRM_EXPORT int grm_get_hover_mode(int mouse_x, int mouse_y, int disable_movable_xform);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_INTERACTION_H_INCLUDED */
