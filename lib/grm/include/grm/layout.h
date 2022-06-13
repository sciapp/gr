#ifndef LAYOUT_H_INCLUDED
#define LAYOUT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "args.h"
#include "error.h"

/* ######################### public interface ####################################################################### */

/* ========================= datatypes ============================================================================== */

/* ------------------------- grid ----------------------------------------------------------------------------------- */

struct _grid_t;
typedef struct _grid_t grid_t;

/* ------------------------- element -------------------------------------------------------------------------------- */

struct _element_t;
typedef struct _element_t element_t;

/* ========================= methods ================================================================================ */

/* ------------------------- grid ----------------------------------------------------------------------------------- */

err_t grid_new(int nrows, int ncols, grid_t **a_grid);
void grid_print(const grid_t *grid);
err_t grid_setElement(int row, int col, element_t *a_element, grid_t *a_grid);
err_t grid_setElementArgs(int row, int col, grm_args_t *subplot_args, grid_t *a_grid);
err_t grid_setElementSlice(int rowStart, int rowStop, int colStart, int colStop, element_t *a_element, grid_t *a_grid);
err_t grid_setElementArgsSlice(int rowStart, int rowStop, int colStart, int colStop, grm_args_t *subplot_args,
                               grid_t *a_grid);
err_t grid_getElement(int row, int col, grid_t *a_grid, element_t **a_element);
err_t grid_ensureCellIsGrid(int row, int col, grid_t *a_grid);
err_t grid_ensureCellsAreGrid(int rowStart, int rowStop, int colStart, int colStop, grid_t *a_grid);
void grid_finalize(grid_t *a_grid);
void grid_delete(const grid_t *grid);
void trim(grid_t *a_grid);

/* ------------------------- element -------------------------------------------------------------------------------- */

err_t element_new(element_t **a_element);
err_t element_setAbsHeight(element_t *a_element, double height);
err_t element_setAbsHeightPxl(element_t *a_element, int height);
err_t element_setRelativeHeight(element_t *a_element, double height);
err_t element_setAbsWidth(element_t *a_element, double width);
err_t element_setAbsWidthPxl(element_t *a_element, int width);
err_t element_setRelativeWidth(element_t *a_element, double width);
err_t element_setAspectRatio(element_t *a_element, double ar);
void element_setFitParentsHeight(element_t *a_element, int fitParentsHeight);
void element_setFitParentsWidth(element_t *a_element, int fitParentsWidth);

void element_getSubplot(element_t *a_element, double **subplot);

#ifdef __cplusplus
}
#endif
#endif /* ifndef LAYOUT_H_INCLUDED */
