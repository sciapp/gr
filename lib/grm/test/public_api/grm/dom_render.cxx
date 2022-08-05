#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <random>

#include "grm.h"
#include <grm/dom_render/graphics_tree/util.hxx>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define LENGTH 2000
#define NBINS 20


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
      //      markersizes[i] = rand() % 255;
    }

  for (i = 0; i < 2; ++i)
    {
      series[i] = grm_args_new();
      grm_args_push(series[i], "x", "nD", n, plots[i][0]);
      grm_args_push(series[i], "y", "nD", n, plots[i][1]);

      //      grm_args_push(series[i], "c", "nD", n, markercolordoubs);
      //      grm_args_push(series[i], "markertype", "nD", n, markertypes);
      //      grm_args_push(series[i], "z", "nD", n, markersizes);
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
  auto render = GR::Render::createRender();
  auto context = std::make_shared<GR::Context>(GR::Context());
  for (const auto &elem : root->querySelectorsAll("polymarker"))
    {
      elem->setAttribute("markertype", 2);
      //      std::vector<double> v(markersizes, markersizes + sizeof markersizes / sizeof markersizes[0]);
      //      render->setMarkerSize(elem, "sizes", v, context);
      //      break;
    }

  std::cout << toXML(root) << std::endl;

  gr_clearws();
  grm_render();
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


int main(void)
{

  //    test_dom_render();
  // test_wireframe();
  // test_plot3();
  testTrisurf();
  //  test_plot();
  //  test_hist();
  //  test_contour();
  grm_finalize();

  return 0;
}