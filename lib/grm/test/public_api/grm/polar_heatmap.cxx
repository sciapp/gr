#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif
#include <math.h>
#include <stdio.h>

#include "grm.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define X_DIM 360
#define Y_DIM 200

#define X_MIN -2.0
#define X_MAX 2.0
#define Y_MIN 0.0
#define Y_MAX M_PI

static void test_y_z()
{
  // x is phi, y is rho, z is z

  double x[X_DIM], y[Y_DIM], z[X_DIM * Y_DIM];
  int i, j;
  grm_args_t *args, *series[2];

  for (i = 0; i < X_DIM; ++i)
    {
      x[i] = i * M_PI * 2 / (X_DIM - 1);
    }
  for (i = 0; i < Y_DIM; ++i)
    {
      y[i] = i * 12.0 / (Y_DIM - 1);
    }
  for (i = 0; i < X_DIM; ++i)
    {
      for (j = 0; j < Y_DIM; ++j)
        {
          z[i + j * X_DIM] = (i + j) * 1.0 / (X_DIM * Y_DIM);
        }
    }

  printf("plot a polar_heatmap with x, y and z\n");
  args = grm_args_new();
  grm_args_push(args, "y", "nD", Y_DIM, y);
  grm_args_push(args, "z", "nD", X_DIM * Y_DIM, z);
  grm_args_push(args, "kind", "s", "polar_heatmap");
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();
}

static void test_polar_heatmap_uniform(void)
{
  // x is phi, y is rho, z is z

  double x[X_DIM], y[Y_DIM], z[X_DIM * Y_DIM];
  int i, j;
  grm_args_t *args, *series[2];

  for (i = 0; i < X_DIM; ++i)
    {
      x[i] = i * M_PI * 2 / (X_DIM - 1);
    }
  for (i = 0; i < Y_DIM; ++i)
    {
      y[i] = i * 7.0 / (Y_DIM - 1);
    }
  for (i = 0; i < X_DIM; ++i)
    {
      for (j = 0; j < Y_DIM; ++j)
        {
          z[i + j * X_DIM] = sin(y[j] * 2.0) * cos(x[i]);
        }
    }

  printf("plot a polar_heatmap with x, y and z\n");
  args = grm_args_new();
  grm_args_push(args, "x", "nD", X_DIM, x);
  grm_args_push(args, "y", "nD", Y_DIM, y);
  grm_args_push(args, "z", "nD", X_DIM * Y_DIM, z);
  grm_args_push(args, "kind", "s", "polar_heatmap");
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();
}

static void test_polar_heatmap_z_only(void)
{
  // x is phi, y is rho, z is z

  double x[X_DIM], y[Y_DIM], z[X_DIM * Y_DIM];
  int i, j;
  grm_args_t *args, *series[2];

  for (i = 0; i < X_DIM; ++i)
    {
      x[i] = i * M_PI * 2 / (X_DIM - 1);
    }
  for (i = 0; i < Y_DIM; ++i)
    {
      y[i] = i * 7.0 / (Y_DIM - 1);
    }
  for (i = 0; i < X_DIM; ++i)
    {
      for (j = 0; j < Y_DIM; ++j)
        {
          z[i + j * X_DIM] = sin(y[j] * 2.0) * cos(x[i]);
        }
    }

  printf("plot a polar_heatmap with x, y and z\n");
  args = grm_args_new();
  grm_args_push(args, "x", "nD", X_DIM, x);
  grm_args_push(args, "y", "nD", Y_DIM, y);
  grm_args_push(args, "z", "nD", X_DIM * Y_DIM, z);
  grm_args_push(args, "kind", "s", "polar_heatmap");
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();
}

static void test_polar_heatmap_nonuniform(void)
{
  // x is phi, y is rho, z is z
  double x[X_DIM], y[Y_DIM], z[X_DIM * Y_DIM];
  int i, j;
  grm_args_t *args, *series[2];

  for (i = 0; i < X_DIM; ++i)
    {
      x[i] = std::pow(i, 2) * M_PI * 2 / (std::pow(X_DIM - 1, 2));
    }
  for (i = 0; i < Y_DIM; ++i)
    {
      y[i] = i * 7.0 / (Y_DIM - 1);
    }
  for (i = 0; i < X_DIM; ++i)
    {
      for (j = 0; j < Y_DIM; ++j)
        {
          z[i + j * X_DIM] = sin(y[j] * 2.0) * cos(x[i]);
        }
    }


  printf("plot a polar_heatmap with x, y and z\n");
  args = grm_args_new();
  grm_args_push(args, "x", "nD", X_DIM, x);
  grm_args_push(args, "y", "nD", Y_DIM, y);
  grm_args_push(args, "z", "nD", X_DIM * Y_DIM, z);
  grm_args_push(args, "kind", "s", "nonuniformpolar_heatmap");
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();
}


int main(void)
{
  test_y_z();
  test_polar_heatmap_uniform();
  //  test_polar_heatmap_z_only(); /* z only does not work?*/
  test_polar_heatmap_nonuniform();
  grm_finalize();

  return 0;
}
