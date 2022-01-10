#include <iostream>
#include "layout.hpp"

double epsilon = std::numeric_limits<double>::epsilon();

GridElement::GridElement()
{
  this->subplot = new double[4];
};

void GridElement::setSubplot(double x1, double x2, double y1, double y2)
{
  if (this->finalized || this->subplotSet == 0)
    {
      this->subplot[0] = x1;
      this->subplot[1] = x2;
      this->subplot[2] = y1;
      this->subplot[3] = y2;

      this->finalized = 0;
      this->subplotSet = 1;
    }
  else
    {
      if (y1 < this->subplot[2])
        {
          this->subplot[2] = y1;
        }
      if (x2 > this->subplot[1])
        {
          this->subplot[1] = x2;
        }
    }
}

void GridElement::setAbsHeight(double height)
{
  if (this->heightSet)
    {
      throw std::invalid_argument("Can only set one height attribute");
    }
  if (height <= 0 || height > 1)
    {
      throw std::invalid_argument("Height has to be between 0 and 1");
    }
  if (this->arSet and this->widthSet)
    {
      throw std::invalid_argument("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  this->absHeight = height;
  this->heightSet = 1;
}

void GridElement::setAbsHeightPxl(int height)
{
  if (this->heightSet)
    {
      throw std::invalid_argument("Can only set one height attribute");
    }
  if (height <= 0)
    {
      throw std::invalid_argument("Pixel height has to be an positive integer");
    }
  if (this->arSet and this->widthSet)
    {
      throw std::invalid_argument("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  this->absHeightPxl = height;
  this->heightSet = 1;
}

void GridElement::setRelativeHeight(double height)
{
  if (this->heightSet)
    {
      throw std::invalid_argument("Can only set one height attribute");
    }
  if (height <= 0 || height > 1)
    {
      throw std::invalid_argument("Height has to be between 0 and 1");
    }
  if (this->arSet and this->widthSet)
    {
      throw std::invalid_argument("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  this->relativeHeight = height;
  this->heightSet = 1;
}

void GridElement::setAbsWidth(double width)
{
  if (this->widthSet)
    {
      throw std::invalid_argument("Can only set one width attribute");
    }
  if (width <= 0 || width > 1)
    {
      throw std::invalid_argument("Width has to be between 0 and 1");
    }
  if (this->arSet and this->heightSet)
    {
      throw std::invalid_argument("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  this->absWidth = width;
  this->widthSet = 1;
}

void GridElement::setAbsWidthPxl(int width)
{
  if (this->widthSet)
    {
      throw std::invalid_argument("Can only set one width attribute");
    }
  if (width <= 0)
    {
      throw std::invalid_argument("Pixel Width has to be an positive integer");
    }
  if (this->arSet and this->heightSet)
    {
      throw std::invalid_argument("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  this->absWidthPxl = width;
  this->widthSet = 1;
}

void GridElement::setRelativeWidth(double width)
{
  if (this->widthSet)
    {
      throw std::invalid_argument("Can only set one width attribute");
    }
  if (width <= 0 || width > 1)
    {
      throw std::invalid_argument("Width has to be between 0 and 1");
    }
  if (this->arSet and this->heightSet)
    {
      throw std::invalid_argument("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  this->relativeWidth = width;
  this->widthSet = 1;
}

void GridElement::setAspectRatio(double ar)
{
  if (ar <= 0)
    {
      throw std::invalid_argument("Aspect ration has to be bigger than 0");
    }
  if (this->widthSet && this->heightSet)
    {
      throw std::invalid_argument("You cant restrict the aspect ratio on a plot with fixed sides");
    }
  this->aspectRatio = ar;
  this->arSet = 1;
}

void GridElement::finalizeSubplot()
{
  if (this->finalized)
    {
      return;
    }

  if (this->absHeight != -1)
    {
      double availableHeight = subplot[3] - subplot[2];
      if (this->absHeight > availableHeight + epsilon)
        {
          throw std::invalid_argument("Absolute height is bigger than available height");
        }
      double middle = subplot[2] + availableHeight / 2;
      subplot[2] = middle - this->absHeight / 2;
      subplot[3] = middle + this->absHeight / 2;
    }

  if (this->absWidth != -1)
    {
      double availableWidth = subplot[1] - subplot[0];
      if (this->absWidth > availableWidth + epsilon)
        {
          throw std::invalid_argument("Absolute width is bigger than available width");
        }
      double middle = subplot[0] + availableWidth / 2;
      subplot[0] = middle - this->absWidth / 2;
      subplot[1] = middle + this->absWidth / 2;
    }

  if (this->relativeHeight != -1)
    {
      double availableHeight = subplot[3] - subplot[2];
      double middle = subplot[2] + availableHeight / 2;
      double newHeight = availableHeight * this->relativeHeight;
      subplot[2] = middle - newHeight / 2;
      subplot[3] = middle + newHeight / 2;
    }

  if (this->relativeWidth != -1)
    {
      double availableWidth = subplot[1] - subplot[0];
      double middle = subplot[0] + availableWidth / 2;
      double newWidth = availableWidth * this->relativeWidth;
      subplot[0] = middle - newWidth / 2;
      subplot[1] = middle + newWidth / 2;
    }

  /* TODO: implement for pxl */

  if (this->arSet)
    {
      double currentHeigth = (subplot[3] - subplot[2]);
      double currentWidth = (subplot[1] - subplot[0]);
      double currentAR = currentWidth / currentHeigth;

      if (currentAR < this->aspectRatio)
        {
          double newHeight = currentWidth / this->aspectRatio;
          double middle = subplot[2] + currentHeigth / 2;
          subplot[2] = middle - newHeight / 2;
          subplot[3] = middle + newHeight / 2;
        }
      else
        {
          double newWidth = currentHeigth * this->aspectRatio;
          double middle = subplot[0] + currentWidth / 2;
          subplot[0] = middle - newWidth;
          subplot[1] = middle + newWidth;
        }
    }

  this->finalized = 1;
}

double *GridElement::getSubplot()
{
  return this->subplot;
}

void GridElement::setFitParentsHeight(bool fitParentsHeight)
{
  this->fitParentsHeight = fitParentsHeight;
}

void GridElement::setFitParentsWidth(bool fitParentsWidth)
{
  this->fitParentsWidth = fitParentsWidth;
}

Grid::Grid(int nrows, int ncols) : GridElement(), nrows(nrows), ncols(ncols), elements(nrows * ncols, nullptr) {}

Grid::~Grid() {}

GridElement *Grid::getElement(int row, int col) const
{
  return this->elements[row * (this->ncols) + col];
};

void Grid::setElement(int row, int col, GridElement *element)
{
  this->elements[row * (this->ncols) + col] = element;
}

void Grid::printGrid() const
{
  double *subplot;
  for (int i = 0; i < this->nrows; i++)
    {
      for (int j = 0; j < this->ncols; j++)
        {
          subplot = this->getElement(i, j)->subplot;
          printf("[%f %f %f %f] ", subplot[0], subplot[1], subplot[2], subplot[3]);
        }
      printf("\n");
    }
}

void Grid::finalizeSubplot()
{
  double xmin, xmax, ymin, ymax, rowHeight, elementWidth;
  int y, x;
  GridElement *element;

  if (!this->subplotSet)
    {
      this->setSubplot(0, 1, 0, 1);
    }

  GridElement::finalizeSubplot();

  /* calculate height of each row */
  double rowHeights[this->nrows];
  double totalHeightLeft = this->subplot[3] - this->subplot[2];
  int numRowsWithFlexibleHeight = 0;
  for (y = 0; y < this->nrows; y++)
    {
      double rowHeight = -1;
      for (x = 0; x < this->ncols; x++)
        {
          element = getElement(y, x);
          if (element->fitParentsHeight && element->absHeight != -1)
            {
              if (element->absHeight > rowHeight)
                {
                  rowHeight = element->absHeight;
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
      throw std::invalid_argument("Not enough vertical space for the rows");
    }
  if (numRowsWithFlexibleHeight == 0)
    {
      /* distribute the height that is left */
      for (y = 0; y < this->nrows; y++)
        {
          rowHeights[y] += totalHeightLeft / this->nrows;
        }
    }

  /* calculate width of each column */
  double totalWidthLeft = this->subplot[1] - this->subplot[0];
  double colWidths[this->ncols];
  int numColsWithFlexibleWidth = 0;
  for (x = 0; x < this->ncols; x++)
    {
      double colWidth = -1;
      for (y = 0; y < this->nrows; y++)
        {
          element = getElement(y, x);
          if (element->fitParentsWidth && element->absWidth != -1)
            {
              if (element->absWidth > colWidth)
                {
                  colWidth = element->absWidth;
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
      throw std::invalid_argument("Not enough horizontal space for the cols");
    }
  if (numColsWithFlexibleWidth == 0)
    {
      for (x = 0; x < this->ncols; x++)
        {
          colWidths[x] = totalWidthLeft / this->ncols;
        }
    }

  /* calculate the subplot for each element */
  ymax = this->subplot[3];
  ymin = ymax;
  for (y = 0; y < this->nrows; y++)
    {
      xmin = this->subplot[0];
      xmax = xmin;

      rowHeight = (rowHeights[y] == -1) ? totalHeightLeft / numRowsWithFlexibleHeight : rowHeights[y];
      ymin -= rowHeight;

      for (x = 0; x < this->ncols; x++)
        {
          element = getElement(y, x);

          elementWidth = (colWidths[x] == -1) ? totalWidthLeft / numColsWithFlexibleWidth : colWidths[x];
          xmax += elementWidth;

          element->setSubplot(xmin, xmax, ymin, ymax);
          xmin = xmax;
        }
      ymax = ymin;
    }

  /* call finalize on each element */
  for (y = 0; y < this->nrows; y++)
    {
      for (x = 0; x < this->ncols; x++)
        {
          element = getElement(y, x);
          element->finalizeSubplot();
        }
    }
}
