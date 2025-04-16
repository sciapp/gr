#include <iostream>
#include <limits>
#include <sstream>
#include "args_int.h"
#include <grm/layout_error.hxx>
#include "grm/layout.hxx"

using namespace GRM;

double epsilon = std::numeric_limits<double>::epsilon();

Slice::Slice(int row_start, int row_stop, int col_start, int col_stop)
    : row_start(row_start), row_stop(row_stop), col_start(col_start), col_stop(col_stop)
{
  if (!this->isPositive()) throw InvalidIndex("Indices must be positive values");
  if (!this->isForward()) throw InvalidIndex("Start value can`t be bigger than stop value");
}

Slice *Slice::copy()
{
  Slice *copy = new Slice(this->row_start, this->row_stop, this->col_start, this->col_stop);
  return copy;
}

bool Slice::isPositive()
{
  return (row_start >= 0 && row_stop >= 0 && col_start >= 0 && col_stop >= 0);
}

bool Slice::isForward()
{
  return (row_start <= row_stop && col_start <= col_stop);
}

GridElement::GridElement()
{
  plot = new double[4]{0.0, 0.0, 0.0, 0.0};
};

GridElement::GridElement(double abs_height, double abs_width, int abs_height_pxl, int abs_width_pxl,
                         int fit_parents_height, int fit_parents_width, double relative_height, double relative_width,
                         double aspect_ratio)
    : abs_height(abs_height), abs_width(abs_width), abs_height_pxl(abs_height_pxl), abs_width_pxl(abs_width_pxl),
      fit_parents_height(fit_parents_height), fit_parents_width(fit_parents_width), relative_height(relative_height),
      relative_width(relative_width), aspect_ratio(aspect_ratio)
{
  setAbsHeight(abs_height);
  setAbsWidth(abs_width);
  setAbsHeightPxl(abs_height_pxl);
  setAbsWidthPxl(abs_width_pxl);
  setRelativeHeight(relative_height);
  setRelativeWidth(relative_width);
  plot = new double[4]{0.0, 0.0, 0.0, 0.0};
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
      if (y1 < plot[2]) plot[2] = y1;
      if (x2 > plot[1]) plot[1] = x2;
    }
}

void GridElement::setAbsHeight(double height)
{
  if (height_set && height != -1) throw ContradictingAttributes("Can only set one height attribute");
  if ((height <= 0 || height > 1) && height != -1)
    throw std::invalid_argument("Height has to be between 0 and 1 or be -1");
  if (ar_set && width_set && height != -1)
    throw ContradictingAttributes("You cant restrict the height on a plot with fixed width and aspect ratio");

  abs_height = height;
  height_set = (height != -1) ? 1 : 0;
}

void GridElement::setAbsHeightPxl(int height)
{
  if (height_set && height != -1) throw ContradictingAttributes("Can only set one height attribute");
  if (height <= 0 && height != -1) throw InvalidArgumentRange("Pixel height has to be an positive integer or be -1");
  if (ar_set && width_set && height != -1)
    throw ContradictingAttributes("You cant restrict the height on a plot with fixed width and aspect ratio");

  abs_height_pxl = height;
  height_set = (height != -1) ? 1 : 0;
}

void GridElement::setRelativeHeight(double height)
{
  if (height_set && height != -1) throw ContradictingAttributes("Can only set one height attribute");
  if ((height <= 0 || height > 1) && height != -1)
    throw InvalidArgumentRange("Height has to be between 0 and 1 or be -1");
  if (ar_set && width_set && height != -1)
    throw ContradictingAttributes("You cant restrict the height on a plot with fixed width and aspect ratio");

  relative_height = height;
  height_set = (height != -1) ? 1 : 0;
}

void GridElement::setAbsWidth(double width)
{
  if (width_set && width != -1) throw ContradictingAttributes("Can only set one width attribute");
  if ((width <= 0 || width > 1) && width != -1) throw InvalidArgumentRange("Width has to be between 0 and 1 or be -1");
  if (ar_set and height_set)
    throw ContradictingAttributes("You cant restrict the width on a plot with fixed height and aspect ratio");

  abs_width = width;
  width_set = (width != -1) ? 1 : 0;
}

void GridElement::setAbsWidthPxl(int width)
{
  if (width_set && width != -1) throw ContradictingAttributes("Can only set one width attribute");
  if (width <= 0 && width != -1) throw InvalidArgumentRange("Pixel Width has to be an positive integer or be -1");
  if (ar_set && height_set && width != -1)
    throw ContradictingAttributes("You cant restrict the width on a plot with fixed height and aspect ratio");

  abs_width_pxl = width;
  width_set = (width != -1) ? 1 : 0;
}

void GridElement::setRelativeWidth(double width)
{
  if (width_set && width != -1) throw ContradictingAttributes("Can only set one width attribute");
  if ((width <= 0 || width > 1) && width != -1) throw InvalidArgumentRange("Width has to be between 0 and 1 or be -1");
  if (ar_set && height_set && width != -1)
    throw ContradictingAttributes("You cant restrict the width on a plot with fixed height and aspect ratio");

  relative_width = width;
  width_set = (width != -1) ? 1 : 0;
}

void GridElement::setAspectRatio(double ar)
{
  if (ar <= 0 && ar != -1) throw InvalidArgumentRange("Aspect ration has to be bigger than 0 or be -1");
  if (width_set && height_set && ar != -1)
    throw ContradictingAttributes("You cant restrict the aspect ratio on a plot with fixed sides");

  aspect_ratio = ar;
  ar_set = (ar != -1) ? 1 : 0;
}

void GridElement::finalizePlot()
{
  if (finalized) return;

  if (abs_height != -1)
    {
      double available_height = plot[3] - plot[2];
      if (abs_height > available_height + epsilon)
        throw ContradictingAttributes("Absolute height is bigger than available height");
      double middle = plot[2] + available_height / 2;
      plot[2] = middle - abs_height / 2;
      plot[3] = middle + abs_height / 2;
    }

  if (abs_width != -1)
    {
      double available_width = plot[1] - plot[0];
      if (abs_width > available_width + epsilon)
        throw ContradictingAttributes("Absolute width is bigger than available width");
      double middle = plot[0] + available_width / 2;
      plot[0] = middle - abs_width / 2;
      plot[1] = middle + abs_width / 2;
    }

  if (relative_height != -1)
    {
      double available_height = plot[3] - plot[2];
      double middle = plot[2] + available_height / 2;
      double new_height = available_height * relative_height;
      plot[2] = middle - new_height / 2;
      plot[3] = middle + new_height / 2;
    }

  if (relative_width != -1)
    {
      double available_width = plot[1] - plot[0];
      double middle = plot[0] + available_width / 2;
      double new_width = available_width * relative_width;
      plot[0] = middle - new_width / 2;
      plot[1] = middle + new_width / 2;
    }

  /* TODO: implement for pxl */

  if (ar_set)
    {
      double current_height = (plot[3] - plot[2]);
      double current_width = (plot[1] - plot[0]);
      double current_ar = current_width / current_height;

      if (current_ar < aspect_ratio)
        {
          double new_height = current_width / aspect_ratio;
          double middle = plot[2] + current_height / 2;
          plot[2] = middle - new_height / 2;
          plot[3] = middle + new_height / 2;
        }
      else
        {
          double new_width = current_height * aspect_ratio;
          double middle = plot[0] + current_width / 2;
          plot[0] = middle - new_width;
          plot[1] = middle + new_width;
        }
    }

  if (plot_args != nullptr) grm_args_push(plot_args, "subplot", "nD", 4, plot);

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

void GridElement::setFitParentsHeight(bool fit_parents_height)
{
  this->fit_parents_height = fit_parents_height;
}

void GridElement::setFitParentsWidth(bool fit_parents_width)
{
  this->fit_parents_width = fit_parents_width;
}

bool GridElement::isGrid()
{
  return false;
}

Grid::Grid(int n_rows, int n_cols) : Grid(n_rows, n_cols, -1, -1, -1, -1, 0, 1, -1, -1, -1) {}

Grid::Grid(int n_rows, int n_cols, double abs_height, double abs_width, int abs_height_pxl, int abs_width_pxl,
           int fit_parents_height, int fit_parents_width, double relative_height, double relative_width,
           double aspect_ratio)
    : GridElement(abs_height, abs_width, abs_height_pxl, abs_width_pxl, fit_parents_height, fit_parents_width,
                  relative_height, relative_width, aspect_ratio),
      n_rows(n_rows), n_cols(n_cols)
{
  if (n_rows < 1 || n_cols < 1)
    throw InvalidArgumentRange("The number of rows and cols in a grid must be bigger than 0");
  for (int i = 0; i < n_rows; ++i)
    {
      std::vector<GridElement *> row(n_cols, nullptr);
      rows.push_back(row);
    }
}

Grid::~Grid()
{
  for (auto const &x : this->element_to_position)
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
  Slice new_slice(row, row + 1, col, col + 1);
  setElement(&new_slice, element);
}

void Grid::setElement(int row, int col, grm_args_t *plot_args)
{
  Slice new_slice(row, row + 1, col, col + 1);
  setElement(&new_slice, plot_args);
}

void Grid::setElement(Slice *slice, GridElement *element)
{
  int nrows_to_allocate, ncols_to_allocate;
  Slice *old_slice;
  std::vector<GridElement *> old_elements;

  nrows_to_allocate = slice->row_stop;
  ncols_to_allocate = slice->col_stop;

  /* Resize the container if necessary */
  upsize(nrows_to_allocate, ncols_to_allocate);

  /* Delete element from grid if it already exists */
  try
    {
      old_slice = element_to_position.at(element);
      for (int row = old_slice->row_start; row < old_slice->row_stop; ++row)
        {
          for (int col = old_slice->col_start; col < old_slice->col_stop; ++col)
            {
              rows.at(row).at(col) = nullptr;
            }
        }
      element_to_position.erase(element);
      delete (old_slice);
    }
  catch (const std::out_of_range &e)
    {
    };

  for (int row = slice->row_start; row < slice->row_stop; ++row)
    {
      for (int col = slice->col_start; col < slice->col_stop; ++col)
        {
          old_elements.push_back(this->getElement(row, col));
          rows.at(row).at(col) = element;
        }
    }
  Slice *new_slice = slice->copy();
  element_to_position[element] = new_slice;

  /* Delete old elements */
  for (auto &old_element : old_elements)
    {
      if (element_to_position.count(old_element) == 0) delete old_element;
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
  for (int i = 0; i < n_rows; i++)
    {
      for (int j = 0; j < n_cols; j++)
        {
          plot = getElement(i, j)->plot;
          printf("[%f %f %f %f] ", plot[0], plot[1], plot[2], plot[3]);
        }
      printf("\n");
    }
}

void Grid::finalizePlot()
{
  double xmin, xmax, ymin, ymax, row_height, element_width;
  int y, x, row_span, col_span;
  GridElement *element;

  if (!plot_set) setPlot(0, 1, 0, 1);

  GridElement::finalizePlot();

  /* calculate height of each row */
  std::vector<double> row_heights(n_rows);
  double total_height_left = plot[3] - plot[2];
  int num_rows_with_flexible_height = 0;
  for (y = 0; y < n_rows; y++)
    {
      double row_height = -1;
      for (x = 0; x < n_cols; x++)
        {
          element = getElement(y, x);
          if (element != nullptr && element->fit_parents_height && element->abs_height != -1)
            {
              /* taking into account that an element can range over multiple rows */
              row_span = this->getRowSpan(element);
              if (element->abs_height / row_span > row_height) row_height = element->abs_height / row_span;
            }
        }
      row_heights[y] = row_height;
      if (row_height == -1)
        {
          num_rows_with_flexible_height += 1;
        }
      else
        {
          total_height_left -= row_height;
        }
    }
  if (total_height_left + epsilon < 0) throw ContradictingAttributes("Not enough vertical space for the rows");
  if (num_rows_with_flexible_height == 0)
    {
      /* distribute the height that is left */
      for (y = 0; y < n_rows; y++)
        {
          row_heights[y] += total_height_left / n_rows;
        }
    }

  /* calculate width of each column */
  double total_width_left = plot[1] - plot[0];
  std::vector<double> col_widths(n_cols);
  int num_cols_with_flexible_width = 0;
  for (x = 0; x < n_cols; x++)
    {
      double col_width = -1;
      for (y = 0; y < n_rows; y++)
        {
          element = getElement(y, x);
          if (element != nullptr && element->fit_parents_width && element->abs_width != -1)
            {
              /* taking into account that an element can range over multiple columns */
              col_span = this->getColSpan(element);
              if (element->abs_width / col_span > col_width) col_width = element->abs_width / col_span;
            }
        }
      col_widths[x] = col_width;
      if (col_width == -1)
        {
          num_cols_with_flexible_width += 1;
        }
      else
        {
          total_width_left -= col_width;
        }
    }
  if (total_width_left + epsilon < 0) throw ContradictingAttributes("Not enough horizontal space for the cols");
  if (num_cols_with_flexible_width == 0)
    {
      for (x = 0; x < n_cols; x++)
        {
          col_widths[x] += total_width_left / n_cols;
        }
    }

  /* calculate the plot for each element */
  ymax = plot[3];
  ymin = ymax;
  for (y = 0; y < n_rows; y++)
    {
      xmin = plot[0];
      xmax = xmin;

      row_height = (row_heights[y] == -1) ? total_height_left / num_rows_with_flexible_height : row_heights[y];
      ymin -= row_height;

      for (x = 0; x < n_cols; x++)
        {
          element = getElement(y, x);

          element_width = (col_widths[x] == -1) ? total_width_left / num_cols_with_flexible_width : col_widths[x];
          xmax += element_width;

          if (element != nullptr) element->setPlot(xmin, xmax, ymin, ymax);
          xmin = xmax;
        }
      ymax = ymin;
    }

  /* call finalize on each element */
  for (y = 0; y < n_rows; y++)
    {
      for (x = 0; x < n_cols; x++)
        {
          element = getElement(y, x);
          if (element != nullptr) element->finalizePlot();
        }
    }
}

void Grid::upsize(int nrows, int ncols)
{
  int i;
  if (ncols > this->n_cols)
    {
      for (i = 0; i < this->n_rows; ++i)
        {
          rows.at(i).resize(ncols, nullptr);
        }
      this->n_cols = ncols;
    }
  if (nrows > this->n_rows)
    {
      for (i = this->n_rows; i < nrows; ++i)
        {
          std::vector<GridElement *> row(this->n_cols, nullptr);
          this->rows.insert(this->rows.end(), row);
        }
      this->n_rows = nrows;
    }
}

void Grid::trim()
{
  int row, col;
  bool remove_row, remove_col;
  std::vector<int> cols_to_remove;

  /* remove empty rows */
  auto row_iterator = rows.begin();
  while (row_iterator != rows.end())
    {
      remove_row = true;
      for (auto col_iterator = row_iterator->begin(); col_iterator != row_iterator->end(); ++col_iterator)
        {
          if (*col_iterator != nullptr) remove_row = false;
        }
      if (remove_row)
        {
          row_iterator = rows.erase(row_iterator);
          --(n_rows);
        }
      else
        {
          ++row_iterator;
        }
    }

  /* remove empty cols */
  col = 0;
  while (col != n_cols)
    {
      remove_col = true;
      for (row = 0; row < n_rows; ++row)
        {
          if (getElement(row, col) != nullptr) remove_col = false;
        }
      if (remove_col)
        {
          for (row = 0; row < n_rows; ++row)
            {
              auto col_iterator = rows.at(row).begin();
              rows.at(row).erase(col_iterator + col);
            }
          --(n_cols);
        }
      else
        {
          ++col;
        }
    }
}

int Grid::getColSpan(GridElement *element)
{
  Slice *slice = element_to_position.at(element);
  return slice->col_stop - slice->col_start;
}

int Grid::getRowSpan(GridElement *element)
{
  Slice *slice = element_to_position.at(element);
  return slice->row_stop - slice->row_start;
}

int Grid::getNRows() const
{
  return this->n_rows;
}

int Grid::getNCols() const
{
  return this->n_cols;
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
  Grid *first_grid_found;
  GridElement *current_element;
  int row, col;
  int nrows_to_allocate = slice->row_stop;
  int ncols_to_allocate = slice->col_stop;

  this->upsize(nrows_to_allocate, ncols_to_allocate);

  for (row = slice->row_start; row < slice->row_stop; ++row)
    {
      for (col = slice->col_start; col < slice->col_stop; ++col)
        {
          current_element = this->getElement(row, col);
          if (current_element != nullptr && current_element->isGrid())
            {
              first_grid_found = dynamic_cast<Grid *>(current_element);
              this->setElement(slice, first_grid_found);
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
  return this->element_to_position;
}

const std::vector<std::vector<GridElement *>> &Grid::getRows()
{
  return this->rows;
}
