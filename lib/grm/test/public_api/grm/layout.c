#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif
#include <math.h>
#include "grm.h"
#include "gr.h"

int STD_COLORS[] = {989, 982, 980, 981, 996, 983, 995, 988, 986, 990, 991, 984, 992, 993, 994, 987, 985, 997, 998, 999};

void visualize(grm_element_t **elements, int nelems)
{
  int i;
  double *subplot;

  gr_setwindow(0, 1, 0, 1);
  gr_setviewport(0, 1, 0, 1);
  gr_setfillintstyle(1);

  for (i = 0; i < nelems; i++)
    {
      grm_element_get_subplot(elements[i], &subplot);
      gr_setfillcolorind(STD_COLORS[i]);
      gr_fillrect(subplot[0], subplot[1], subplot[2], subplot[3]);
      /*        printf("[%f %f %f %f]\n", subplot[0], subplot[1], subplot[2], subplot[3]); */
    }
  getchar();
  gr_clearws();
}

void test_grid(void)
{
  int i, nelems = 6;

  grm_grid_t *grid1;
  grm_grid_new(1, 1, &grid1);

  grm_element_t *elements[nelems];

  for (i = 0; i < nelems; i++)
    {
      grm_element_new(&elements[i]);
    }

  grm_grid_set_element_slice(0, 1, 0, 1, elements[0], grid1);
  grm_grid_set_element_slice(0, 1, 1, 2, elements[1], grid1);

  grm_grid_finalize(grid1);

  visualize(elements, nelems);

  grm_element_set_abs_width(elements[2], 0.25);
  grm_element_set_abs_height(elements[2], 0.2);
  grm_element_set_fit_parents_height(elements[2], 1);
  grm_grid_set_element_slice(0, 1, 2, 3, elements[2], grid1);

  grm_grid_finalize(grid1);

  visualize(elements, nelems);

  grm_element_set_fit_parents_width(elements[2], 0);
  grm_grid_set_element_slice(1, 2, 0, 2, elements[2], grid1);

  grm_grid_finalize(grid1);

  visualize(elements, nelems);

  grm_trim(grid1);

  grm_grid_finalize(grid1);

  visualize(elements, nelems);

  grm_grid_set_element_slice(0, 1, 2, 3, elements[3], grid1);
  grm_grid_set_element_slice(1, 2, 2, 3, elements[4], grid1);

  grm_grid_finalize(grid1);

  visualize(elements, nelems);

  grm_grid_t *grid2;
  grm_grid_new(2, 1, &grid2);
  grm_grid_set_element_slice(0, 1, 0, 1, elements[3], grid2);
  grm_grid_set_element_slice(1, 2, 0, 1, elements[4], grid2);
  grm_grid_set_element_slice(0, 2, 2, 3, grid2, grid1);

  grm_grid_finalize(grid1);

  visualize(elements, nelems);

  grm_grid_delete(grid1);
}

void test_grid_with_grm(void)
{
  double plots[4][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  grm_args_t *args, *subplots[4];
  int i, j;
  int nelems = 6;
  grm_grid_t *grid = NULL;

  printf("filling argument container...\n");

  for (i = 0; i < 2; ++i)
    {
      for (j = 0; j < n; ++j)
        {
          plots[i][0][j] = j * 2 * M_PI / n;
          plots[i][1][j] = sin((j * (i + 1) * 2) * M_PI / n);
        }
    }
  for (i = 0; i < 2; ++i)
    {
      for (j = 0; j < n; ++j)
        {
          plots[2 + i][0][j] = j * 2 * M_PI / n;
          plots[2 + i][1][j] = cos((j * (i + 1) * 2) * M_PI / n);
        }
    }

  grm_grid_new(2, 2, &grid);
  for (i = 0; i < 4; ++i)
    {
      subplots[i] = grm_args_new();
      grm_args_push(subplots[i], "x", "nD", n, plots[i][0]);
      grm_args_push(subplots[i], "y", "nD", n, plots[i][1]);
      grm_grid_set_element_args(i / 2, i % 2, subplots[i], grid);
    }
  grm_grid_finalize(grid);

  args = grm_args_new();
  grm_args_push(args, "subplots", "nA", 4, subplots);

  printf("plotting data...\n");

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
  grm_grid_delete(grid);
}

void test_grid_with_grm_args(void)
{
  double plots[4][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  grm_args_t *args, *subplots[5];
  int i, j;

  printf("filling argument container...\n");

  for (i = 0; i < 2; ++i)
    {
      for (j = 0; j < n; ++j)
        {
          plots[i][0][j] = j * 2 * M_PI / n;
          plots[i][1][j] = sin((j * (i + 1) * 2) * M_PI / n);
        }
    }
  for (i = 0; i < 2; ++i)
    {
      for (j = 0; j < n; ++j)
        {
          plots[2 + i][0][j] = j * 2 * M_PI / n;
          plots[2 + i][1][j] = cos((j * (i + 1) * 2) * M_PI / n);
        }
    }

  for (i = 0; i < 4; ++i)
    {
      subplots[i] = grm_args_new();
      grm_args_push(subplots[i], "x", "nD", n, plots[i][0]);
      grm_args_push(subplots[i], "y", "nD", n, plots[i][1]);

      if (i == 0)
        {
          grm_args_push(subplots[i], "row", "i", 0);
          grm_args_push(subplots[i], "col", "i", 0);
        }
      else
        {
          grm_args_push(subplots[i], "row", "ii", 0, (i - 1) / 2);
          grm_args_push(subplots[i], "col", "ii", 1, (i - 1) % 2);
          grm_args_push(subplots[i], "colspan", "i", 2);
        }
      if (i == 3)
        {
          grm_args_push(subplots[i], "colspan", "ii", 2, 2);
        }
    }

  args = grm_args_new();
  grm_args_push(args, "subplots", "nA", 4, subplots);

  printf("plotting data...\n");

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

void test_grid_with_grm_args_and_width_parameters(void)
{
  double plots[6][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  grm_args_t *args, *subplots[6];
  int i, j;

  printf("filling argument container...\n");

  for (i = 0; i < 2; ++i)
    {
      for (j = 0; j < n; ++j)
        {
          plots[i][0][j] = j * 2 * M_PI / n;
          plots[i][1][j] = sin((j * (i + 1) * 2) * M_PI / n);
        }
    }
  for (i = 0; i < 4; ++i)
    {
      for (j = 0; j < n; ++j)
        {
          plots[2 + i][0][j] = j * 2 * M_PI / n;
          plots[2 + i][1][j] = cos((j * (i + 1) * 2) * M_PI / n);
        }
    }

  for (i = 0; i < 6; ++i)
    {
      subplots[i] = grm_args_new();
      grm_args_push(subplots[i], "x", "nD", n, plots[i][0]);
      grm_args_push(subplots[i], "y", "nD", n, plots[i][1]);
    }


  for (i = 0; i < 3; ++i)
    {
      grm_args_push(subplots[i], "row", "i", i / 2);
      grm_args_push(subplots[i], "col", "i", i % 2);
    }
  for (i = 0; i < 3; ++i)
    {
      grm_args_push(subplots[i + 3], "row", "i", i % 2);
      grm_args_push(subplots[i + 3], "col", "i", 2 + i / 2);
    }

  grm_args_push(subplots[2], "colspan", "i", 2);
  grm_args_push(subplots[5], "rowspan", "i", 2);

  args = grm_args_new();
  grm_args_push(args, "subplots", "nA", 6, subplots);

  grm_plot(args);
  printf("Lets give the lower left subplot an absolute height...\n");
  getchar();

  grm_args_push(subplots[2], "abs_height", "d", 0.15);

  grm_plot(args);
  printf("There is alot of whitespace now so lets set the fit_parents_height parameter of the lower left plot to true "
         "so that the grid is aware of its height...\n");
  getchar();

  grm_args_push(subplots[2], "fit_parents_height", "i", 1);

  grm_plot(args);
  printf("On the left side of the layout the available space is now better distributed however this also effected the "
         "right side of the layout so lets put these subplots in a seperate grid...\n");
  getchar();

  for (i = 0; i < 3; ++i)
    {
      grm_args_push(subplots[i + 3], "row", "ii", 0, i % 2);
      grm_args_push(subplots[i + 3], "col", "ii", 2, i / 2);
      grm_args_push(subplots[i + 3], "colspan", "i", 2);
      if (i == 2)
        {
          grm_args_push(subplots[i + 3], "rowspan", "ii", 2, 2);
        }
    }

  grm_plot(args);
  printf("Now we want to change some attributes of the subplot on the right side...\n");
  getchar();

  grm_args_push(subplots[5], "abs_width", "dd", -1.0, 0.1);
  grm_args_push(subplots[5], "rel_height", "dd", -1.0, 2 / 3.0);

  grm_plot(args);
  printf("Next...\n");
  getchar();

  grm_args_delete(args);
}

int main(void)
{
  /* test_grid(); */
  /* test_grid_with_grm(); */
  /* test_grid_with_grm_args(); */
  test_grid_with_grm_args_and_width_parameters();
  /* grm_finalize(); */

  return 0;
}
