#ifndef GRM_IMPORT_H_INCLUDED
#define GRM_IMPORT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include "args.h"
#include "util.h"
#include "net.h"
#include "plot.h"


/* ######################### public interface ####################################################################### */

/* ========================= functions ============================================================================== */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

EXPORT int grm_interactive_plot_from_file(grm_args_t *args, const char *data_file, const char **plot_type,
                                          const char *colms, const char *heatmap_type, const char *heatmap_algo);
EXPORT int grm_plot_from_file(const char *data_file, const char **plot_type, const char *colms);

#ifdef __cplusplus
}
#endif
#endif // GRM_IMPORT_H_INCLUDED
