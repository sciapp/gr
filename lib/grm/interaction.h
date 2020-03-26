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
  char *xlabel;
  char *ylabel;
  char *label;
} grm_tooltip_info_t;

/* ========================= functions ============================================================================== */

/* ------------------------- user interaction ----------------------------------------------------------------------- */

EXPORT int grm_input(const grm_args_t *);
EXPORT int grm_get_box(const int, const int, const int, const int, const int, int *, int *, int *, int *);
EXPORT int grm_input(const grm_args_t *input_args);
EXPORT int grm_get_box(const int x1, const int y1, const int x2, const int y2, const int keep_aspect_ratio, int *x,
                       int *y, int *w, int *h);
EXPORT grm_tooltip_info_t *grm_get_tooltip(const int, const int);


#ifdef __cplusplus
}
#endif

#endif /* ifndef GRM_INTERACTION_H_INCLUDED */
