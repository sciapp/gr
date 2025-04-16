#ifndef GRM_DUMP_H_INCLUDED
#define GRM_DUMP_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdio.h>

#include "args.h"
#include "util.h"


/* ######################### public interface ####################################################################### */

/* ========================= functions ============================================================================== */

/* ------------------------- dump ----------------------------------------------------------------------------------- */

GRM_EXPORT void grm_dump(const grm_args_t *args, FILE *f);
GRM_EXPORT void grm_dump_json(const grm_args_t *args, FILE *f);
GRM_EXPORT char *grm_dump_json_str(void);
GRM_EXPORT char *grm_dump_html(char *plot_id);
GRM_EXPORT char *grm_dump_html_args(char *plot_id, grm_args_t *args);
GRM_EXPORT void grm_dump_bson(const grm_args_t *args, FILE *f);

#ifdef __cplusplus
}
#endif
#endif /* ifndef GRM_DUMP_H_INCLUDED */
