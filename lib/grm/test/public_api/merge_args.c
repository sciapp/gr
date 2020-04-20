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

#define cleanup_if(condition) \
  do                          \
    {                         \
      if (condition)          \
        {                     \
          goto cleanup;       \
        }                     \
    }                         \
  while (0)

static void test_merge(void)
{
  double plots[4][2][3];
  grm_args_t *subplot = NULL, *series = NULL;
  int i, j, k;

  for (i = 0; i < 4; ++i)
    {
      for (j = 0; j < 2; ++j)
        {
          for (k = 0; k < 3; ++k)
            {
              plots[i][j][k] = i * 3 * 2 + j * 3 + k;
            }
        }
    }

  series = grm_args_new();
  grm_args_push(series, "x", "nD", 3, plots[1][0]);
  grm_args_push(series, "y", "nD", 3, plots[1][1]);
  cleanup_if(!grm_merge(series));
  grm_args_delete(series);
  series = NULL;

  series = grm_args_new();
  grm_args_push(series, "x", "nD", 3, plots[0][0]);
  grm_args_push(series, "y", "nD", 3, plots[0][1]);
  cleanup_if(!grm_merge(series));
  grm_args_delete(series);
  series = NULL;

  series = grm_args_new();
  grm_args_push(series, "x", "nD", 3, plots[1][0]);
  grm_args_push(series, "y", "nD", 3, plots[1][1]);
  grm_args_push(series, "series_id", "i", 2);
  cleanup_if(!grm_merge(series));
  grm_args_delete(series);
  series = NULL;

  series = grm_args_new();
  grm_args_push(series, "x", "nD", 3, plots[3][0]);
  grm_args_push(series, "y", "nD", 3, plots[3][1]);
  grm_args_push(series, "id", "s", "2.2");
  cleanup_if(!grm_merge(series));
  grm_args_delete(series);
  series = NULL;

  series = grm_args_new();
  grm_args_push(series, "x", "nD", 3, plots[2][0]);
  grm_args_push(series, "y", "nD", 3, plots[2][1]);
  subplot = grm_args_new();
  grm_args_push(subplot, "series", "a", series);
  grm_args_push(subplot, "subplot_id", "i", 2);
  cleanup_if(!grm_merge(subplot));
  grm_args_delete(subplot);
  subplot = NULL;
  series = NULL;

  series = grm_args_new();
  grm_args_push(series, "x", "nD", 3, plots[3][1]);
  grm_args_push(series, "id", "s", "2.1");
  cleanup_if(!grm_merge(series));
  grm_args_delete(series);
  series = NULL;

  series = grm_args_new();
  grm_args_push(series, "x", "nD", 3, plots[2][0]);
  grm_args_push(series, "y", "nD", 3, plots[2][1]);
  grm_args_push(series, "id", "s", "2.1");
  cleanup_if(!grm_merge(series));
  grm_args_delete(series);
  series = NULL;

cleanup:
  if (subplot != NULL)
    {
      grm_args_delete(subplot);
    }
  if (series != NULL)
    {
      grm_args_delete(series);
    }
  grm_finalize();
}

int main(void)
{
  test_merge();

  return 0;
}
