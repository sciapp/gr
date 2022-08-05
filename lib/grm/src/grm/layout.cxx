#include <iostream>
#include <limits>
#include <sstream>
#include "args_int.h"
#include <grm/layout.hxx>
#include <grm/layout_error.hxx>

using namespace grm;

double epsilon = std::numeric_limits<double>::epsilon();

Slice::Slice(int rowStart, int rowStop, int colStart, int colStop)
    : rowStart(rowStart), rowStop(rowStop), colStart(colStart), colStop(colStop)
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
  Slice *copy = new Slice(this->rowStart, this->rowStop, this->colStart, this->colStop);
  return copy;
}

bool Slice::isPositive()
{
  if (rowStart < 0 || rowStop < 0 || colStart < 0 || colStop < 0)
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
  if (rowStart > rowStop || colStart > colStop)
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
  subplot = new double[4];
};

void GridElement::setSubplot(double x1, double x2, double y1, double y2)
{
  if (finalized || subplotSet == 0)
    {
      subplot[0] = x1;
      subplot[1] = x2;
      subplot[2] = y1;
      subplot[3] = y2;

      finalized = 0;
      subplotSet = 1;
    }
  else
    {
      if (y1 < subplot[2])
        {
          subplot[2] = y1;
        }
      if (x2 > subplot[1])
        {
          subplot[1] = x2;
        }
    }
}

void GridElement::setAbsHeight(double height)
{
  if (heightSet)
    {
      throw ContradictingAttributes("Can only set one height attribute");
    }
  if (height <= 0 || height > 1)
    {
      throw std::invalid_argument("Height has to be between 0 and 1");
    }
  if (arSet and widthSet)
    {
      throw ContradictingAttributes("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  absHeight = height;
  heightSet = 1;
}

void GridElement::setAbsHeightPxl(int height)
{
  if (heightSet)
    {
      throw ContradictingAttributes("Can only set one height attribute");
    }
  if (height <= 0)
    {
      throw InvalidArgumentRange("Pixel height has to be an positive integer");
    }
  if (arSet and widthSet)
    {
      throw ContradictingAttributes("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  absHeightPxl = height;
  heightSet = 1;
}

void GridElement::setRelativeHeight(double height)
{
  if (heightSet)
    {
      throw ContradictingAttributes("Can only set one height attribute");
    }
  if (height <= 0 || height > 1)
    {
      throw InvalidArgumentRange("Height has to be between 0 and 1");
    }
  if (arSet and widthSet)
    {
      throw ContradictingAttributes("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  relativeHeight = height;
  heightSet = 1;
}

void GridElement::setAbsWidth(double width)
{
  if (widthSet)
    {
      throw ContradictingAttributes("Can only set one width attribute");
    }
  if (width <= 0 || width > 1)
    {
      throw InvalidArgumentRange("Width has to be between 0 and 1");
    }
  if (arSet and heightSet)
    {
      throw ContradictingAttributes("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  absWidth = width;
  widthSet = 1;
}

void GridElement::setAbsWidthPxl(int width)
{
  if (widthSet)
    {
      throw ContradictingAttributes("Can only set one width attribute");
    }
  if (width <= 0)
    {
      throw InvalidArgumentRange("Pixel Width has to be an positive integer");
    }
  if (arSet and heightSet)
    {
      throw ContradictingAttributes("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  absWidthPxl = width;
  widthSet = 1;
}

void GridElement::setRelativeWidth(double width)
{
  if (widthSet)
    {
      throw ContradictingAttributes("Can only set one width attribute");
    }
  if (width <= 0 || width > 1)
    {
      throw InvalidArgumentRange("Width has to be between 0 and 1");
    }
  if (arSet and heightSet)
    {
      throw ContradictingAttributes("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  relativeWidth = width;
  widthSet = 1;
}

void GridElement::setAspectRatio(double ar)
{
  if (ar <= 0)
    {
      throw InvalidArgumentRange("Aspect ration has to be bigger than 0");
    }
  if (widthSet && heightSet)
    {
      throw ContradictingAttributes("You cant restrict the aspect ratio on a plot with fixed sides");
    }
  aspectRatio = ar;
  arSet = 1;
}

void GridElement::finalizeSubplot()
{
  if (finalized)
    {
      return;
    }

  if (absHeight != -1)
    {
      double availableHeight = subplot[3] - subplot[2];
      if (absHeight > availableHeight + epsilon)
        {
          throw ContradictingAttributes("Absolute height is bigger than available height");
        }
      double middle = subplot[2] + availableHeight / 2;
      subplot[2] = middle - absHeight / 2;
      subplot[3] = middle + absHeight / 2;
    }

  if (absWidth != -1)
    {
      double availableWidth = subplot[1] - subplot[0];
      if (absWidth > availableWidth + epsilon)
        {
          throw ContradictingAttributes("Absolute width is bigger than available width");
        }
      double middle = subplot[0] + availableWidth / 2;
      subplot[0] = middle - absWidth / 2;
      subplot[1] = middle + absWidth / 2;
    }

  if (relativeHeight != -1)
    {
      double availableHeight = subplot[3] - subplot[2];
      double middle = subplot[2] + availableHeight / 2;
      double newHeight = availableHeight * relativeHeight;
      subplot[2] = middle - newHeight / 2;
      subplot[3] = middle + newHeight / 2;
    }

  if (relativeWidth != -1)
    {
      double availableWidth = subplot[1] - subplot[0];
      double middle = subplot[0] + availableWidth / 2;
      double newWidth = availableWidth * relativeWidth;
      subplot[0] = middle - newWidth / 2;
      subplot[1] = middle + newWidth / 2;
    }

  /* TODO: implement for pxl */

  if (arSet)
    {
      double currentHeigth = (subplot[3] - subplot[2]);
      double currentWidth = (subplot[1] - subplot[0]);
      double currentAR = currentWidth / currentHeigth;

      if (currentAR < aspectRatio)
        {
          double newHeight = currentWidth / aspectRatio;
          double middle = subplot[2] + currentHeigth / 2;
          subplot[2] = middle - newHeight / 2;
          subplot[3] = middle + newHeight / 2;
        }
      else
        {
          double newWidth = currentHeigth * aspectRatio;
          double middle = subplot[0] + currentWidth / 2;
          subplot[0] = middle - newWidth;
          subplot[1] = middle + newWidth;
        }
    }

  if (subplot_args != nullptr)
    {
      grm_args_push(subplot_args, "subplot", "nD", 4, subplot);
    }

  finalized = 1;
}

double *GridElement::getSubplot()
{
  return subplot;
}

void GridElement::setFitParentsHeight(bool fitParentsHeight)
{
  this->fitParentsHeight = fitParentsHeight;
}

void GridElement::setFitParentsWidth(bool fitParentsWidth)
{
  this->fitParentsWidth = fitParentsWidth;
}

bool GridElement::isGrid()
{
  return false;
}

Grid::Grid(int nrows, int ncols) : GridElement(), nrows(nrows), ncols(ncols)
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

void Grid::setElement(int row, int col, grm_args_t *subplot_args)
{
  Slice newSlice(row, row + 1, col, col + 1);
  setElement(&newSlice, subplot_args);
}

void Grid::setElement(Slice *slice, GridElement *element)
{
  int nrowsToAllocate, ncolsToAllocate;
  Slice *oldSlice;
  std::vector<GridElement *> oldElements;

  nrowsToAllocate = slice->rowStop;
  ncolsToAllocate = slice->colStop;

  /* Resize the container if necessary */
  upsize(nrowsToAllocate, ncolsToAllocate);

  /* Delete element from grid if it already exists */
  try
    {
      oldSlice = elementToPosition.at(element);
      for (int row = oldSlice->rowStart; row < oldSlice->rowStop; ++row)
        {
          for (int col = oldSlice->colStart; col < oldSlice->colStop; ++col)
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

  for (int row = slice->rowStart; row < slice->rowStop; ++row)
    {
      for (int col = slice->colStart; col < slice->colStop; ++col)
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

void Grid::setElement(Slice *slice, grm_args_t *subplot_args)
{
  GridElement *element = nullptr;
  const char *grid_element_address = nullptr;
  if (grm_args_values(subplot_args, "grid_element", "s", &grid_element_address))
    {
      element = reinterpret_cast<GridElement *>(std::stoi(grid_element_address));
    }
  else
    {
      element = new GridElement();
      element->subplot_args = subplot_args;
    }
  std::stringstream address_stream;
  address_stream << element;
  grm_args_push(subplot_args, "grid_element", "s", address_stream.str().c_str());
  setElement(slice, element);
}

void Grid::printGrid() const
{
  double *subplot;
  for (int i = 0; i < nrows; i++)
    {
      for (int j = 0; j < ncols; j++)
        {
          subplot = getElement(i, j)->subplot;
          printf("[%f %f %f %f] ", subplot[0], subplot[1], subplot[2], subplot[3]);
        }
      printf("\n");
    }
}

void Grid::finalizeSubplot()
{
  double xmin, xmax, ymin, ymax, rowHeight, elementWidth;
  int y, x, rowSpan, colSpan;
  GridElement *element;

  if (!subplotSet)
    {
      setSubplot(0, 1, 0, 1);
    }

  GridElement::finalizeSubplot();

  /* calculate height of each row */
  std::vector<double> rowHeights(nrows);
  double totalHeightLeft = subplot[3] - subplot[2];
  int numRowsWithFlexibleHeight = 0;
  for (y = 0; y < nrows; y++)
    {
      double rowHeight = -1;
      for (x = 0; x < ncols; x++)
        {
          element = getElement(y, x);
          if (element != nullptr && element->fitParentsHeight && element->absHeight != -1)
            {
              /* taking into account that an element can range over multiple rows */
              rowSpan = this->getRowSpan(element);
              if (element->absHeight / rowSpan > rowHeight)
                {
                  rowHeight = element->absHeight / rowSpan;
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
  double totalWidthLeft = subplot[1] - subplot[0];
  std::vector<double> colWidths(ncols);
  int numColsWithFlexibleWidth = 0;
  for (x = 0; x < ncols; x++)
    {
      double colWidth = -1;
      for (y = 0; y < nrows; y++)
        {
          element = getElement(y, x);
          if (element != nullptr && element->fitParentsWidth && element->absWidth != -1)
            {
              /* taking into account that an element can range over multiple columns */
              colSpan = this->getColSpan(element);
              if (element->absWidth / colSpan > colWidth)
                {
                  colWidth = element->absWidth / colSpan;
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

  /* calculate the subplot for each element */
  ymax = subplot[3];
  ymin = ymax;
  for (y = 0; y < nrows; y++)
    {
      xmin = subplot[0];
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
              element->setSubplot(xmin, xmax, ymin, ymax);
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
              element->finalizeSubplot();
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
  return slice->colStop - slice->colStart;
}

int Grid::getRowSpan(GridElement *element)
{
  Slice *slice = elementToPosition.at(element);
  return slice->rowStop - slice->rowStart;
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
  int nrowsToAllocate = slice->rowStop;
  int ncolsToAllocate = slice->colStop;

  this->upsize(nrowsToAllocate, ncolsToAllocate);

  for (row = slice->rowStart; row < slice->rowStop; ++row)
    {
      for (col = slice->colStart; col < slice->colStop; ++col)
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
