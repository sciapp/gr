#ifndef GRM_INTERACTION_H_INCLUDED
#define GRM_INTERACTION_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include "args.h"
#include "util.h"


/* ######################### public interface ####################################################################### */

/* ========================= functions ============================================================================== */

/* ------------------------- user interaction ----------------------------------------------------------------------- */

EXPORT int grm_input(const grm_args_t *);
EXPORT int grm_get_box(const int x1, const int y1, const int x2, const int y2, const int keep_aspect_ratio, int *x,
                       int *y, int *w, int *h);


#ifdef __cplusplus
}
#endif

#endif /* ifndef GRM_INTERACTION_H_INCLUDED */
