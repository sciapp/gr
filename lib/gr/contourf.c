#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "gr.h"
#include "contour.h"

#ifndef NAN
#define NAN (0.0/0.0)
#endif

#define DEFAULT_CONTOUR_LINES 16 /* default number of contour lines */

const unsigned char EDGE_N = 1 << 0;
const unsigned char EDGE_E = 1 << 1;
const unsigned char EDGE_S = 1 << 2;
const unsigned char EDGE_W = 1 << 3;

typedef struct {
  void *list;
  size_t size;
  size_t _capacity;
  size_t _element_size;
} _list_t;

_list_t *list_create(size_t initial_capacity, size_t element_size)
{
  _list_t *list = (_list_t*)malloc(sizeof(_list_t));
  assert(list);
  list->list = malloc(initial_capacity*element_size);
  assert(list->list);
  list->size = 0;
  list->_capacity = initial_capacity;
  list->_element_size = element_size;
  return list;
}

void list_append(_list_t *list, const void *data)
{
  if (list->size + 1 >= list->_capacity)
    {
      list->list = realloc(list->list, 2*list->_capacity*list->_element_size);
      assert(list->list);
      list->_capacity *= 2;
    }
  memcpy(list->list + list->size * list->_element_size, data, list->_element_size);
  list->size++;
}

void *list_get(const _list_t *list, size_t ind)
{
  if (ind > list->size)
    {
      return NULL;
    }
  return list->list + list->_element_size * ind;
}

void list_destroy(_list_t* list)
{
  if (list->list)
    {
      free(list->list);
    }
  free(list);
}

double padded_array_lookup(const double *z, size_t nx, size_t ny, long i, long j)
{
  /*
   * This function returns the value of an Array z with the shape nx*ny at the position i, j.
   * If (i, j) is 1 cell outside of z the value at the border is repeated. If (i, j) is more
   * than 1 cell outside of z, NAN is returned.
   */
  i-=2;
  j-=2;
  if (i==-1)
    {
      i++;
    }
  else if (i == nx)
    {
      i--;
    }
  if (j==-1)
    {
      j++;
    }
  else if (j == ny)
    {
      j--;
    }
  if (i < 0 || j < 0 || i >= nx || j >= ny)
    {
      return NAN;
    }
  return z[j*nx+i];
}

double padded_array_lookup_1d(const double *z, size_t nx, long i)
{
  /*
   * This function performs a safe lookup in a 1 dimensional array z with the length nx.
   * If the index i is outside of z the first / last value is returned.
   */
  i-=2;
  if (i<0)
    {
      i=0;
    }
  else if (i >= nx)
    {
      i=nx-1;
    }
  return z[i];
}

unsigned char get_bitmask(const double *z, size_t nx, size_t ny, long i, long j, double contour)
{
  /*
   * Calculate the bitmask for cell (i, j) of z and contour used for the marching squares algorithm.
   */
  unsigned char result = 0;
  result |= (padded_array_lookup(z, nx, ny, i  , j  ) > contour) << 3;
  result |= (padded_array_lookup(z, nx, ny, i+1, j  ) > contour) << 2;
  result |= (padded_array_lookup(z, nx, ny, i+1, j+1) > contour) << 1;
  result |= (padded_array_lookup(z, nx, ny, i  , j+1) > contour);
  return result;
}

double interpolate(double v1, double v2, double contour)
{
  double d = v2 - v1;
  if (d == 0)
    {
      return 0;
    }
  double interp = (contour - v1) / d;
  if (interp < 0)
    {
      interp *= -1;
    }
  if (interp > 1)
    {
      interp = 1;
    }
  return interp;
}

double interpolate_edge(const double *z, size_t nx, size_t ny, long i1, long i2, long j1, long j2, double contour)
{
  /*
   * Linear interpolation along the edge of a cell. Return 0 if one point is outside of z.
   */
  if (i1 < 2 || i2 < 2 || j1 < 2 || j2 < 2)
    {
      return 0;
    }
  if (i1 > nx+1 || i2 > nx+1 || j1 > ny+1 || j2 > ny+1)
    {
      return 0;
    }
  return interpolate(padded_array_lookup(z, nx, ny, i1, j1), padded_array_lookup(z, nx, ny, i2, j2), contour);
}

void marching_squares(const double *x, const double *y, const double *z, size_t nx, size_t ny,
                      const double *contours, size_t nc, int first_color, int last_color)
{
  /*
   * Calculate and fill / draw contours using the marching squares algorithm.
   *
   * In this implementation the array z is padded twice. 1 cell outside of z the border value
   * is repeated and 2 cells outside of z NAN. This assures that contour lines that cross the
   * border of z are also closed (outside of z).
   */
  double x_step = x[1] - x[0];
  double y_step = y[1] - y[0];

  double x_pos, y_pos;
  long i, j, fillarea_start_index;
  size_t contour_index;
  double color_step = 0;
  if (nc > 0)
    {
      color_step = 1.0 * (last_color - first_color) / (nc - 1);
    }

  /* Create list structures to store the polyline's vertices and the start index of each individual polyline */
  _list_t *polylines_x = list_create(nx*ny/4, sizeof(double));
  _list_t *polylines_y = list_create(nx*ny/4, sizeof(double));
  _list_t *line_indices = list_create(100, sizeof(size_t));

  size_t nx_padded = nx + 4;
  size_t ny_padded = ny + 4;
  unsigned char *edges = calloc(nx_padded*ny_padded, sizeof(unsigned char));
  assert(edges);

  gr_setfillintstyle(1);
  for (contour_index=0; contour_index<nc; contour_index++)
    {
      double contour = contours[contour_index];
      fillarea_start_index = polylines_x->size;

      /*
       * Calculate the binary index of the marching squares algorithm for each cell of the padded z array\
       * and store it in `edges`.
       */
      for (j=0; j<ny_padded; j++)
        {
          for (i=0; i<nx_padded; i++)
            {
              unsigned char bitmask = get_bitmask(z, nx, ny, i, j, contour);
              if (bitmask == 1 || bitmask == 10 || bitmask == 14)    /* bottom \ */
                {
                  edges[j*nx_padded+i] |= EDGE_W | EDGE_S;
                }
              if (bitmask == 2 || bitmask == 5 || bitmask == 13)     /* bottom / */
                {
                  edges[j*nx_padded+i] |= EDGE_E | EDGE_S;
                }
              if (bitmask == 3 || bitmask == 12)                    /*         - */
                {
                  edges[j*nx_padded+i] |= EDGE_W | EDGE_E;
                }
              if (bitmask == 6 || bitmask == 9)                     /*         | */
                {
                  edges[j*nx_padded+i] |= EDGE_N | EDGE_S;
                }
              if (bitmask == 7 || bitmask == 8 || bitmask == 5)     /* top     / */
                {
                  edges[j*nx_padded+i] |= EDGE_N | EDGE_W;
                }
              if (bitmask == 4 || bitmask == 10 || bitmask == 11)   /* top     \ */
                {
                  edges[j*nx_padded+i] |= EDGE_N | EDGE_E;
                }
            }
        }

      /* Find and follow connected polylines */
      for (j=0; j<ny_padded; j++)
        {
          for (i=0; i<nx_padded; i++)
            {
              if (edges[j*nx_padded+i]) /* Start of a new polyline found */
                {
                  long xi = i;
                  long yi = j;

                  /* Start and end all lines at (0, 0) to avoid connecting separate regions while filling */
                  x_pos = y_pos = 0;
                  list_append(polylines_x, &x_pos);
                  list_append(polylines_y, &y_pos);
                  size_t polyline_start_index = polylines_x->size;
                  list_append(line_indices, &polyline_start_index);

                  /*
                   * Follow polyline until start point is reached again. When adding a line segment
                   * to the polyline, remove the corresponding EDGE_* bit from the cell and the
                   * corresponding bit of the following cell in the line.
                   */
                  while(edges[yi*nx_padded+xi])
                    {
                      if (edges[yi*nx_padded+xi] & EDGE_N)
                        {
                          x_pos = padded_array_lookup_1d(x, nx, xi) + \
                                  x_step * interpolate_edge(z, nx, ny, xi, xi + 1, yi, yi, contour);
                          y_pos = padded_array_lookup_1d(y, ny, yi);
                          edges[yi*nx_padded+xi] &= ~EDGE_N;
                          yi--;
                          assert(edges[yi*nx_padded+xi] | EDGE_S);
                          edges[yi*nx_padded+xi] &= ~EDGE_S;
                        }
                      else if (edges[yi*nx_padded+xi] & EDGE_E)
                        {
                          x_pos = padded_array_lookup_1d(x, nx, xi + 1);
                          y_pos = padded_array_lookup_1d(y, ny, yi) + \
                                  y_step * interpolate_edge(z, nx, ny, xi + 1, xi + 1, yi, yi + 1, contour);
                          edges[yi*nx_padded+xi] &= ~EDGE_E;
                          xi++;
                          assert(edges[yi*nx_padded+xi] | EDGE_W);
                          edges[yi*nx_padded+xi] &= ~EDGE_W;
                        }
                      else if (edges[yi*nx_padded+xi] & EDGE_S)
                        {
                          x_pos = padded_array_lookup_1d(x, nx, xi) + \
                                  x_step * interpolate_edge(z, nx, ny, xi, xi + 1, yi + 1, yi + 1, contour);
                          y_pos = padded_array_lookup_1d(y, ny, yi + 1);
                          edges[yi*nx_padded+xi] &= ~EDGE_S;
                          yi++;
                          assert(edges[yi*nx_padded+xi] | EDGE_N);
                          edges[yi*nx_padded+xi] &= ~EDGE_N;
                        }
                      else if (edges[yi*nx_padded+xi] & EDGE_W)
                        {
                          x_pos = padded_array_lookup_1d(x, nx, xi);
                          y_pos = padded_array_lookup_1d(y, ny, yi) + \
                                  y_step * interpolate_edge(z, nx, ny, xi, xi, yi, yi + 1, contour);
                          edges[yi*nx_padded+xi] &= ~EDGE_W;
                          xi--;
                          assert(edges[yi*nx_padded+xi] | EDGE_E);
                          edges[yi*nx_padded+xi] &= ~EDGE_E;
                        }
                      list_append(polylines_x, &x_pos);
                      list_append(polylines_y, &y_pos);
                    }
                  assert(xi == i && yi == j);
                  /* Repeat first polyline point to get a closed line */
                  list_append(polylines_x, ((double *)polylines_x->list)+polyline_start_index);
                  list_append(polylines_y, ((double *)polylines_y->list)+polyline_start_index);
                  x_pos = y_pos = 0;
                  list_append(polylines_x, &x_pos);
                  list_append(polylines_y, &y_pos);
                  break;
                }
            }
        }
      /* Fill all areas for the current contour. Filling must use Even-Odd-Rule. */
      long n = polylines_x->size-fillarea_start_index;
      if (n > 2)
        {
          gr_setfillcolorind(first_color+(int)floor(color_step*contour_index));
          gr_fillarea(n, (double *)list_get(polylines_x, fillarea_start_index),
                      (double *)list_get(polylines_y, fillarea_start_index));
        }
    }
  free(edges);

  long num_lines = line_indices->size;

  size_t polylines_end_indices = polylines_x->size+1;
  list_append(line_indices, &(polylines_end_indices));
  size_t *line_ind = (size_t *)line_indices->list;

  /* Draw contour lines for all `contour` values */
  for (i=0; i<num_lines; i++)
    {
      long n = line_ind[i+1] - line_ind[i] - 2; /* Remove (0, 0) points which are required for filling from polyline. */
      if (n >= 2)
        {
          gr_polyline(n, (double *)list_get(polylines_x, line_ind[i]), (double *)list_get(polylines_y, line_ind[i]));
        }
    }
  list_destroy(polylines_x);
  list_destroy(polylines_y);
  list_destroy(line_indices);
}

void gr_draw_contourf(int nx, int ny, int nh, double *px, double *py, double *h,
                      double *pz, int first_color, int last_color)
{
  double zmin, zmax;
  int i;
  double *contours;
  if (nh < 1)
    {
      nh = DEFAULT_CONTOUR_LINES;
      h = NULL;
    }
  if (h == NULL)
    {
      zmin = zmax = pz[0];
      for (i=0; i<nx*ny; i++)
        {
          if (pz[i] < zmin)
            {
              zmin = pz[i];
            }
          if (pz[i] > zmax)
            {
              zmax = pz[i];
            }
        }
      contours = (double *)malloc(nh * sizeof(double));
      for (i=0; i<nh; i++)
      {
        contours[i] = zmin + (zmax - zmin) * 1.0 / (nh - 1) * i;
      }
      assert(contours);
      marching_squares(px, py, pz, nx, ny, contours, nh, first_color, last_color);
      free(contours);
    }
  else
    {
      marching_squares(px, py, pz, nx, ny, h, nh, first_color, last_color);
    }
}