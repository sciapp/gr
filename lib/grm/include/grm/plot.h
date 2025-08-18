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

GRM_EXPORT void grm_finalize(void);
GRM_EXPORT int grm_clear(void);
GRM_EXPORT void grm_dump_graphics_tree(FILE *f);
GRM_EXPORT char *grm_dump_graphics_tree_str(void);
GRM_EXPORT unsigned int grm_max_plot_id(void);
GRM_EXPORT int grm_merge(const grm_args_t *args);
GRM_EXPORT int grm_merge_extended(const grm_args_t *args, int hold, const char *identificator);
GRM_EXPORT int grm_merge_hold(const grm_args_t *args);
GRM_EXPORT int grm_merge_named(const grm_args_t *args, const char *identificator);
GRM_EXPORT int grm_plot(const grm_args_t *args);
GRM_EXPORT int grm_render(void);
GRM_EXPORT int grm_process_tree(void);
GRM_EXPORT int grm_export(const char *file_path);
GRM_EXPORT int grm_switch(unsigned int id);
GRM_EXPORT int grm_get_error_code();

#if !defined(NO_XERCES_C)
GRM_EXPORT int grm_load_graphics_tree(FILE *file);
#endif
GRM_EXPORT int grm_validate(void);

#ifdef __cplusplus
}

GRM_EXPORT std::shared_ptr<GRM::Element> grm_get_document_root(void);
GRM_EXPORT std::shared_ptr<GRM::Render> grm_get_render(void);
GRM_EXPORT int grm_iterate_grid(GRM::Grid *grid, const std::shared_ptr<GRM::Element> &parent_dom_element, int plot_id);
GRM_EXPORT int grm_plot_helper(GRM::GridElement *grid_element, GRM::Slice *slice,
                               const std::shared_ptr<GRM::Element> &parent_dom_element, int plot_id);
GRM_EXPORT std::shared_ptr<GRM::Element> grm_get_subplot_from_ndc_point_using_dom(double x, double y);
GRM_EXPORT std::shared_ptr<GRM::Element> grm_get_subplot_from_ndc_points_using_dom(unsigned int n, const double *x,
                                                                                   const double *y);
GRM_EXPORT void grm_set_attribute_on_all_subplots(std::string attribute, int value);
GRM_EXPORT int grm_get_focus_and_factor_from_dom(const int x1, const int y1, const int x2, const int y2,
                                                 const int keep_aspect_ratio, double *factor_x, double *factor_y,
                                                 double *focus_x, double *focus_y,
                                                 std::shared_ptr<GRM::Element> &subplot_element);
GRM_EXPORT std::map<std::string, std::list<std::string>> grm_get_context_data();

#if !defined(NO_XERCES_C)
GRM_EXPORT std::shared_ptr<GRM::Document> grm_load_graphics_tree_schema(bool with_private_attributes = false);
#endif
#endif
#endif /* ifndef GRM_PLOT_H_INCLUDED */
