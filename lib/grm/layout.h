#ifndef LAYOUT_H_INCLUDED
#define LAYOUT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

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

grid_t *grid_new(int nrows, int ncols);
void grid_print(const grid_t *grid);
void grid_setElement(int row, int col, element_t *a_element, grid_t *a_grid);
void grid_finalize(grid_t *a_grid);
void grid_delete(const grid_t *grid);

/* ------------------------- element -------------------------------------------------------------------------------- */

element_t *element_new();
void element_setAbsHeight(element_t *a_element, double height);
void element_setAbsHeightPxl(element_t *a_element, int height);
void element_setRelativeHeight(element_t *a_element, double height);
void element_setAbsWidth(element_t *a_element, double width);
void element_setAbsWidthPxl(element_t *a_element, int width);
void element_setRelativeWidth(element_t *a_element, double width);
void element_setAspectRatio(element_t *a_element, double ar);
void element_setFitParentsHeight(element_t *a_element, int fitParentsHeight);
void element_setFitParentsWidth(element_t *a_element, int fitParentsWidth);

double *element_getSubplot(element_t *a_element);

#ifdef __cplusplus
}
#endif
#endif /* ifndef LAYOUT_H_INCLUDED */
