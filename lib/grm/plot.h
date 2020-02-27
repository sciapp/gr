#ifndef GRM_PLOT_H_INCLUDED
#define GRM_PLOT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include "args.h"
#include "util.h"


/* ######################### public interface ####################################################################### */

/* ========================= functions ============================================================================== */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

EXPORT void grm_finalize(void);
EXPORT int grm_clear(void);
EXPORT unsigned int grm_max_plotid(void);
EXPORT int grm_merge(const grm_args_t *args);
EXPORT int grm_merge_extended(const grm_args_t *args, int hold, const char *identificator);
EXPORT int grm_merge_hold(const grm_args_t *args);
EXPORT int grm_merge_named(const grm_args_t *args, const char *identificator);
EXPORT int grm_plot(const grm_args_t *args);
EXPORT int grm_switch(unsigned int id);


#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_PLOT_H_INCLUDED */
