#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <inttypes.h>

#include "boundary.h"

#define FIND_BOUNDARY_BALL_TOO_SMALL -1
#define FIND_BOUNDARY_BALL_TOO_LARGE -2
#define FIND_BOUNDARY_MEMORY_EXCEEDED -3
#define FIND_BOUNDARY_INVALID_POINTS -4

typedef struct
{
  double x;
  double y;
  int index;
} double2;

typedef struct
{
  int x;
  int y;
} int2;

typedef struct
{
  double2 *points;
  int2 num_cells;
  double cell_size;
  int *cell_offsets;
  double2 bounding_box[2];
} grid;

typedef struct
{
  int current_index;
  double2 current_point;
  double2 center_point;
  double r;
  void *user_data;
} grid_callback_data;

typedef struct
{
  int current;
  size_t size;
  size_t num_points_reachable;
  size_t capacity;
  int *point_list;
} neighbor_point_list;

static grid *current_grid = NULL; /* grid pointer used in compare callback of qsort */

static void calculate_bounding_box(int n, const double2 *points, double2 *min, double2 *max)
{
  /*
   * Calculate the smallest box containing all points. A small offset is subtracted from the bottom left
   * point to assure that the closest data point has a positive distance to the point min.
   */
  int i;
  min->x = max->x = points[0].x;
  min->y = max->y = points[0].y;
  for (i = 1; i < n; i++)
    {
      if (points[i].x < min->x)
        {
          min->x = points[i].x;
        }
      else if (points[i].x > max->x)
        {
          max->x = points[i].x;
        }
      if (points[i].y < min->y)
        {
          min->y = points[i].y;
        }
      else if (points[i].y > max->y)
        {
          max->y = points[i].y;
        }
    }
  min->x -= 0.0001;
  min->y -= 0.0001;
}

static double distance_sq(double2 position1, double2 position2)
{
  /*
   * Calculate the square of the distance between point1 and point2.
   */
  double dx = position1.x - position2.x;
  double dy = position1.y - position2.y;
  return dx * dx + dy * dy;
}

static int2 grid_get_cell(grid *grid, const double2 *position)
{
  /*
   * Calculate the cell index (row / column) of the cell containing the coordinate position.
   * Row and column indices are both clipped to [0, num_cells).
   */
  int2 result;
  result.x = (int)((position->x - grid->bounding_box[0].x) / (grid->cell_size));
  result.y = (int)((position->y - grid->bounding_box[0].y) / (grid->cell_size));
  if (result.x < 0)
    {
      result.x = 0;
    }
  else if (result.x >= grid->num_cells.x)
    {
      result.x = grid->num_cells.x - 1;
    }
  if (result.y < 0)
    {
      result.y = 0;
    }
  else if (result.y >= grid->num_cells.y)
    {
      result.y = grid->num_cells.y - 1;
    }
  return result;
}

static int grid_get_cell_number(grid *grid, const double2 *position)
{
  /*
   * Calculate the consecutive cell index of the cell containing the coordinate position.
   */
  int2 c = grid_get_cell(grid, position);
  return c.x + c.y * grid->num_cells.x;
}

static double2 grid_get_elem(grid *grid, int k)
{
  return grid->points[k];
}

static int compare_element_fun(const void *position1, const void *position2)
{
  /*
   * Comparator function to sort positions by their cell index using qsort function.
   */
  return grid_get_cell_number(current_grid, (const double2 *)position1) >
         grid_get_cell_number(current_grid, (const double2 *)position2);
}

static grid *grid_create(int n, double2 *points, double cell_size)
{
  /*
   * Create a grid data structure containing n points in square cells of the size `cell_size`.
   * The cells are sorted by their cell number internally, so that the points of each cell are
   * in a continuous part of the `points` array.
   */
  int i;
  int cur_cell = 0;

  grid *grid_ptr;
  assert(n > 0);
  grid_ptr = malloc(sizeof(grid));
  assert(grid_ptr);

  calculate_bounding_box(n, points, grid_ptr->bounding_box, grid_ptr->bounding_box + 1);
  grid_ptr->num_cells.x = (int)((grid_ptr->bounding_box[1].x - grid_ptr->bounding_box[0].x) / cell_size) + 1;
  grid_ptr->num_cells.y = (int)((grid_ptr->bounding_box[1].y - grid_ptr->bounding_box[0].y) / cell_size) + 1;
  grid_ptr->cell_offsets = malloc((grid_ptr->num_cells.x * grid_ptr->num_cells.y + 1) * sizeof(int));
  assert(grid_ptr->cell_offsets);

  grid_ptr->points = points;
  grid_ptr->cell_size = cell_size;

  assert(current_grid == NULL);
  current_grid = grid_ptr;
  qsort(points, (size_t)n, sizeof(double2), compare_element_fun);
  current_grid = NULL;

  for (i = 0; i < n; i++)
    {
      while (grid_get_cell_number(grid_ptr, points + i) >= cur_cell)
        {
          grid_ptr->cell_offsets[cur_cell++] = i;
        }
    }
  while (cur_cell <= grid_ptr->num_cells.x * grid_ptr->num_cells.y)
    {
      grid_ptr->cell_offsets[cur_cell++] = n;
    }
  return grid_ptr;
}

static void grid_apply_function(grid *grid_ptr, double2 center, double radius,
                                int (*cb_fun)(grid *, grid_callback_data), void *user_data, int n_excluded,
                                const int *excluded_indices)
{
  /*
   * Apply a callback function `cb_fun` to all points with a distance less than `radius` to the point `center`
   * except for those that are contained in `excluded_indices`. The information about the search position and the
   * current point are passed to the callback function using the `grid_callback_data` struct.
   * Returns if all points are processed or the callback function returns a non-zero value.
   */
  int i, j, k, l;
  double2 top_right_position, bottom_left_position;
  int2 top_right_cell, bottom_left_cell;
  grid_callback_data callback_data;
  top_right_position.x = center.x + radius;
  top_right_position.y = center.y + radius;
  bottom_left_position.x = center.x - radius;
  bottom_left_position.y = center.y - radius;
  top_right_cell = grid_get_cell(grid_ptr, &top_right_position);
  bottom_left_cell = grid_get_cell(grid_ptr, &bottom_left_position);

  callback_data.center_point = center;
  callback_data.r = radius;
  callback_data.user_data = user_data;

  for (i = bottom_left_cell.y; i <= top_right_cell.y; i++)
    {
      for (j = bottom_left_cell.x; j <= top_right_cell.x; j++)
        {
          int cell_index = i * grid_ptr->num_cells.x + j;
          for (k = grid_ptr->cell_offsets[cell_index]; k < grid_ptr->cell_offsets[cell_index + 1]; k++)
            {
              for (l = 0; l < n_excluded; l++)
                {
                  if (excluded_indices[l] == k)
                    {
                      break;
                    }
                }
              if (l < n_excluded)
                {
                  continue;
                }
              if (distance_sq(center, grid_ptr->points[k]) >= radius * radius)
                {
                  continue;
                }
              callback_data.current_index = k;
              callback_data.current_point = grid_ptr->points[k];
              if (cb_fun(grid_ptr, callback_data))
                {
                  return;
                }
            }
        }
    }
}

static void grid_destroy(grid *grid_ptr)
{
  if (grid_ptr)
    {
      if (grid_ptr->cell_offsets)
        {
          free(grid_ptr->cell_offsets);
        }
      free(grid_ptr);
    }
}

static double2 calculate_ball_center(double2 point1, double2 point2, double r)
{
  /*
   * Calculate the center of a ball with radius `r` which has `point1` and `point2` on its circumference.
   */
  double length;
  double distance = distance_sq(point1, point2) / 4;
  double h = sqrt(r * r - distance);
  double2 m, m_normalized, center;
  m.x = (point2.x - point1.x) / 2;
  m.y = (point2.y - point1.y) / 2;
  length = sqrt(m.x * m.x + m.y * m.y);
  m_normalized.x = m.x / length;
  m_normalized.y = m.y / length;
  center.x = point1.x + m.x + h * m_normalized.y;
  center.y = point1.y + m.y - h * m_normalized.x;
  return center;
}

static int grid_cb_ball_empty(grid *grid_ptr, grid_callback_data callback_data)
{
  /*
   * Callback function to check if a point is located inside a ball. If this function is called (and
   * both points on the circumference are excluded) at least one point is located inside the ball.
   */
  (void)grid_ptr;
  *(int *)(callback_data.user_data) = 0;
  return 1;
}

static int grid_cb_find_possible_neighbors(grid *grid_ptr, grid_callback_data callback_data)
{
  /*
   * Callback function to find all possible neighbor points. There must be a ball (sphere) with radius r
   * that has the current point (`callback_data.center_point`) and the new neighbor point on its circumference.
   * This is only possible for points with a distance less (or equal) 2*r. For each of those points the ball
   * center is calculated using `calculate_ball_center` and checked if the ball is empty using the
   * `grid_cb_ball_empty` callback in the `grid_apply_function` method.
   * After all points are processed the `neighbor_point_list` struct provided via `callback_data.user_data`
   * contains the number of points which have a distance <= 2*r (if there are none, the ball radius is too small)
   * and the number and indices of those points that resulted in an empty ball and can be used as the next
   * point in the contour.
   */
  neighbor_point_list *data = (neighbor_point_list *)(callback_data.user_data);
  double r = callback_data.r / 2;
  double2 center = callback_data.center_point;
  double2 p = callback_data.current_point;
  int index = callback_data.current_index;

  double2 ball_center = calculate_ball_center(center, p, r);
  int ball_empty = 1;
  int excluded_indices[2];
  excluded_indices[0] = data->current;
  excluded_indices[1] = index;
  grid_apply_function(grid_ptr, ball_center, r, grid_cb_ball_empty, (void *)&ball_empty, 2, excluded_indices);
  data->num_points_reachable++;
  if (!ball_empty)
    {
      return 0;
    }
  if (data->size + 1 > data->capacity)
    {
      data->capacity *= 2;
      data->point_list = realloc(data->point_list, data->capacity * sizeof(int));
      assert(data->point_list);
    }
  data->point_list[data->size++] = index;
  return 0;
}

static int grid_cb_nearest_neighbor(grid *grid_ptr, grid_callback_data callback_data)
{
  /*
   * Callback function to find the index and distance of the nearest neighbor to `center`.
   */
  double distance;
  double2 *data = (double2 *)callback_data.user_data;
  double2 center = callback_data.center_point;
  double2 point = callback_data.current_point;
  (void)grid_ptr;

  distance = distance_sq(center, point);
  if (distance > 0 && (distance < data->x || data->x < 0))
    {
      data->x = distance_sq(center, point);
      data->index = callback_data.current_index;
    }
  return 0;
}

static double2 grid_find_nearest_neighbor(grid *grid_ptr, double2 position, int index, double r)
{
  /*
   * Find the nearest point in the grid `grid_ptr` to `position`. If `index` is greater or equal 0, this
   * point of the grid is excluded. The start search radius around `position` is `r`. If no points are
   * found within this radius, it will be doubled until a point is found.
   */
  double2 result = {-1, -1, -1};
  double scale = 1;
  while (result.index < 0)
    {
      grid_apply_function(grid_ptr, position, scale * r, grid_cb_nearest_neighbor, (void *)&result, 1, &index);
      scale *= 2;
    }
  return result;
}

static double angle(double2 c, double2 p1, double2 p2)
{
  double2 v1, v2;
  v1.x = p1.x - c.x;
  v1.y = p1.y - c.y;
  v2.x = p2.x - c.x;
  v2.y = p2.y - c.y;
  return acos(v1.x * v2.x + v1.y * v2.y);
}

static int in_contour(int index, int n_contour, const int *contour)
{
  /*
   * Check if `index` is already contained in the `contour` consisting of `n_contour` points.
   */
  int i;
  for (i = n_contour - 1; i >= 0; i--)
    {
      if (contour[i] == index)
        {
          return i;
        }
    }
  return -1;
}

int find_boundary(int n, double *x, double *y, double r, double (*r_function)(double, double), int n_contour,
                  int *contour)
{
  /*
   * Find the boundary of the `n` 2-dimensional points located at `x` and `y` using a ballpivot approach.
   * The indices of the contour points are stored in the `contour` array. There are several possibilities
   * to provide the ball radius used in the ballpivot algorithm:
   * - As a callback function (`r_function`) that returns the ball radius for the current position.
   * - As a constant ball radius `r`
   * - Automatically calculated / estimated from the data points
   *
   * If `r_function` is different from NULL it will be used to calculate the ball radius for each position.
   * In that case the parameter `r` is only used to set the cell size of the internal grid data structure storing
   * the points if it has a value greater 0.
   * If `r_function` is NULL and `r` is greater 0 it is used as a constant ball radius and determines the cell
   * size of the internal grid. Otherwise a (constant) ball radius is automatically calculated as 1.2 times the
   * the largest distance from a point to its nearest neighbor.
   *
   * If the algorithm is successful it returns the number of contour points that were written in `contour`.
   * Otherwise the return value is less than 0 indicating a too small (`FIND_BOUNDARY_BALL_TOO_SMALL`) or too
   * large (`FIND_BOUNDARY_BALL_TOO_LARGE`) ball radius, an invalid number of points (`FIND_BOUNDARY_INVALID_POINTS`)
   * or that the number of contour points exceed the available memory in `contour` given by `n_contour`.
   */
  double2 bb_bottom_left, bb_top_right;
  double2 *points;
  double cell_size;
  neighbor_point_list data;
  grid *grid_ptr;
  int i, start_point, current_index;
  int num_contour_points = 0;

  if (n < 2)
    {
      return FIND_BOUNDARY_INVALID_POINTS;
    }

  if (n_contour <= 1)
    {
      return FIND_BOUNDARY_MEMORY_EXCEEDED;
    }

  points = malloc(n * sizeof(double2));
  assert(points);
  for (i = 0; i < n; i++)
    {
      points[i].x = x[i];
      points[i].y = y[i];
      points[i].index = i;
    }
  calculate_bounding_box(n, points, &bb_bottom_left, &bb_top_right);
  if (r <= 0)
    {
      /* If no radius is given, estimate the cell size as 1/10 of the average of width and height of the data's
       * bounding box. */
      cell_size = ((bb_top_right.x - bb_bottom_left.x) + (bb_top_right.y - bb_bottom_left.y)) / 2. / 10.;
    }
  else
    {
      /* Scale radius by 1.1 to make sure that only direct neighbor cells have to be checked. */
      cell_size = r * 1.1;
    }
  grid_ptr = grid_create(n, points, cell_size);

  /* Start from the point closest to the bottom left corner of the bounding box*/
  start_point = grid_find_nearest_neighbor(grid_ptr, grid_ptr->bounding_box[0], -1, cell_size).index;
  contour[num_contour_points] = start_point;

  if (r <= 0 && r_function == NULL)
    { /* No radius given, calculate from data */
      for (i = 0; i < n; i++)
        {
          double nearest_neighbor = grid_find_nearest_neighbor(grid_ptr, grid_get_elem(grid_ptr, i), i, cell_size).x;
          if (nearest_neighbor > r)
            {
              /* Calculate r as the largest distance from one point to its nearest neighbor. This makes sure, that
               * at least one neighbor can be reached from each point. */
              r = nearest_neighbor;
            }
        }
      r = sqrt(r);
      r *= 1.2;
    }

  /* Initialize list structure that stores the possible neighbors in each step. */
  data.capacity = 10;
  data.point_list = malloc(data.capacity * sizeof(int));
  assert(data.point_list);

  current_index = start_point;
  while (num_contour_points == 0 || contour[num_contour_points] != contour[0])
    {
      double2 current_point;
      data.current = current_index;
      data.size = 0;
      data.num_points_reachable = 0;
      current_point = grid_get_elem(grid_ptr, current_index);

      if (num_contour_points >= n_contour)
        {
          return FIND_BOUNDARY_MEMORY_EXCEEDED;
        }

      if (r_function)
        {
          r = r_function(current_point.x, current_point.y);
          if (r <= 0)
            {
              return FIND_BOUNDARY_BALL_TOO_SMALL;
            }
        }

      /* Find the possible neighbors of `current_point` using the grid structure. */
      grid_apply_function(grid_ptr, current_point, 2 * r, grid_cb_find_possible_neighbors, (void *)&data, 1,
                          &current_index);
      if (data.size == 1)
        { /* Only one neighbor is possible */
          current_index = data.point_list[0];
          contour[++num_contour_points] = current_index;
        }
      else if (data.size > 1)
        { /* More than one point is a possible neighbor */
          int best_neighbor = 0;
          double best_angle = 0;
          int oldest = num_contour_points + 1;
          int unvisited_points = 0;

          /* If at least one possible neighbor is not included in the contour until now use the (unvisited) one
           * with the smallest angle. Otherwise use the one that was visited first to avoid (infinite) loops. */
          for (i = 0; i < (int)data.size; i++)
            {
              int contour_point_index = in_contour(data.point_list[i], num_contour_points, contour);
              if (contour_point_index < 0)
                {
                  double2 previous_contour_point = grid_get_elem(grid_ptr, contour[num_contour_points - 1]);
                  double2 possible_contour_point = grid_get_elem(grid_ptr, data.point_list[i]);
                  double a = angle(current_point, previous_contour_point, possible_contour_point);
                  if (a > best_angle)
                    {
                      best_angle = a;
                      best_neighbor = i;
                    }
                  unvisited_points++;
                }
              else if (!unvisited_points)
                {
                  if (contour_point_index < oldest)
                    {
                      oldest = contour_point_index;
                      best_neighbor = i;
                    }
                }
            }
          current_index = data.point_list[best_neighbor];
          contour[++num_contour_points] = current_index;
        }
      else
        { /* No possible neighbor is found. */
          if (data.num_points_reachable == 0)
            { /* No point was reachable -> ball too small */
              return FIND_BOUNDARY_BALL_TOO_SMALL;
            }
          else
            { /* No reachable point resulted in an empty ball -> ball too large */
              return FIND_BOUNDARY_BALL_TOO_LARGE;
            }
        }
    }
  /* The grid data structure reorders the points. Restore original indices of the contour points. */
  for (i = 0; i < num_contour_points; i++)
    {
      contour[i] = points[contour[i]].index;
    }

  free(points);
  free(data.point_list);
  grid_destroy(grid_ptr);

  return num_contour_points;
}
