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

#define X_DIM 30
#define Y_DIM 20

#define X_MIN -1.0
#define X_MAX 1.0
#define Y_MIN -1.0
#define Y_MAX 1.0


static void test_quiver(void)
{
  double x[X_DIM], y[Y_DIM], u[X_DIM * Y_DIM], v[X_DIM * Y_DIM];
  int i, j;
  grm_args_t *args;

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
          u[j * X_DIM + i] = X_MIN + (X_MAX - X_MIN) * ((double)i / (X_DIM - 1));
          v[j * X_DIM + i] = Y_MIN + (Y_MAX - Y_MIN) * ((double)j / (Y_DIM - 1));
        }
    }

  printf("plot a quiver with x, y and z\n");
  args = grm_args_new();
  grm_args_push(args, "x", "nD", X_DIM, x);
  grm_args_push(args, "y", "nD", Y_DIM, y);
  grm_args_push(args, "u", "nD", X_DIM * Y_DIM, u);
  grm_args_push(args, "v", "nD", X_DIM * Y_DIM, v);
  grm_args_push(args, "kind", "s", "quiver");
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}


int main(void)
{
  test_quiver();
  grm_finalize();

  return 0;
}
