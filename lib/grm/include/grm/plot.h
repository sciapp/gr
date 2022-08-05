#ifndef GRM_PLOT_H_INCLUDED
#define GRM_PLOT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ######################### includes ############################################################################### */

#include <stdio.h>

#include "args.h"
#include "util.h"

#ifdef __cplusplus
}

/* clang-format off */
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/dom_render/render.hxx>
/* clang-format on */

extern "C" {
#endif

/* ######################### public interface ####################################################################### */

/* ========================= functions ============================================================================== */

/* ------------------------- plot ----------------------------------------------------------------------------------- */

EXPORT void grm_finalize(void);
EXPORT int grm_clear(void);
EXPORT void grm_dump_graphics_tree(FILE *f);
EXPORT char *grm_dump_graphics_tree_str(void);
EXPORT unsigned int grm_max_plotid(void);
EXPORT int grm_merge(const grm_args_t *args);
EXPORT int grm_merge_extended(const grm_args_t *args, int hold, const char *identificator);
EXPORT int grm_merge_hold(const grm_args_t *args);
EXPORT int grm_merge_named(const grm_args_t *args, const char *identificator);
EXPORT int grm_plot(const grm_args_t *args);
EXPORT void grm_render(void);
EXPORT int grm_switch(unsigned int id);

#ifdef __cplusplus
}

EXPORT std::shared_ptr<GR::Element> grm_get_document_root(void);

#endif
#endif /* ifndef GRM_PLOT_H_INCLUDED */