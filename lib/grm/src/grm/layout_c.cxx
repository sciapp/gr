/* ######################### includes ############################################################################### */

#include <grm/layout.h>
#include <grm/layout.hxx>
#include <grm/layout_error.hxx>

/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- grid ----------------------------------------------------------------------------------- */

using namespace grm;

err_t grid_new(int nrows, int ncols, grid_t **a_grid)
{
  try
    {
      Grid *grid = new Grid(nrows, ncols);
      *a_grid = reinterpret_cast<grid_t *>(grid);
    }
  catch (std::bad_alloc)
    {
      return ERROR_MALLOC;
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return ERROR_NONE;
}

void grid_print(const grid_t *a_grid)
{

  const Grid *grid = reinterpret_cast<const Grid *>(a_grid);
  grid->printGrid();
}

err_t grid_setElement(int row, int col, element_t *a_element, grid_t *a_grid)
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
  return ERROR_NONE;
}

err_t grid_setElementArgs(int row, int col, grm_args_t *subplot_args, grid_t *a_grid)
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
  return ERROR_NONE;
}

err_t grid_setElementSlice(int rowStart, int rowStop, int colStart, int colStop, element_t *a_element, grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  try
    {
      Slice slice(rowStart, rowStop, colStart, colStop);
      grid->setElement(&slice, element);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return ERROR_NONE;
}

err_t grid_setElementArgsSlice(int rowStart, int rowStop, int colStart, int colStop, grm_args_t *subplot_args,
                               grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  try
    {
      Slice slice(rowStart, rowStop, colStart, colStop);
      grid->setElement(&slice, subplot_args);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return ERROR_NONE;
}

err_t grid_ensureCellIsGrid(int row, int col, grid_t *a_grid)
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
  return ERROR_NONE;
}

err_t grid_ensureCellsAreGrid(int rowStart, int rowStop, int colStart, int colStop, grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  try
    {
      Slice slice(rowStart, rowStop, colStart, colStop);
      grid->ensureCellsAreGrid(&slice);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return ERROR_NONE;
}

err_t grid_getElement(int row, int col, grid_t *a_grid, element_t **a_element)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  try
    {
      GridElement *element = grid->getElement(row, col);
      *a_element = reinterpret_cast<element_t *>(element);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return ERROR_NONE;
}

void grid_delete(const grid_t *a_grid)
{
  const Grid *grid = reinterpret_cast<const Grid *>(a_grid);

  delete grid;
}

void grid_finalize(grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);

  grid->finalizePlot();
}

void trim(grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  grid->trim();
}

/* ------------------------- element -------------------------------------------------------------------------------- */

err_t element_new(element_t **a_element)
{
  GridElement *element;
  try
    {
      element = new GridElement();
    }
  catch (std::bad_alloc)
    {
      return ERROR_MALLOC;
    }
  *a_element = reinterpret_cast<element_t *>(element);

  return ERROR_NONE;
}

err_t element_setAbsHeight(element_t *a_element, double height)
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
  return ERROR_NONE;
}

err_t element_setAbsHeightPxl(element_t *a_element, int height)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  try
    {
      element->setAbsHeightPxl(height);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return ERROR_NONE;
}

err_t element_setRelativeHeight(element_t *a_element, double height)
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
  return ERROR_NONE;
}

err_t element_setAbsWidth(element_t *a_element, double width)
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
  return ERROR_NONE;
}

err_t element_setAbsWidthPxl(element_t *a_element, int width)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  try
    {
      element->setAbsWidthPxl(width);
    }
  catch (const InvalidArgument &e)
    {
      return e.getErrorNumber();
    }
  return ERROR_NONE;
}

err_t element_setRelativeWidth(element_t *a_element, double width)
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
  return ERROR_NONE;
}

err_t element_setAspectRatio(element_t *a_element, double ar)
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
  return ERROR_NONE;
}

void element_setFitParentsHeight(element_t *a_element, int fitParentsHeight)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  element->setFitParentsHeight(fitParentsHeight);
}

void element_setFitParentsWidth(element_t *a_element, int fitParentsWidth)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  element->setFitParentsWidth(fitParentsWidth);
}

void element_getSubplot(element_t *a_element, double **subplot)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  *subplot = element->getPlot();
}
