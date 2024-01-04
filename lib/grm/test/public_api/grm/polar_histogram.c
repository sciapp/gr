#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif

#include "grm.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <stdio.h>
#include <stdlib.h>

static void polar_histogram_ylim(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  grm_args_t *args;

  printf("Polar histogram with ylim...\n");

  args = grm_args_new();
  double bin_edges[] = {0.9, M_PI / 2, M_PI * 1.0, M_PI * 1.33, 2.0 * M_PI};
  unsigned int bin_edges_length = sizeof(bin_edges) / sizeof(bin_edges[0]);

  /*
    grm_args_push(args, "bin_edges", "nD", bin_edges_length, bin_edges);
  */

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "y_lim", "dd", 0.0, 8.0);
  /*
      grm_args_push(args, "x_range", "dd", 60.0, 240.0);
  */

  grm_args_push(args, "x_colormap", "i", 44);
  grm_args_push(args, "y_colormap", "i", 44);
  grm_args_push(args, "draw_edges", "i", 1);

  /*
    grm_args_push(args, "stairs", "i", 1);
  */


  grm_args_push(args, "keep_radii_axes", "i", 1);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);

  printf("Polar histogram with ylim...\n");

  args = grm_args_new();

  /*
    grm_args_push(args, "bin_edges", "nD", bin_edges_length, bin_edges);
  */

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "y_lim", "dd", 2.0, 4.0);

  /*  grm_args_push(args, "xcolormap", "i", 44); */ /* VIRIDIS */ /*
        grm_args_push(args, "ycolormap", "i", 44);
        grm_args_push(args, "draw_edges", "i", 1);*/
                                                                  /*
                                                                    grm_args_push(args, "stairs", "i", 1);
                                                                  */


  grm_args_push(args, "keep_radii_axes", "i", 1);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_xrange(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  grm_args_t *args;

  printf("Polar histogram with xrange...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "xrange", "dd", M_PI / 2.0, M_PI * 1.5);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_minimal(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  grm_args_t *args;

  printf("Polar histogram with minimal input...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_phiflip(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  grm_args_t *args;

  printf("Polar histogram with phiflip...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "phiflip", "i", 1);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_nbins(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  grm_args_t *args;

  printf("Polar histogram with nbins = 3...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "nbins", "i", 3);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_bin_counts(void)
{
  int bin_counts[] = {4, 2, 5, 1};
  unsigned int bin_counts_length = sizeof(bin_counts) / sizeof(bin_counts[0]);
  grm_args_t *args;

  printf("Polar histogram with bin_counts instead of theta values...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nI", bin_counts_length, &bin_counts);
  grm_args_push(args, "bin_counts", "i", 1);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_colormap(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  grm_args_t *args;

  printf("Polar histogram with colormaps...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "xcolormap", "i", 44); /* VIRIDIS */
  grm_args_push(args, "ycolormap", "i", 44);
  grm_args_push(args, "draw_edges", "i", 1);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_bin_edges(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  double bin_edges[] = {0.0, M_PI / 2, M_PI * 1.0, M_PI * 1.33, 2 * M_PI};
  unsigned int bin_edges_length = sizeof(bin_edges) / sizeof(bin_edges[0]);
  grm_args_t *args;

  printf("Polar histogram with bin_edges instead of theta values...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "bin_edges", "nD", bin_edges_length, bin_edges);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_stairs(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  grm_args_t *args;

  printf("Polar histogram with stairs...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "stairs", "i", 1);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_rlim(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  double rlim[] = {0.25, 0.5};
  grm_args_t *args;

  printf("Polar histogram with rlim...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "rlim", "dd", rlim[0], rlim[1]);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_philim(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  double philim[] = {M_PI / 4.0, 1.5 * M_PI};
  grm_args_t *args;

  printf("Polar histogram with philim...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "philim", "dd", philim[0], philim[1]);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_normalization(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  const char *norms[] = {"count", "probability", "countdensity", "pdf", "cumcount", "cdf", NULL};
  const char **current_norm_ptr = norms;
  grm_args_t *args;

  printf("Polar histogram with different normalizations...\n");

  while (*current_norm_ptr != NULL)
    {
      printf("    norm = \"%s\"...\n", *current_norm_ptr);

      args = grm_args_new();

      grm_args_push(args, "kind", "s", "polar_histogram");
      grm_args_push(args, "x", "nD", theta_length, &theta);
      grm_args_push(args, "normalization", "s", *current_norm_ptr);

      grm_plot(args);
      printf("Press any key to continue...\n");
      printf("%s\n", *current_norm_ptr);
      getchar();
      grm_args_delete(args);

      ++current_norm_ptr;
    }
}

static void polar_histogram_bin_width(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  double bin_width = M_PI / 2.0;
  grm_args_t *args;

  printf("Polar histogram with bin_width...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "bin_width", "d", bin_width);

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void polar_histogram_title(void)
{
  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);
  double bin_width = M_PI / 2.0;
  grm_args_t *args;

  printf("Polar histogram with a title...\n");

  args = grm_args_new();

  grm_args_push(args, "kind", "s", "polar_histogram");
  grm_args_push(args, "x", "nD", theta_length, &theta);
  grm_args_push(args, "title", "s", "testing the polar histogram");

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}


static void test_plot(void)
{
  polar_histogram_ylim();
  /* polar_histogram_xrange();
   polar_histogram_minimal();
   polar_histogram_phiflip();
   polar_histogram_nbins();
   polar_histogram_bin_counts();
   polar_histogram_colormap();
   polar_histogram_bin_edges(); */
  polar_histogram_stairs();
  polar_histogram_rlim();
  polar_histogram_philim();
  polar_histogram_normalization();
  polar_histogram_bin_width();
  polar_histogram_title();

  grm_finalize();
}


int main(void)
{
  test_plot();
}
