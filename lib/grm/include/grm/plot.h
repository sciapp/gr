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
EXPORT int grm_render(void);
EXPORT int grm_process_tree(void);
EXPORT int grm_export(const char *file_path);
EXPORT int grm_switch(unsigned int id);

#if !defined(NO_EXPAT) || !defined(NO_LIBXML2)
EXPORT int grm_load_graphics_tree(FILE *file);
#endif

#ifdef __cplusplus
}

EXPORT std::shared_ptr<GRM::Element> grm_get_document_root(void);
EXPORT std::shared_ptr<GRM::Render> grm_get_render(void);
EXPORT int grm_iterate_grid(grm::Grid *grid, const std::shared_ptr<GRM::Element> &parentDomElement, int plotId);
EXPORT int grm_plot_helper(grm::GridElement *gridElement, grm::Slice *slice,
                           const std::shared_ptr<GRM::Element> &parentDomElement, int plotId);
EXPORT std::shared_ptr<GRM::Element> get_subplot_from_ndc_point_using_dom(double x, double y);
EXPORT std::shared_ptr<GRM::Element> get_subplot_from_ndc_points_using_dom(unsigned int n, const double *x,
                                                                           const double *y);
EXPORT void grm_set_attribute_on_all_subplots(std::string attribute, int value);
EXPORT int get_focus_and_factor_from_dom(const int x1, const int y1, const int x2, const int y2,
                                         const int keep_aspect_ratio, double *factor_x, double *factor_y,
                                         double *focus_x, double *focus_y,
                                         std::shared_ptr<GRM::Element> &subplot_element);
EXPORT bool grm_validate(void);

#if !defined(NO_LIBXML2)
EXPORT std::shared_ptr<GRM::Document> grm_load_graphics_tree_schema(void);
#endif
#endif
#endif /* ifndef GRM_PLOT_H_INCLUDED */
