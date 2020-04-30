#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "grm.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ROWS 20
#define COLS 40

static void test_plot(void)
{
  double plot[ROWS][COLS];
  int n = ROWS * COLS;

  grm_args_t *args;
  int i, j;

  printf("filling argument container...\n");

  for (i = 0; i < ROWS; ++i)
    {
      for (j = 0; j < COLS; ++j)
        {
          plot[i][j] = sin(4.0 * i / ROWS - 2.0) + cos(M_PI * j / COLS);
        }
    }

  args = grm_args_new();
  grm_args_push(args, "kind", "s", "imshow");
  grm_args_push(args, "title", "s", "imshow-test from c!");
  grm_args_push(args, "c", "nD", n, plot);
  grm_args_push(args, "c_dims", "ii", ROWS, COLS);
  grm_args_push(args, "colormap", "i", 44);

  printf("plotting data...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

int main(void)
{
  test_plot();
  grm_finalize();

  return 0;
}
