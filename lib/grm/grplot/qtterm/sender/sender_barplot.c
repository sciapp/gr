#include "grm.h"
#include "unistd.h"

#define N_SERIES 3

int main(int argc, char **argv)
{
  int n_y = 3;
  int n_yy = 3;
  double y[] = {4, 5, 8};
  double yy1[] = {4, 8, 1};
  double yy2[] = {7, 3, 9};
  double yy3[] = {1, 4, 6};
  int c[] = {984, 992, 993};
  double c_rgb[3][3] = {{0.5, 0.4, 0.3}, {0.3, 0.4, 0.5}, {0.4, 0.3, 0.5}};
  int bar_color = 992;
  int edge_color = 989;
  double bar_width = 0.85;
  double edge_width = 1.5;
  const char *xnotations[3] = {"One", "Two", "Three"};
  grm_args_t *args;
  grm_args_t *series[N_SERIES];
  int i, j;
  void *handle;
  int port = -1;

  if (argc != 2)
    {
      fprintf(stderr, "Usage: sender_barplot <port>\n");
      return 1;
    }
  port = atoi(argv[1]);
  if (port <= 0 || port > 65536)
    {
      fprintf(stderr, "Port must be between 1 and 65536\n");
      return 1;
    }


  handle = grm_open(GRM_SENDER, "localhost", port, NULL, NULL);
  if (handle == NULL)
    {
      fprintf(stderr, "sender could not be created\n");
      return 1;
    }
  args = grm_args_new();

  /* Draw the bar plot */
  grm_args_push(args, "y", "nD", n_y, y);
  grm_args_push(args, "kind", "s", "barplot");
  grm_send_args(handle, args);
  sleep(3);
  /* Draw the bar plot with locations specified by x */
  grm_args_push(args, "x_tick_labels", "nS", n_y, xnotations);
  grm_send_args(handle, args);
  sleep(3);

  /* Draw the bar plot with different bar_width, edge_width, edge_color and bar_color */
  grm_args_push(args, "edge_width", "d", edge_width);
  grm_args_push(args, "bar_width", "d", bar_width);
  grm_args_push(args, "edge_color", "i", edge_color);
  grm_args_push(args, "bar_color", "i", bar_color);
  grm_send_args(handle, args);
  sleep(3);
  /* or */
  grm_args_push(args, "bar_color", "ddd", 0.66, 0.66, 0.66);
  grm_args_push(args, "edge_color", "ddd", 0.33, 0.33, 0.33);
  grm_send_args(handle, args);
  sleep(3);

  /* Draw the bar plot with colorlist */
  grm_args_delete(args);
  args = grm_args_new();
  grm_args_push(args, "y", "nD", n_y, y);
  grm_args_push(args, "kind", "s", "barplot");
  grm_args_push(args, "c", "nI", n_y, c);
  grm_send_args(handle, args);
  sleep(3);
  /* Or */
  grm_args_push(args, "c", "nD", 3 * n_y, c_rgb);
  grm_send_args(handle, args);
  sleep(3);

  /* Draw a 2D bar plot lined */
  grm_args_delete(args);
  args = grm_args_new();
  for (i = 0; i < N_SERIES; i++)
    {
      series[i] = grm_args_new();
    }
  grm_args_push(series[0], "y", "nD", n_yy, yy1);
  grm_args_push(series[1], "y", "nD", n_yy, yy2);
  grm_args_push(series[2], "y", "nD", n_yy, yy3);

  grm_args_push(args, "kind", "s", "barplot");
  grm_args_push(args, "style", "s", "lined");
  grm_args_push(args, "series", "nA", N_SERIES, series);
  grm_send_args(handle, args);
  sleep(3);

  /* Draw a 2D bar plot stacked */
  grm_args_push(args, "style", "s", "stacked");
  grm_send_args(handle, args);
  sleep(3);

  /* Draw a 2D bar plot with colorlist */
  grm_args_push(args, "c", "nI", n_yy, c);
  grm_send_args(handle, args);
  sleep(3);

  grm_close(handle);
  grm_args_delete(args);
  grm_finalize();
}
