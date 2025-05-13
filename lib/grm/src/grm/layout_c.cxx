/* ######################### includes ############################################################################### */

#include <grm/layout.h>
#include <grm/layout.hxx>
#include <grm/layout_error.hxx>

/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- grid ----------------------------------------------------------------------------------- */

using namespace GRM;

grm_error_t grm_grid_new(int nrows, int ncols, grm_grid_t **a_grid)
{
  try
    {
      Grid *new_grid = new Grid(nrows, ncols);
      *a_grid = reinterpret_cast<grm_grid_t *>(new_grid);
    }
  catch (std::bad_alloc)
    {
      return GRM_ERROR_MALLOC;
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

void grm_grid_print(const grm_grid_t *a_grid)
{

  const Grid *grid = reinterpret_cast<const Grid *>(a_grid);
  grid->printGrid();
}

grm_error_t grm_grid_set_element(int row, int col, grm_element_t *a_element, grm_grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  try
    {
      grid->setElement(row, col, element);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

grm_error_t grm_grid_set_element_args(int row, int col, grm_args_t *subplot_args, grm_grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  try
    {
      grid->setElement(row, col, subplot_args);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

grm_error_t grm_grid_set_element_slice(int row_start, int row_stop, int col_start, int col_stop,
                                       grm_element_t *a_element, grm_grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  try
    {
      Slice slice(row_start, row_stop, col_start, col_stop);
      grid->setElement(&slice, element);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

grm_error_t grm_grid_set_element_args_slice(int row_start, int row_stop, int col_start, int col_stop,
                                            grm_args_t *subplot_args, grm_grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  try
    {
      Slice slice(row_start, row_stop, col_start, col_stop);
      grid->setElement(&slice, subplot_args);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

grm_error_t grm_grid_ensure_cell_is_grid(int row, int col, grm_grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  try
    {
      grid->ensureCellIsGrid(row, col);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

grm_error_t grm_grid_ensure_cells_are_grid(int row_start, int row_stop, int col_start, int col_stop, grm_grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  try
    {
      Slice slice(row_start, row_stop, col_start, col_stop);
      grid->ensureCellsAreGrid(&slice);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

grm_error_t grm_grid_get_element(int row, int col, grm_grid_t *a_grid, grm_element_t **a_element)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  try
    {
      GridElement *grid_element = grid->getElement(row, col);
      *a_element = reinterpret_cast<grm_element_t *>(grid_element);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

void grm_grid_delete(const grm_grid_t *a_grid)
{
  const Grid *grid = reinterpret_cast<const Grid *>(a_grid);

  delete grid;
}

void grm_grid_finalize(grm_grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);

  grid->finalizePlot();
}

void grm_trim(grm_grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  grid->trim();
}

/* ------------------------- element -------------------------------------------------------------------------------- */

grm_error_t grm_element_new(grm_element_t **a_element)
{
  GridElement *grid_element;
  try
    {
      grid_element = new GridElement();
    }
  catch (std::bad_alloc)
    {
      return GRM_ERROR_MALLOC;
    }
  *a_element = reinterpret_cast<grm_element_t *>(grid_element);

  return GRM_ERROR_NONE;
}

grm_error_t grm_element_set_abs_height(grm_element_t *a_element, double height)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  try
    {
      element->setAbsHeight(height);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

grm_error_t grm_element_set_relative_height(grm_element_t *a_element, double height)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  try
    {
      element->setRelativeHeight(height);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

grm_error_t grm_element_set_abs_width(grm_element_t *a_element, double width)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  try
    {
      element->setAbsWidth(width);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

grm_error_t grm_element_set_relative_width(grm_element_t *a_element, double width)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  try
    {
      element->setRelativeWidth(width);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

grm_error_t grm_element_set_aspect_ratio(grm_element_t *a_element, double ar)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  try
    {
      element->setAspectRatio(ar);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return GRM_ERROR_NONE;
}

void grm_element_set_fit_parents_height(grm_element_t *a_element, int fit_parents_height)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  element->setFitParentsHeight(fit_parents_height);
}

void grm_element_set_fit_parents_width(grm_element_t *a_element, int fit_parents_width)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  element->setFitParentsWidth(fit_parents_width);
}

void grm_element_get_subplot(grm_element_t *a_element, double **subplot)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  *subplot = element->getPlot();
}
