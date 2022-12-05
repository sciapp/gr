#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <random>

#include "grm.h"
#include <grm/dom_render/graphics_tree/util.hxx>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define LENGTH 2000
#define NBINS 20

#define N_SERIES 3
#define INNER_N_SERIES 3

#define X_DIM 40
#define Y_DIM 20

#define X_MIN -2.0
#define X_MAX 2.0
#define Y_MIN 0.0
#define Y_MAX M_PI

static void test_dom_render(void)
{
  double plots[2][2][20];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  double markertypes[n];
  int markercolorinds[n];
  double markersizes[n];
  double markercolordoubs[n];
  const char *labels[] = {"sin", "cos"};
  grm_args_t *args, *series[2];
  int i;

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      plots[0][0][i] = i * 2 * M_PI / n;
      plots[0][1][i] = sin(i * 2 * M_PI / n);
    }
  for (i = 0; i < n; ++i)
    {
      plots[1][0][i] = i * 2 * M_PI / n;
      plots[1][1][i] = cos(i * 2 * M_PI / n);
    }

  for (i = 0; i < n; ++i)
    {
      markertypes[i] = rand() % 6 + (-32);
      markercolorinds[i] = rand() % (255);
      markercolordoubs[i] = rand() % 255 * 1.0;
      markersizes[i] = rand() % 25 * 10.0;
    }

  for (i = 0; i < 2; ++i)
    {
      series[i] = grm_args_new();
      grm_args_push(series[i], "x", "nD", n, plots[i][0]);
      grm_args_push(series[i], "y", "nD", n, plots[i][1]);

      grm_args_push(series[i], "c", "nD", n, markercolordoubs);
      grm_args_push(series[i], "markertype", "nD", n, markertypes);
      grm_args_push(series[i], "z", "nD", n, markersizes);
    }


  args = grm_args_new();
  grm_args_push(args, "series", "nA", 2, series);
  grm_args_push(args, "labels", "nS", 2, labels);
  grm_args_push(args, "kind", "s", "scatter");

  printf("plotting data...\n");

  grm_plot(args);

  auto root = grm_get_document_root();
  std::cout << toXML(root) << std::endl;

  printf("Press any key to continue...\n");
  getchar();

  //! ---------

  //  int color_ind = 983;
  auto render = grm_get_render();
  //  auto context = std::make_shared<GR::Context>(GR::Context());
  for (const auto &elem : root->querySelectorsAll("polymarker"))
    {
      elem->setAttribute("markertype", 2);
      //      break;
      std::vector<double> v(markersizes, markersizes + sizeof markersizes / sizeof markersizes[0]);
      render->setMarkerSize(elem, "sizes2", v);
      break;
    }
  std::cout << toXML(root) << std::endl;

  gr_clearws();
  render->render();
  gr_updatews();

  printf("Press any key to continue...\n");
  getchar();


  grm_args_delete(args);
}

static void test_subplots(void)
{
  double plots[4][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  grm_args_t *args, *subplots[4];
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
      grm_args_push(subplots[i], "subplot", "dddd", 0.5 * (i % 2), 0.5 * (i % 2 + 1), 0.5 * (i / 2), 0.5 * (i / 2 + 1));
    }

  args = grm_args_new();
  grm_args_push(args, "subplots", "nA", 4, subplots);

  printf("plotting data...\n");

  grm_plot(args);
  printf("Press any key to continue...\n");
  auto root = grm_get_document_root();
  std::cout << toXML(root) << std::endl;

  getchar();

  gr_clearws();
  grm_render();
  gr_updatews();

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void test_hist(void)
{
  double plot[LENGTH];
  double weights[LENGTH];
  int n = LENGTH;
  double errors[2][NBINS];

  grm_args_t *args, *error;
  int i;

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      plot[i] = sin(2 * M_PI * i / n);
      weights[i] = -1.0;
    }
  for (i = 0; i < NBINS; ++i)
    {
      errors[0][i] = fabs(sin(M_PI * i / (NBINS - 1)));
      errors[1][i] = fabs(sin(M_PI * i / (NBINS - 1)));
    }

  error = grm_args_new();
  grm_args_push(error, "relative", "nDD", NBINS, errors[0], errors[1]);
  grm_args_push(error, "upwardscap_color", "i", 2);
  grm_args_push(error, "downwardscap_color", "i", 3);
  grm_args_push(error, "errorbar_color", "i", 4);

  args = grm_args_new();
  grm_args_push(args, "x", "nD", n, plot);
  grm_args_push(args, "weights", "nD", n, weights);
  grm_args_push(args, "error", "a", error);
  grm_args_push(args, "kind", "s", "hist");
  grm_args_push(args, "nbins", "i", NBINS);
  /* Color as a RGB-list */
  grm_args_push(args, "bar_color", "ddd", 0., 0., 1.);
  grm_args_push(args, "edge_color", "ddd", 1., 0., 0.);
  /* Color as an index */
  /*grm_args_push(args, "bar_color", "i", 989);*/
  /*grm_args_push(args, "edge_color", "i", 1);*/
  grm_args_push(args, "title", "s", "Histogram of a sine wave [0; 2pi] with 20 bins and negative weights");

  printf("plotting data...\n");

  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();


  auto root = grm_get_document_root();
  std::cout << toXML(root) << std::endl;

  printf("Press any key to continue...\n");
  getchar();


  std::cout << toXML(root) << std::endl;

  gr_clearws();
  grm_render();
  gr_updatews();

  printf("Press any key to continue...\n");
  getchar();


  grm_args_delete(args);
}

static void test_contour()
{
  int n = 100, i, j;
  double x[100], y[100], z[100 * 100];
  grm_args_t *args;

  std::random_device rd;  // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> dis(-4.0, 4.0);

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      x[i] = dis(gen);
      y[i] = dis(gen);
    }
  for (j = 0; j < n; ++j)
    {
      for (i = 0; i < n; ++i)
        {
          z[i] = sin(x[j]) + cos(y[i]);
        }
    }


  args = grm_args_new();
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "z", "nD", n, z);
  grm_args_push(args, "kind", "s", "contourf");

  printf("plotting data...\n");

  grm_plot(args);

  getchar();
}

static void test_plot(void)
{
  test_subplots();
  grm_finalize();
}

static void test_wireframe(void)
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
  grm_args_push(args, "kind", "s", "wireframe");
  grm_plot(args);

  printf("Press any key to continue...\n");
  getchar();
}

static void test_plot3()
{
  double x[20], y[20], z[20];
  int n = 20;
  grm_args_t *args, *series;
  int i;

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      x[i] = i;
      y[i] = cos(i) * i;
      z[i] = sin(i) * i;
    }

  series = grm_args_new();

  args = grm_args_new();
  grm_args_push(series, "x", "nD", n, x);
  grm_args_push(series, "y", "nD", n, y);
  grm_args_push(series, "z", "nD", n, z);
  grm_args_push(args, "series", "a", series);
  grm_args_push(args, "kind", "s", "scatter3");


  printf("plotting data...\n");

  grm_plot(args);

  auto root = grm_get_document_root();
  std::cout << toXML(root) << std::endl;

  printf("Press any key to continue...\n");
  getchar();
}

static void testTrisurf()
{

  int n = 100, i, j;
  double x[100], y[100], z[100];
  grm_args_t *args;

  std::random_device rd;  // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> dis(-4.0, 4.0);

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      x[i] = dis(gen);
      y[i] = dis(gen);
      z[i] = sin(x[i]) + cos(x[i]);
    }


  args = grm_args_new();
  grm_args_push(args, "x", "nD", n, x);
  grm_args_push(args, "y", "nD", n, y);
  grm_args_push(args, "z", "nD", n, z);
  grm_args_push(args, "levels", "i", 100);
  grm_args_push(args, "kind", "s", "tricont");

  printf("plotting data...\n");

  grm_plot(args);

  getchar();
}

void testBar()
{
  int n_y = 3;
  int n_yy = 3;
  double y[] = {4, 5, 8};
  double yy1[] = {4, 8, 2};
  double yy2[] = {7, 3, 9};
  double yy3[] = {1, 4, 6};
  double inner_yy2_1[] = {5, 2};
  double inner_yy2_2[] = {3};
  double inner_yy2_3[] = {9};
  int n_inner_yy2_1 = 2;
  double yy_pos_neg[] = {5, -5, 3, -3};
  int n_yy_pos_neg = 4;
  int c[] = {984, 992, 993};
  int c2[] = {997, 998, 999};
  double c_rgb[3][3] = {{0.5, 0.4, 0.3}, {0.3, 0.4, 0.5}, {0.4, 0.3, 0.5}};
  int inner_c[] = {989, 984};
  int n_inner_c = 2;
  double inner_c_rgb[2][3] = {{0.5, 0.4, 0.3}, {0.8, 0.1, 0.1}};
  int n_inner_c_rgb = 6;
  int bar_color = 992;
  int edge_color = 989;
  double bar_width = 0.85;
  double edge_width = 1.5;
  const char *xticklabels[3] = {"eins", "zwei", "drei"};
  const char *ylabels[3] = {"4", "5", "8"};
  const char *yy1_labels[3] = {"4", "8", "2"};
  const char *yy2_labels[3] = {"7", "3", "9"};
  const char *yy3_labels[3] = {"1", "4", "6"};
  const char *yy2_labels_for_inner[4] = {"5", "2", "3", "9"};
  const char *yy_pos_neg_labels[4] = {"5", "-5", "3", "-3"};
  int indices[2] = {1, 2};
  grm_args_t *args;
  grm_args_t *ind_bar_color[2];
  grm_args_t *ind_edge_color;
  grm_args_t *ind_edge_width;
  grm_args_t *series[N_SERIES];
  grm_args_t *inner_series[INNER_N_SERIES];
  int i, j;

  args = grm_args_new();

  /* Draw the bar plot */
  grm_args_push(args, "y", "nD", n_y, y);
  grm_args_push(args, "kind", "s", "barplot");
  //  grm_plot(args);
  //  sleep(3);

  /* Draw the bar plot with locations specified by x and y values in the bars*/
  grm_args_push(args, "xticklabels", "nS", n_y, xticklabels);
  grm_args_push(args, "ylabels", "nS", n_y, ylabels);

  grm_plot(args);

  auto root = grm_get_document_root();
  std::cout << toXML(root) << std::endl;

  printf("Press any key to continue...\n");
  getchar();


  /* Draw the bar plot with different bar_width, edge_width, edge_color and bar_color */
  grm_args_push(args, "edge_width", "d", edge_width);
  grm_args_push(args, "bar_width", "d", bar_width);
  grm_args_push(args, "edge_color", "i", edge_color);
  grm_args_push(args, "bar_color", "i", bar_color);
  grm_plot(args);
  sleep(3);
  /* or */
  grm_args_push(args, "bar_color", "ddd", 0.66, 0.66, 0.66);
  grm_args_push(args, "edge_color", "ddd", 0.33, 0.33, 0.33);
  grm_plot(args);
  sleep(3);

  /* Draw the bar plot with bars that have individual bar_color, edge_color, edge_with */
  for (j = 0; j < 2; ++j)
    {
      ind_bar_color[j] = grm_args_new();
    }
  grm_args_push(ind_bar_color[0], "indices", "nI", 2, indices);
  grm_args_push(ind_bar_color[0], "rgb", "ddd", 0.0, 0.666, 0.333);
  grm_args_push(ind_bar_color[1], "indices", "i", 3);
  grm_args_push(ind_bar_color[1], "rgb", "ddd", 0.111, 0.222, 0.333);

  ind_edge_color = grm_args_new();
  grm_args_push(ind_edge_color, "indices", "i", 3);
  grm_args_push(ind_edge_color, "rgb", "ddd", 0.9, 0.6, 0.3);

  ind_edge_width = grm_args_new();
  grm_args_push(ind_edge_width, "indices", "i", 3);
  grm_args_push(ind_edge_width, "width", "d", 5.0);

  grm_args_push(args, "ind_bar_color", "nA", 2, ind_bar_color);
  grm_args_push(args, "ind_edge_color", "a", ind_edge_color);
  grm_args_push(args, "ind_edge_width", "a", ind_edge_width);
  grm_plot(args);
  sleep(3);

  /* Draw the bar plot with colorlist */
  grm_args_delete(args);
  args = grm_args_new();
  grm_args_push(args, "y", "nD", n_y, y);
  grm_args_push(args, "kind", "s", "barplot");
  grm_args_push(args, "c", "nI", n_y, c);
  grm_plot(args);


  sleep(3);
  /* Or */
  grm_args_push(args, "c", "nD", 3 * n_y, c_rgb);
  grm_plot(args);
  sleep(3);

  /* Draw a 2D bar plot lined */
  grm_args_delete(args);
  args = grm_args_new();
  for (i = 0; i < N_SERIES; i++)
    {
      series[i] = grm_args_new();
    }
  grm_args_push(series[0], "y", "nD", n_yy, yy1);
  grm_args_push(series[0], "ylabels", "nS", n_yy, yy1_labels);
  grm_args_push(series[1], "y", "nD", n_yy, yy2);
  grm_args_push(series[1], "ylabels", "nS", n_yy, yy2_labels);
  grm_args_push(series[2], "y", "nD", n_yy, yy3);
  grm_args_push(series[2], "ylabels", "nS", n_yy, yy3_labels);

  grm_args_push(args, "kind", "s", "barplot");
  grm_args_push(args, "style", "s", "lined");
  grm_args_push(args, "series", "nA", N_SERIES, series);
  grm_plot(args);
  sleep(3);

  /* Draw a 2D bar plot stacked */
  grm_args_push(args, "style", "s", "stacked");
  grm_plot(args);
  sleep(3);

  /* Draw a 2D bar plot with colorlist */
  grm_args_delete(args);
  args = grm_args_new();
  for (i = 0; i < N_SERIES; i++)
    {
      series[i] = grm_args_new();
    }
  grm_args_push(series[0], "y", "nD", n_yy, yy1);
  grm_args_push(series[0], "c", "nI", n_yy, c);
  grm_args_push(series[1], "y", "nD", n_yy, yy2);
  grm_args_push(series[2], "y", "nD", n_yy, yy3);

  grm_args_push(args, "kind", "s", "barplot");
  grm_args_push(args, "style", "s", "stacked");
  grm_args_push(args, "series", "nA", N_SERIES, series);
  grm_plot(args);
  sleep(3);

  /* Draw a 2D bar plot stacked with positive and negative values */
  /* The positive and negative values are stacked separately */
  grm_args_delete(args);
  args = grm_args_new();

  grm_args_push(args, "y", "nD", n_yy_pos_neg, yy_pos_neg);
  grm_args_push(args, "kind", "s", "barplot");
  grm_args_push(args, "style", "s", "stacked");
  grm_args_push(args, "ylabels", "nS", n_yy_pos_neg, yy_pos_neg_labels);
  grm_plot(args);
  sleep(3);

  /* Draw a bar plot that is lined and stacked with inner color list (rgb) */
  grm_args_delete(args);
  args = grm_args_new();
  for (i = 0; i < N_SERIES; i++)
    {
      series[i] = grm_args_new();
    }
  for (i = 0; i < INNER_N_SERIES; i++)
    {
      inner_series[i] = grm_args_new();
    }

  grm_args_push(inner_series[0], "y", "nD", n_inner_yy2_1, inner_yy2_1);
  grm_args_push(inner_series[0], "c", "nD", n_inner_c_rgb, inner_c_rgb);
  grm_args_push(inner_series[1], "y", "nD", 1, inner_yy2_2);
  grm_args_push(inner_series[2], "y", "nD", 1, inner_yy2_3);

  grm_args_push(series[0], "y", "nD", n_yy, yy1);
  grm_args_push(series[0], "c", "nD", 3 * n_yy, c_rgb);
  grm_args_push(series[1], "inner_series", "nA", INNER_N_SERIES, inner_series);
  grm_args_push(series[1], "c", "nD", 3 * n_yy, c_rgb);
  grm_args_push(series[2], "y", "nD", n_yy, yy3);
  grm_args_push(series[2], "c", "nD", 3 * n_yy, c_rgb);

  grm_args_push(args, "kind", "s", "barplot");
  grm_args_push(args, "style", "s", "lined");
  grm_args_push(args, "series", "nA", N_SERIES, series);

  grm_plot(args);
  sleep(3);

  /* Draw a bar plot that is lined and stacked with inner color list and ylabels */
  grm_args_delete(args);
  args = grm_args_new();
  for (i = 0; i < N_SERIES; i++)
    {
      series[i] = grm_args_new();
    }
  for (i = 0; i < INNER_N_SERIES; i++)
    {
      inner_series[i] = grm_args_new();
    }

  grm_args_push(inner_series[0], "y", "nD", n_inner_yy2_1, inner_yy2_1);
  grm_args_push(inner_series[0], "c", "nI", n_inner_c, inner_c);
  grm_args_push(inner_series[1], "y", "nD", 1, inner_yy2_2);
  grm_args_push(inner_series[2], "y", "nD", 1, inner_yy2_3);

  grm_args_push(series[0], "y", "nD", n_yy, yy1);
  grm_args_push(series[0], "ylabels", "nS", n_yy, yy1_labels);
  grm_args_push(series[1], "inner_series", "nA", INNER_N_SERIES, inner_series);
  /* ylabels for series containing inner_series can alternatively be put in each inner_series */
  grm_args_push(series[1], "ylabels", "nS", n_yy + 1, yy2_labels_for_inner);
  grm_args_push(series[2], "y", "nD", n_yy, yy3);
  grm_args_push(series[2], "ylabels", "nS", n_yy, yy3_labels);

  grm_args_push(args, "kind", "s", "barplot");
  grm_args_push(args, "style", "s", "lined");
  grm_args_push(args, "series", "nA", N_SERIES, series);

  grm_plot(args);
  sleep(3);

  grm_args_delete(args);
  grm_finalize();
}

static void test_shade(void)
{
  double plots[2][2][20];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  double markertypes[n];
  int markercolorinds[n];
  double markersizes[n];
  double markercolordoubs[n];
  const char *labels[] = {"sin", "cos"};
  grm_args_t *args, *series[2];
  int i;

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      plots[0][0][i] = i * 2 * M_PI / n;
      plots[0][1][i] = sin(i * 2 * M_PI / n);
    }
  for (i = 0; i < n; ++i)
    {
      plots[1][0][i] = i * 2 * M_PI / n;
      plots[1][1][i] = cos(i * 2 * M_PI / n);
    }

  for (i = 0; i < n; ++i)
    {
      markertypes[i] = rand() % 6 + (-32);
      markercolorinds[i] = rand() % (255);
      markercolordoubs[i] = rand() % 255 * 1.0;
      //      markersizes[i] = rand() % 255;
    }

  for (i = 0; i < 2; ++i)
    {
      series[i] = grm_args_new();
      grm_args_push(series[i], "x", "nD", n, plots[i][0]);
      grm_args_push(series[i], "y", "nD", n, plots[i][1]);
    }


  args = grm_args_new();
  grm_args_push(args, "series", "nA", 2, series);
  grm_args_push(args, "labels", "nS", 2, labels);
  grm_args_push(args, "kind", "s", "shade");

  printf("plotting data...\n");

  grm_plot(args);

  auto root = grm_get_document_root();
  std::cout << toXML(root) << std::endl;

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void testContext()
{
  GR::Context context = GR::Context();
  std::vector<std::string> vec = {"a", "b", "abc"};
  context["vec"] = vec;

  std::vector<std::string> res = GR::get<std::vector<std::string>>(context["vec"]);
  for (auto elem : res)
    {
      std::cout << elem << "\t";
    }
}

static void test_polarhistogram_subplots(void)
{
  grm_args_t *args, *subplots[2];
  int i, j;

  double theta[] = {0.1, 1.1, 5.4, 3.4, 2.3, 4.5, 3.2, 3.4, 5.6, 2.3, 2.1, 3.5, 0.6, 6.1};
  unsigned int theta_length = sizeof(theta) / sizeof(theta[0]);

  printf("Polar histogram with minimal input...\n");

  args = grm_args_new();

  subplots[0] = grm_args_new();
  subplots[1] = grm_args_new();

  grm_args_push(subplots[0], "kind", "s", "polar_histogram");
  grm_args_push(subplots[0], "x", "nD", theta_length, &theta);
  i = 0;
  //  grm_args_push(subplots[0], "subplot", "dddd", 0.5 * (i % 2), 0.5 * (i % 2 + 1), 0.5 * (i / 2), 0.5 * (i / 2 + 1));
  grm_args_push(subplots[0], "subplot", "dddd", 0.0, 0.5, 0.0, 1.0);
  i = 1;
  grm_args_push(subplots[1], "kind", "s", "polar_histogram");
  grm_args_push(subplots[1], "x", "nD", theta_length, &theta);
  //  grm_args_push(subplots[i], "subplot", "dddd", 0.5 * (i % 2), 0.5 * (i % 2 + 1), 0.5 * (i / 2), 0.5 * (i / 2 + 1));
  grm_args_push(subplots[i], "subplot", "dddd", 0.5, 1.0, 0.0, 1.0);

  args = grm_args_new();
  grm_args_push(args, "subplots", "nA", 2, subplots);

  printf("plotting data...\n");

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void test_interaction_on_tree(void)
{
  double plots[2][1000];
  int n = sizeof(plots[0]) / sizeof(plots[0][0]);
  grm_args_t *args;
  int i;

  printf("filling argument container...\n");

  for (i = 0; i < n; ++i)
    {
      plots[0][i] = i * 2 * M_PI / n;
      plots[1][i] = 2 * sin(i * 2 * M_PI / n);
    }

  args = grm_args_new();

  grm_args_push(args, "x", "nD", n, plots[0]);
  grm_args_push(args, "y", "nD", n, plots[1]);
  grm_plot(args);
  //    printf("Press any key to continue...\n");
  //    getchar();

  auto root = grm_get_document_root();
  auto render = grm_get_render();
  std::cout << toXML(root) << std::endl;

  auto subplot_element = get_subplot_from_ndc_point_using_dom(0.5, 0.5);
  std::cout << toXML(subplot_element) << std::endl;
  auto panzoom = render->createPanzoom(-0.04, 0.0, 0.8, 1.0);
  subplot_element->setAttribute("panzoom", true);
  subplot_element->appendChild(panzoom);

  gr_clearws();
  render->render();
  gr_updatews();

  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
  grm_finalize();
}

static void testPolar()
{
  grm_args_t *args, *subplots;
  int i, j;
  int n = 40;

  double angles[n], radii[n];
  double r_tick = 2.0 / n;
  double a_tick = 2 * M_PI / n;
  for (i = 0; i < n; ++i)
    {
      angles[i] = i * a_tick;
      radii[i] = i * r_tick;
    }
  angles[n - 1] = 2 * M_PI;
  radii[n - 1] = 2.0;

  printf("Polar plot\n");

  args = grm_args_new();

  subplots = grm_args_new();

  grm_args_push(args, "kind", "s", "polar");
  grm_args_push(args, "x", "nD", n, &angles);
  grm_args_push(args, "y", "nD", n, &radii);

  printf("plotting data...\n");

  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}
int main(void)
{

  testPolar();
  //  test_dom_render();
  //    test_polarhistogram_subplots();
  // test_wireframe();
  // test_plot3();
  //  testTrisurf();
  //    test_plot();
  //  test_shade();
  //  test_hist();
  //  test_contour();
  //  testBar();
  //  testContext();
  //  grm_finalize();
  test_interaction_on_tree();
  return 0;
}
