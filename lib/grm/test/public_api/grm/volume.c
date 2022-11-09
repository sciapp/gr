#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "grm.h"


#define LENGTH 50

static void test_plot(void)
{
  double plot[LENGTH][LENGTH][LENGTH];
  int n = LENGTH * LENGTH * LENGTH;
  unsigned int DIMS[3] = {LENGTH, LENGTH, LENGTH};
  double x, y, z;

  grm_args_t *args;
  int i, j, k;

  printf("filling argument container...\n");

  for (i = 0; i < LENGTH; ++i)
    {
      for (j = 0; j < LENGTH; ++j)
        {
          for (k = 0; k < LENGTH; ++k)
            {
              x = i / (LENGTH / 2.) - 1;
              y = j / (LENGTH / 2.) - 1;
              z = k / (LENGTH / 2.) - 1;
              plot[i][j][k] = 1. - sqrt(x * x + y * y + z * z);
            }
        }
    }

  args = grm_args_new();
  grm_args_push(args, "kind", "s", "volume");
  grm_args_push(args, "c", "nD", n, plot);
  grm_args_push(args, "c_dims", "nI", 3, DIMS);
  grm_args_push(args, "algorithm", "s", "emission");

  printf("Plot volume with algorithm \"emission\"...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_push(args, "algorithm", "s", "absorption");

  printf("Plot volume with algorithm \"absorption\"...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_push(args, "algorithm", "s", "maximum");

  printf("Plot volume with algorithm \"maximum\"...\n");

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
