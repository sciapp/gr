#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif
#include <stdio.h>
#include "grm.h"

#define ROWS 4
#define COLS 4
#define XMIN 0
#define XMAX 4
#define YMIN 0
#define YMAX 4

static void test_marginalheatmap()
{
  unsigned int row, col;
  grm_args_t *args;
  double xi[ROWS], yi[COLS], zi[ROWS * COLS];
  double df[COLS][ROWS];

  df[0][0] = 1, df[1][0] = 2, df[2][0] = 3, df[3][0] = 4;
  df[0][1] = 2, df[1][1] = 8, df[2][1] = 4, df[3][1] = 5;
  df[0][2] = 3, df[1][2] = 4, df[2][2] = 5, df[3][2] = 6;
  df[0][3] = 4, df[1][3] = 5, df[2][3] = 6, df[3][3] = 7;

  for (row = 0; row < ROWS; ++row)
    {
      xi[row] = XMIN + (XMAX - XMIN) * ((double)row / ((double)ROWS - 1));
      for (col = 0; col < COLS; ++col)
        {
          if (row == 0)
            {
              yi[col] = YMIN + (YMAX - YMIN) * ((double)col / ((double)COLS - 1));
            }
          zi[((COLS - 1) - col) * ROWS + row] = df[col][row];
        }
    }

  printf("plot a marginalheatmap with x, y and z\n");
  args = grm_args_new();
  grm_args_push(args, "x", "nD", ROWS, xi);
  grm_args_push(args, "y", "nD", COLS, yi);
  grm_args_push(args, "z", "nD", ROWS * COLS, zi);
  grm_args_push(args, "kind", "s", "marginalheatmap");
  grm_args_push(args, "type", "s", "all");
  grm_args_push(args, "algorithm", "s", "sum");
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);

  printf("plot a special type of marginalheatmap where only one line and column is shown\n");
  args = grm_args_new();
  grm_args_push(args, "x", "nD", ROWS, xi);
  grm_args_push(args, "y", "nD", COLS, yi);
  grm_args_push(args, "z", "nD", ROWS * COLS, zi);
  grm_args_push(args, "kind", "s", "marginalheatmap");
  grm_args_push(args, "type", "s", "line");
  grm_args_push(args, "algorithm", "s", "sum");
  grm_args_push(args, "xind", "i", 1);
  grm_args_push(args, "yind", "i", 1);
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}


int main()
{
  test_marginalheatmap();
  grm_finalize();

  return 0;
}
