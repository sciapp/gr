#include <iostream>
#include <limits>
#include <sstream>
#include "args_int.h"
#include <grm/layout_error.hxx>
#include "grm/layout.hxx"

using namespace grm;

double epsilon = std::numeric_limits<double>::epsilon();

Slice::Slice(int rowStart, int rowStop, int colStart, int colStop)
    : row_start(rowStart), row_stop(rowStop), col_start(colStart), col_stop(colStop)
{
  if (!this->isPositive())
    {
      throw InvalidIndex("Indices must be positive values");
    }
  if (!this->isForward())
    {
      throw InvalidIndex("Start value can`t be bigger than stop value");
    }
}

Slice *Slice::copy()
{
  Slice *copy = new Slice(this->row_start, this->row_stop, this->col_start, this->col_stop);
  return copy;
}

bool Slice::isPositive()
{
  if (row_start < 0 || row_stop < 0 || col_start < 0 || col_stop < 0)
    {
      return false;
    }
  else
    {
      return true;
    }
}

bool Slice::isForward()
{
  if (row_start > row_stop || col_start > col_stop)
    {
      return false;
    }
  else
    {
      return true;
    }
}

GridElement::GridElement()
{
  plot = new double[4];
};

GridElement::GridElement(double absHeight, double absWidth, int absHeightPxl, int absWidthPxl, int fitParentsHeight,
                         int fitParentsWidth, double relativeHeight, double relativeWidth, double aspectRatio)
    : abs_height(absHeight), abs_width(absWidth), abs_height_pxl(absHeightPxl), abs_width_pxl(absWidthPxl),
      fit_parents_height(fitParentsHeight), fit_parents_width(fitParentsWidth), relative_height(relativeHeight),
      relative_width(relativeWidth), aspect_ratio(aspectRatio)
{
  setAbsHeight(absHeight);
  setAbsWidth(absWidth);
  setAbsHeightPxl(absHeightPxl);
  setAbsWidthPxl(absWidthPxl);
  setRelativeHeight(relativeHeight);
  setRelativeWidth(relativeWidth);
  plot = new double[4];
}

GridElement::~GridElement() {}

void GridElement::setPlot(double x1, double x2, double y1, double y2)
{
  if (finalized || plot_set == 0)
    {
      plot[0] = x1;
      plot[1] = x2;
      plot[2] = y1;
      plot[3] = y2;

      finalized = 0;
      plot_set = 1;
    }
  else
    {
      if (y1 < plot[2])
        {
          plot[2] = y1;
        }
      if (x2 > plot[1])
        {
          plot[1] = x2;
        }
    }
}

void GridElement::setAbsHeight(double height)
{
  if (height_set && height != -1)
    {
      throw ContradictingAttributes("Can only set one height attribute");
    }
  if ((height <= 0 || height > 1) && height != -1)
    {
      throw std::invalid_argument("Height has to be between 0 and 1 or be -1");
    }
  if (ar_set && width_set && height != -1)
    {
      throw ContradictingAttributes("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  abs_height = height;
  height_set = (height != -1) ? 1 : 0;
}

void GridElement::setAbsHeightPxl(int height)
{
  if (height_set && height != -1)
    {
      throw ContradictingAttributes("Can only set one height attribute");
    }
  if (height <= 0 && height != -1)
    {
      throw InvalidArgumentRange("Pixel height has to be an positive integer or be -1");
    }
  if (ar_set && width_set && height != -1)
    {
      throw ContradictingAttributes("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  abs_height_pxl = height;
  height_set = (height != -1) ? 1 : 0;
}

void GridElement::setRelativeHeight(double height)
{
  if (height_set && height != -1)
    {
      throw ContradictingAttributes("Can only set one height attribute");
    }
  if ((height <= 0 || height > 1) && height != -1)
    {
      throw InvalidArgumentRange("Height has to be between 0 and 1 or be -1");
    }
  if (ar_set && width_set && height != -1)
    {
      throw ContradictingAttributes("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  relative_height = height;
  height_set = (height != -1) ? 1 : 0;
}

void GridElement::setAbsWidth(double width)
{
  if (width_set && width != -1)
    {
      throw ContradictingAttributes("Can only set one width attribute");
    }
  if ((width <= 0 || width > 1) && width != -1)
    {
      throw InvalidArgumentRange("Width has to be between 0 and 1 or be -1");
    }
  if (ar_set and height_set)
    {
      throw ContradictingAttributes("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  abs_width = width;
  width_set = (width != -1) ? 1 : 0;
}

void GridElement::setAbsWidthPxl(int width)
{
  if (width_set && width != -1)
    {
      throw ContradictingAttributes("Can only set one width attribute");
    }
  if (width <= 0 && width != -1)
    {
      throw InvalidArgumentRange("Pixel Width has to be an positive integer or be -1");
    }
  if (ar_set && height_set && width != -1)
    {
      throw ContradictingAttributes("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  abs_width_pxl = width;
  width_set = (width != -1) ? 1 : 0;
}

void GridElement::setRelativeWidth(double width)
{
  if (width_set && width != -1)
    {
      throw ContradictingAttributes("Can only set one width attribute");
    }
  if ((width <= 0 || width > 1) && width != -1)
    {
      throw InvalidArgumentRange("Width has to be between 0 and 1 or be -1");
    }
  if (ar_set && height_set && width != -1)
    {
      throw ContradictingAttributes("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  relative_width = width;
  width_set = (width != -1) ? 1 : 0;
}

void GridElement::setAspectRatio(double ar)
{
  if (ar <= 0 && ar != -1)
    {
      throw InvalidArgumentRange("Aspect ration has to be bigger than 0 or be -1");
    }
  if (width_set && height_set && ar != -1)
    {
      throw ContradictingAttributes("You cant restrict the aspect ratio on a plot with fixed sides");
    }
  aspect_ratio = ar;
  ar_set = (ar != -1) ? 1 : 0;
}

void GridElement::finalizePlot()
{
  if (finalized)
    {
      return;
    }

  if (abs_height != -1)
    {
      double availableHeight = plot[3] - plot[2];
      if (abs_height > availableHeight + epsilon)
        {
          throw ContradictingAttributes("Absolute height is bigger than available height");
        }
      double middle = plot[2] + availableHeight / 2;
      plot[2] = middle - abs_height / 2;
      plot[3] = middle + abs_height / 2;
    }

  if (abs_width != -1)
    {
      double availableWidth = plot[1] - plot[0];
      if (abs_width > availableWidth + epsilon)
        {
          throw ContradictingAttributes("Absolute width is bigger than available width");
        }
      double middle = plot[0] + availableWidth / 2;
      plot[0] = middle - abs_width / 2;
      plot[1] = middle + abs_width / 2;
    }

  if (relative_height != -1)
    {
      double availableHeight = plot[3] - plot[2];
      double middle = plot[2] + availableHeight / 2;
      double newHeight = availableHeight * relative_height;
      plot[2] = middle - newHeight / 2;
      plot[3] = middle + newHeight / 2;
    }

  if (relative_width != -1)
    {
      double availableWidth = plot[1] - plot[0];
      double middle = plot[0] + availableWidth / 2;
      double newWidth = availableWidth * relative_width;
      plot[0] = middle - newWidth / 2;
      plot[1] = middle + newWidth / 2;
    }

  /* TODO: implement for pxl */

  if (ar_set)
    {
      double currentHeigth = (plot[3] - plot[2]);
      double currentWidth = (plot[1] - plot[0]);
      double currentAR = currentWidth / currentHeigth;

      if (currentAR < aspect_ratio)
        {
          double newHeight = currentWidth / aspect_ratio;
          double middle = plot[2] + currentHeigth / 2;
          plot[2] = middle - newHeight / 2;
          plot[3] = middle + newHeight / 2;
        }
      else
        {
          double newWidth = currentHeigth * aspect_ratio;
          double middle = plot[0] + currentWidth / 2;
          plot[0] = middle - newWidth;
          plot[1] = middle + newWidth;
        }
    }

  if (plot_args != nullptr)
    {
      grm_args_push(plot_args, "subplot", "nD", 4, plot);
    }

  if (element_in_dom != nullptr)
    {
      element_in_dom->setAttribute("plot_x_min", plot[0]);
      element_in_dom->setAttribute("plot_x_max", plot[1]);
      element_in_dom->setAttribute("plot_y_min", plot[2]);
      element_in_dom->setAttribute("plot_y_max", plot[3]);
    }

  finalized = 1;
}

double *GridElement::getPlot()
{
  return plot;
}

void GridElement::setFitParentsHeight(bool fitParentsHeight)
{
  this->fit_parents_height = fitParentsHeight;
}

void GridElement::setFitParentsWidth(bool fitParentsWidth)
{
  this->fit_parents_width = fitParentsWidth;
}

bool GridElement::isGrid()
{
  return false;
}

Grid::Grid(int nrows, int ncols) : Grid(nrows, ncols, -1, -1, -1, -1, 0, 1, -1, -1, -1) {}

Grid::Grid(int nrows, int ncols, double absHeight, double absWidth, int absHeightPxl, int absWidthPxl,
           int fitParentsHeight, int fitParentsWidth, double relativeHeight, double relativeWidth, double aspectRatio)
    : GridElement(absHeight, absWidth, absHeightPxl, absWidthPxl, fitParentsHeight, fitParentsWidth, relativeHeight,
                  relativeWidth, aspectRatio),
      nrows(nrows), ncols(ncols)
{
  if (nrows < 1 || ncols < 1)
    {
      throw InvalidArgumentRange("The number of rows and cols in a grid must be bigger than 0");
    }
  int i;
  for (i = 0; i < nrows; ++i)
    {
      std::vector<GridElement *> row(ncols, nullptr);
      rows.push_back(row);
    }
}

Grid::~Grid()
{
  for (auto const &x : this->elementToPosition)
    {
      /* delete allocated elements */
      delete x.first;
      /* delete allocated slices */
      delete x.second;
    }
}

GridElement *Grid::getElement(int row, int col) const
{
  try
    {
      return rows.at(row).at(col);
    }
  catch (const std::out_of_range &e)
    {
      throw InvalidIndex("There is no element at the specified position");
    }
};

void Grid::setElement(int row, int col, GridElement *element)
{
  Slice newSlice(row, row + 1, col, col + 1);
  setElement(&newSlice, element);
}

void Grid::setElement(int row, int col, grm_args_t *plot_args)
{
  Slice newSlice(row, row + 1, col, col + 1);
  setElement(&newSlice, plot_args);
}

void Grid::setElement(Slice *slice, GridElement *element)
{
  int nrowsToAllocate, ncolsToAllocate;
  Slice *oldSlice;
  std::vector<GridElement *> oldElements;

  nrowsToAllocate = slice->row_stop;
  ncolsToAllocate = slice->col_stop;

  /* Resize the container if necessary */
  upsize(nrowsToAllocate, ncolsToAllocate);

  /* Delete element from grid if it already exists */
  try
    {
      oldSlice = elementToPosition.at(element);
      for (int row = oldSlice->row_start; row < oldSlice->row_stop; ++row)
        {
          for (int col = oldSlice->col_start; col < oldSlice->col_stop; ++col)
            {
              rows.at(row).at(col) = nullptr;
            }
        }
      elementToPosition.erase(element);
      delete (oldSlice);
    }
  catch (const std::out_of_range &e)
    {
    };

  for (int row = slice->row_start; row < slice->row_stop; ++row)
    {
      for (int col = slice->col_start; col < slice->col_stop; ++col)
        {
          oldElements.push_back(this->getElement(row, col));
          rows.at(row).at(col) = element;
        }
    }
  Slice *newSlice = slice->copy();
  elementToPosition[element] = newSlice;

  /* Delete old elements */
  for (auto &oldElement : oldElements)
    {
      if (elementToPosition.count(oldElement) == 0)
        {
          delete oldElement;
        }
    }
}

void Grid::setElement(Slice *slice, grm_args_t *plot_args)
{
  GridElement *element = nullptr;
  const char *grid_element_address = nullptr;
  if (grm_args_values(plot_args, "grid_element", "s", &grid_element_address))
    {
      element = reinterpret_cast<GridElement *>(std::stoi(grid_element_address));
    }
  else
    {
      element = new GridElement();
      element->plot_args = plot_args;
    }
  std::stringstream address_stream;
  address_stream << element;
  grm_args_push(plot_args, "grid_element", "s", address_stream.str().c_str());
  setElement(slice, element);
}

void Grid::printGrid() const
{
  double *plot;
  for (int i = 0; i < nrows; i++)
    {
      for (int j = 0; j < ncols; j++)
        {
          plot = getElement(i, j)->plot;
          printf("[%f %f %f %f] ", plot[0], plot[1], plot[2], plot[3]);
        }
      printf("\n");
    }
}

void Grid::finalizePlot()
{
  double xmin, xmax, ymin, ymax, rowHeight, elementWidth;
  int y, x, rowSpan, colSpan;
  GridElement *element;

  if (!plot_set)
    {
      setPlot(0, 1, 0, 1);
    }

  GridElement::finalizePlot();

  /* calculate height of each row */
  std::vector<double> rowHeights(nrows);
  double totalHeightLeft = plot[3] - plot[2];
  int numRowsWithFlexibleHeight = 0;
  for (y = 0; y < nrows; y++)
    {
      double rowHeight = -1;
      for (x = 0; x < ncols; x++)
        {
          element = getElement(y, x);
          if (element != nullptr && element->fit_parents_height && element->abs_height != -1)
            {
              /* taking into account that an element can range over multiple rows */
              rowSpan = this->getRowSpan(element);
              if (element->abs_height / rowSpan > rowHeight)
                {
                  rowHeight = element->abs_height / rowSpan;
                }
            }
        }
      rowHeights[y] = rowHeight;
      if (rowHeight == -1)
        {
          numRowsWithFlexibleHeight += 1;
        }
      else
        {
          totalHeightLeft -= rowHeight;
        }
    }
  if (totalHeightLeft + epsilon < 0)
    {
      throw ContradictingAttributes("Not enough vertical space for the rows");
    }
  if (numRowsWithFlexibleHeight == 0)
    {
      /* distribute the height that is left */
      for (y = 0; y < nrows; y++)
        {
          rowHeights[y] += totalHeightLeft / nrows;
        }
    }

  /* calculate width of each column */
  double totalWidthLeft = plot[1] - plot[0];
  std::vector<double> colWidths(ncols);
  int numColsWithFlexibleWidth = 0;
  for (x = 0; x < ncols; x++)
    {
      double colWidth = -1;
      for (y = 0; y < nrows; y++)
        {
          element = getElement(y, x);
          if (element != nullptr && element->fit_parents_width && element->abs_width != -1)
            {
              /* taking into account that an element can range over multiple columns */
              colSpan = this->getColSpan(element);
              if (element->abs_width / colSpan > colWidth)
                {
                  colWidth = element->abs_width / colSpan;
                }
            }
        }
      colWidths[x] = colWidth;
      if (colWidth == -1)
        {
          numColsWithFlexibleWidth += 1;
        }
      else
        {
          totalWidthLeft -= colWidth;
        }
    }
  if (totalWidthLeft + epsilon < 0)
    {
      throw ContradictingAttributes("Not enough horizontal space for the cols");
    }
  if (numColsWithFlexibleWidth == 0)
    {
      for (x = 0; x < ncols; x++)
        {
          colWidths[x] += totalWidthLeft / ncols;
        }
    }

  /* calculate the plot for each element */
  ymax = plot[3];
  ymin = ymax;
  for (y = 0; y < nrows; y++)
    {
      xmin = plot[0];
      xmax = xmin;

      rowHeight = (rowHeights[y] == -1) ? totalHeightLeft / numRowsWithFlexibleHeight : rowHeights[y];
      ymin -= rowHeight;

      for (x = 0; x < ncols; x++)
        {
          element = getElement(y, x);

          elementWidth = (colWidths[x] == -1) ? totalWidthLeft / numColsWithFlexibleWidth : colWidths[x];
          xmax += elementWidth;

          if (element != nullptr)
            {
              element->setPlot(xmin, xmax, ymin, ymax);
            }
          xmin = xmax;
        }
      ymax = ymin;
    }

  /* call finalize on each element */
  for (y = 0; y < nrows; y++)
    {
      for (x = 0; x < ncols; x++)
        {
          element = getElement(y, x);
          if (element != nullptr)
            {
              element->finalizePlot();
            }
        }
    }
}

void Grid::upsize(int nrows, int ncols)
{
  int i;
  if (ncols > this->ncols)
    {
      for (i = 0; i < this->nrows; ++i)
        {
          rows.at(i).resize(ncols, nullptr);
        }
      this->ncols = ncols;
    }
  if (nrows > this->nrows)
    {
      for (i = this->nrows; i < nrows; ++i)
        {
          std::vector<GridElement *> row(this->ncols, nullptr);
          this->rows.insert(this->rows.end(), row);
        }
      this->nrows = nrows;
    }
}

void Grid::trim()
{
  int row, col;
  bool removeRow, removeCol;
  std::vector<int> colsToRemove;

  /* remove empty rows */
  auto rowIterator = rows.begin();
  while (rowIterator != rows.end())
    {
      removeRow = true;
      for (auto colIterator = rowIterator->begin(); colIterator != rowIterator->end(); ++colIterator)
        {
          if (*colIterator != nullptr)
            {
              removeRow = false;
            }
        }
      if (removeRow)
        {
          rowIterator = rows.erase(rowIterator);
          --(nrows);
        }
      else
        {
          ++rowIterator;
        }
    }

  /* remove empty cols */
  col = 0;
  while (col != ncols)
    {
      removeCol = true;
      for (row = 0; row < nrows; ++row)
        {
          if (getElement(row, col) != nullptr)
            {
              removeCol = false;
            }
        }
      if (removeCol)
        {
          for (row = 0; row < nrows; ++row)
            {
              auto colIterator = rows.at(row).begin();
              rows.at(row).erase(colIterator + col);
            }
          --(ncols);
        }
      else
        {
          ++col;
        }
    }
}

int Grid::getColSpan(GridElement *element)
{
  Slice *slice = elementToPosition.at(element);
  return slice->col_stop - slice->col_start;
}

int Grid::getRowSpan(GridElement *element)
{
  Slice *slice = elementToPosition.at(element);
  return slice->row_stop - slice->row_start;
}

int Grid::getNRows() const
{
  return this->nrows;
}

int Grid::getNCols() const
{
  return this->ncols;
}

void Grid::ensureCellIsGrid(int row, int col)
{
  this->upsize(row + 1, col + 1);
  GridElement *element = this->getElement(row, col);
  if (element == nullptr || !element->isGrid())
    {
      Grid *grid = new Grid(1, 1);
      this->setElement(row, col, grid);
    }
}

void Grid::ensureCellsAreGrid(Slice *slice)
{
  Grid *firstGridFound;
  GridElement *currentElement;
  int row, col;
  int nrowsToAllocate = slice->row_stop;
  int ncolsToAllocate = slice->col_stop;

  this->upsize(nrowsToAllocate, ncolsToAllocate);

  for (row = slice->row_start; row < slice->row_stop; ++row)
    {
      for (col = slice->col_start; col < slice->col_stop; ++col)
        {
          currentElement = this->getElement(row, col);
          if (currentElement != nullptr && currentElement->isGrid())
            {
              firstGridFound = dynamic_cast<Grid *>(currentElement);
              this->setElement(slice, firstGridFound);
              return;
            }
        }
    }

  Grid *grid = new Grid(1, 1);
  this->setElement(slice, grid);
}

bool Grid::isGrid()
{
  return true;
}

bool Grid::isRowsEmpty() const
{
  return this->rows.empty();
}

const std::unordered_map<GridElement *, Slice *> &Grid::getElementToPosition()
{
  return this->elementToPosition;
}

const std::vector<std::vector<GridElement *>> &Grid::getRows()
{
  return this->rows;
}
