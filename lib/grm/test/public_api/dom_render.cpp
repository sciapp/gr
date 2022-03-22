#ifdef __unix__
#define _XOPEN_SOURCE 500
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "grm.h"
#include "GR/util.hxx"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


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
    }

  for (i = 0; i < 2; ++i)
    {
      series[i] = grm_args_new();
      grm_args_push(series[i], "x", "nD", n, plots[i][0]);
      grm_args_push(series[i], "y", "nD", n, plots[i][1]);
      //      grm_args_push(series[i], "c", "nD", n, markercolordoubs);
      //      grm_args_push(series[i], "markertype", "nD", n, markertypes);
    }


  args = grm_args_new();
  grm_args_push(args, "series", "nA", 2, series);
  grm_args_push(args, "labels", "nS", 2, labels);
  grm_args_push(args, "kind", "s", "stem");

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
  for (const auto &elem : root->querySelectorsAll("group"))
    {
      //! No access to dom doc! Cant manipulate grm_doc's context => Use external context instead!
      //! TODO: Possible Conflicts: When using external contexts fallback to internal context when things are missing?
      //! Or explicit use of only one context -> Context copy constructor!
      std::vector<double> v(markersizes, markersizes + sizeof markersizes / sizeof markersizes[0]);
      render->setMarkerSize(elem, "sizes", v, context);
      break;
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


  grm_plot(args);
  printf("Press any key to continue...\n");
  getchar();

  grm_args_delete(args);
}

static void test_plot(void)
{
  test_subplots();
  grm_finalize();
}


int main(void)
{
  //  test_dom_render();
  test_plot();
  grm_finalize();

  return 0;
}
