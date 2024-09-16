#ifndef GRM_IMPORT_H_INCLUDED
#define GRM_IMPORT_H_INCLUDED

#include "plot.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include "args.h"
#include "util.h"
#include "net.h"


/* ######################### public interface ####################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- argument ------------------------------------------------------------------------------- */

struct _grm_file_args_t;
typedef struct _grm_file_args_t grm_file_args_t;

/* ========================= functions ============================================================================== */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

EXPORT int grm_interactive_plot_from_file(grm_args_t *args, int argc, char **argv);
EXPORT int grm_plot_from_file(int argc, char **argv);

#ifdef __cplusplus
}
EXPORT int grm_context_data_from_file(const std::shared_ptr<GRM::Context> &context, const std::string &path,
                                      bool interpret_matrix_as_one_column = false);
#endif
#endif // GRM_IMPORT_H_INCLUDED
