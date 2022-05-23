/* ######################### includes ############################################################################### */

#include <grm/layout.h>
#include <grm/layout.hpp>


/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- grid ----------------------------------------------------------------------------------- */

grid_t *grid_new(int nrows, int ncols)
{
  Grid *grid = new Grid(nrows, ncols);

  return reinterpret_cast<grid_t *>(grid);
}

void grid_print(const grid_t *a_grid)
{

  const Grid *grid = reinterpret_cast<const Grid *>(a_grid);
  grid->printGrid();
}

void grid_setElement(int row, int col, element_t *a_element, grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  grid->setElement(row, col, element);
}

void grid_setElementArgs(int row, int col, grm_args_t *subplot_args, grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  grid->setElement(row, col, subplot_args);
}

void grid_setElementSlice(int rowStart, int rowStop, int colStart, int colStop, element_t *a_element, grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  GridElement *element = reinterpret_cast<GridElement *>(a_element);
  Slice slice(rowStart, rowStop, colStart, colStop);
  grid->setElement(&slice, element);
}

void grid_setElementArgsSlice(int rowStart, int rowStop, int colStart, int colStop, grm_args_t *subplot_args,
                              grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  Slice slice(rowStart, rowStop, colStart, colStop);
  grid->setElement(&slice, subplot_args);
}

void grid_ensureCellIsGrid(int row, int col, grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  grid->ensureCellIsGrid(row, col);
}

void grid_ensureCellsAreGrid(int rowStart, int rowStop, int colStart, int colStop, grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  Slice slice(rowStart, rowStop, colStart, colStop);
  grid->ensureCellsAreGrid(&slice);
}

element_t *gird_getElement(int row, int col, grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  GridElement *element = grid->getElement(row, col);
  return reinterpret_cast<element_t *>(element);
}

void grid_delete(const grid_t *a_grid)
{
  const Grid *grid = reinterpret_cast<const Grid *>(a_grid);

  delete grid;
}

void grid_finalize(grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);

  grid->finalizeSubplot();
}

void trim(grid_t *a_grid)
{
  Grid *grid = reinterpret_cast<Grid *>(a_grid);
  grid->trim();
}

/* ------------------------- element -------------------------------------------------------------------------------- */

element_t *element_new()
{
  GridElement *element = new GridElement();

  return reinterpret_cast<element_t *>(element);
}

void element_setAbsHeight(element_t *a_element, double height)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  element->setAbsHeight(height);
}

void element_setAbsHeightPxl(element_t *a_element, int height)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  element->setAbsHeightPxl(height);
}

void element_setRelativeHeight(element_t *a_element, double height)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  element->setRelativeHeight(height);
}

void element_setAbsWidth(element_t *a_element, double width)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  element->setAbsWidth(width);
}

void element_setAbsWidthPxl(element_t *a_element, int width)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  element->setAbsWidthPxl(width);
}

void element_setRelativeWidth(element_t *a_element, double width)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  element->setRelativeWidth(width);
}

void element_setAspectRatio(element_t *a_element, double ar)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  element->setAspectRatio(ar);
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

double *element_getSubplot(element_t *a_element)
{
  GridElement *element = reinterpret_cast<GridElement *>(a_element);

  return element->getSubplot();
}
