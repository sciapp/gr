#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "main_window.h"
#include "grm.h"

MainWindow::MainWindow() : QMainWindow()
{
  double plots[2][2][1000];
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);
  const char *labels[] = {"sin", "cos"};
  grm_args_t *args, *series[2];
  int i;

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

  for (i = 0; i < 2; ++i)
    {
      series[i] = grm_args_new();
      grm_args_push(series[i], "x", "nD", n, plots[i][0]);
      grm_args_push(series[i], "y", "nD", n, plots[i][1]);
    }

  args = grm_args_new();
  grm_args_push(args, "series", "nA", 2, series);
  grm_args_push(args, "labels", "nS", 2, labels);
  grm_args_push(args, "kind", "s", "line");
  grm_merge(args);

  grplot_widget_ = new GRPlotWidget(this, args);
  setCentralWidget(grplot_widget_);
  grplot_widget_->resize(600, 450);
  grplot_widget_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

  setWindowTitle("Minimal grplotWidget example");
  grplot_widget_->setMinimumSize(600, 450);
  adjustSize();
  grplot_widget_->setMinimumSize(0, 0);
}
