#ifndef LAYOUT_H_INCLUDED
#define LAYOUT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "args.h"
#include "error.h"
#include "util.h"

/* ######################### public interface ####################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- grm_grid_t ----------------------------------------------------------------------------- */

struct _grm_grid_t;
typedef struct grm_grid grm_grid_t;

/* ------------------------- grm_element_t -------------------------------------------------------------------------- */

struct _grm_element_t;
typedef struct grm_element grm_element_t;

/* ========================= methods ================================================================================ */

/* ------------------------- grid ----------------------------------------------------------------------------------- */

GRM_EXPORT grm_error_t grm_grid_new(int nrows, int ncols, grm_grid_t **a_grid);
GRM_EXPORT void grm_grid_print(const grm_grid_t *grid);
GRM_EXPORT grm_error_t grm_grid_set_element(int row, int col, grm_element_t *a_element, grm_grid_t *a_grid);
GRM_EXPORT grm_error_t grm_grid_set_element_args(int row, int col, grm_args_t *subplot_args, grm_grid_t *a_grid);
GRM_EXPORT grm_error_t grm_grid_set_element_slice(int row_start, int row_stop, int col_start, int col_stop,
                                                  grm_element_t *a_element, grm_grid_t *a_grid);
GRM_EXPORT grm_error_t grm_grid_set_element_args_slice(int row_start, int row_stop, int col_start, int col_stop,
                                                       grm_args_t *subplot_args, grm_grid_t *a_grid);
GRM_EXPORT grm_error_t grm_grid_get_element(int row, int col, grm_grid_t *a_grid, grm_element_t **a_element);
GRM_EXPORT grm_error_t grm_grid_ensure_cell_is_grid(int row, int col, grm_grid_t *a_grid);
GRM_EXPORT grm_error_t grm_grid_ensure_cells_are_grid(int row_start, int row_stop, int col_start, int col_stop,
                                                      grm_grid_t *a_grid);
GRM_EXPORT void grm_grid_finalize(grm_grid_t *a_grid);
GRM_EXPORT void grm_grid_delete(const grm_grid_t *grid);
GRM_EXPORT void grm_trim(grm_grid_t *a_grid);

/* ------------------------- element -------------------------------------------------------------------------------- */

GRM_EXPORT grm_error_t grm_element_new(grm_element_t **a_element);
GRM_EXPORT grm_error_t grm_element_set_abs_height(grm_element_t *a_element, double height);
GRM_EXPORT grm_error_t grm_element_set_relative_height(grm_element_t *a_element, double height);
GRM_EXPORT grm_error_t grm_element_set_abs_width(grm_element_t *a_element, double width);
GRM_EXPORT grm_error_t grm_element_set_relative_width(grm_element_t *a_element, double width);
GRM_EXPORT grm_error_t grm_element_set_aspect_ratio(grm_element_t *a_element, double ar);
GRM_EXPORT void grm_element_set_fit_parents_height(grm_element_t *a_element, int fit_parents_height);
GRM_EXPORT void grm_element_set_fit_parents_width(grm_element_t *a_element, int fit_parents_width);

GRM_EXPORT void grm_element_get_subplot(grm_element_t *a_element, double **subplot);

#ifdef __cplusplus
}
#endif
#endif /* ifndef LAYOUT_H_INCLUDED */
