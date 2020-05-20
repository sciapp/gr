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

#define X_DIM 40
#define Y_DIM 20

#define X_MIN -2.0
#define X_MAX 2.0
#define Y_MIN 0.0
#define Y_MAX M_PI


static void test_heatmap(void)
{
  double x[X_DIM], y[Y_DIM], z[X_DIM * Y_DIM];
  int i, j;
  grm_args_t *args, *series[2];

  for (i = 0; i < X_DIM; ++i)
    {
      x[i] = X_MIN + (X_MAX - X_MIN) * ((double)i / (X_DIM - 1));
    }
  for (i = 0; i < Y_DIM; ++i)
    {
      y[i] = Y_MIN + (Y_MAX - Y_MIN) * ((double)i / (Y_DIM - 1));
    }
  for (i = 0; i < X_DIM; ++i)
    {
      for (j = 0; j < Y_DIM; ++j)
        {
          z[((Y_DIM - 1) - j) * X_DIM + i] = sin(x[i]) + cos(y[j]);
        }
    }

  printf("plot a heatmap with x, y and z\n");
  args = grm_args_new();
  grm_args_push(args, "x", "nD", X_DIM, x);
  grm_args_push(args, "y", "nD", Y_DIM, y);
  grm_args_push(args, "z", "nD", X_DIM * Y_DIM, z);
  grm_args_push(args, "kind", "s", "heatmap");
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);

  printf("plot a heatmap with y and z\n");
  args = grm_args_new();
  grm_args_push(args, "y", "nD", Y_DIM, y);
  grm_args_push(args, "z", "nD", X_DIM * Y_DIM, z);
  grm_args_push(args, "kind", "s", "heatmap");
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);

  printf("plot a heatmap with z only\n");
  args = grm_args_new();
  grm_args_push(args, "z", "nD", X_DIM * Y_DIM, z);
  grm_args_push(args, "z_dims", "ii", Y_DIM, X_DIM);
  grm_args_push(args, "kind", "s", "heatmap");
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);

  printf("plot a heatmap with z only (but with limits)\n");
  args = grm_args_new();
  grm_args_push(args, "z", "nD", X_DIM * Y_DIM, z);
  grm_args_push(args, "z_dims", "ii", Y_DIM, X_DIM);
  grm_args_push(args, "kind", "s", "heatmap");
  grm_args_push(args, "xrange", "dd", -2.0, 2.0);
  grm_args_push(args, "yrange", "dd", 0.0, M_PI);
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);

  printf("plot a heatmap with x, y, z and with (larger) limits\n");
  args = grm_args_new();
  grm_args_push(args, "x", "nD", X_DIM, x);
  grm_args_push(args, "y", "nD", Y_DIM, y);
  grm_args_push(args, "z", "nD", X_DIM * Y_DIM, z);
  grm_args_push(args, "z_dims", "ii", Y_DIM, X_DIM);
  grm_args_push(args, "kind", "s", "heatmap");
  grm_args_push(args, "xrange", "dd", -3.0, 3.0);
  grm_args_push(args, "yrange", "dd", -1.0, M_PI + 1.0);
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);

  printf("plot two heatmaps\n");
  args = grm_args_new();
  series[0] = grm_args_new();
  grm_args_push(series[0], "z", "nD", X_DIM * Y_DIM, z);
  grm_args_push(series[0], "z_dims", "ii", Y_DIM, X_DIM);
  grm_args_push(series[0], "xrange", "dd", -4.25, -0.25);
  grm_args_push(series[0], "yrange", "dd", 0.0, M_PI);
  series[1] = grm_args_new();
  grm_args_push(series[1], "z", "nD", X_DIM * Y_DIM, z);
  grm_args_push(series[1], "z_dims", "ii", Y_DIM, X_DIM);
  grm_args_push(series[1], "xrange", "dd", 0.25, 4.25);
  grm_args_push(series[1], "yrange", "dd", 0.0, M_PI);
  grm_args_push(args, "kind", "s", "heatmap");
  grm_args_push(args, "series", "nA", 2, series);
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);

  printf("plot two non-uniform heatmaps\n");
  args = grm_args_new();
  x[0] = -3.25;
  x[1] = -1.25;
  x[2] = -0.25;
  x[3] = 0.25;
  x[4] = 1.25;
  x[5] = 3.25;
  y[0] = 0.0;
  y[1] = 2.0;
  y[2] = 3.0;
  series[0] = grm_args_new();
  grm_args_push(series[0], "x", "nD", 3, x);
  grm_args_push(series[0], "y", "nD", 3, y);
  grm_args_push(series[0], "z", "nD", 3 * 3, z);
  grm_args_push(series[0], "z_dims", "ii", 3, 3);
  series[1] = grm_args_new();
  grm_args_push(series[1], "x", "nD", 3, x + 3);
  grm_args_push(series[1], "y", "nD", 3, y);
  grm_args_push(series[1], "z", "nD", 3 * 3, z);
  grm_args_push(series[1], "z_dims", "ii", 3, 3);
  grm_args_push(args, "kind", "s", "heatmap");
  grm_args_push(args, "series", "nA", 2, series);
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}


int main(void)
{
  test_heatmap();
  grm_finalize();

  return 0;
}
