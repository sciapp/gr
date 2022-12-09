#include "grplot_mainwindow.hxx"
#include <QStatusBar>
#include <sstream>

GRPlotMainWindow::GRPlotMainWindow(int argc, char **argv) : QMainWindow()
{
  grplot_widget_ = new GRPlotWidget(this, argc, argv);
  setCentralWidget(grplot_widget_);
  setWindowTitle("GR Plot");
  resize(600, 450);
}

GRPlotMainWindow::~GRPlotMainWindow() = default;
