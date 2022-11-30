#include "grplot_mainwindow.hxx"
#include <QStatusBar>
#include <sstream>

MainWindow::MainWindow(const char *csv_file, const char *plot_type, const char *colms) : QMainWindow()
{
  gr_widget_ = new GRWidget(this, csv_file, plot_type, colms);
  setCentralWidget(gr_widget_);
  setWindowTitle("GR Plot");
  resize(600, 450);
}

MainWindow::~MainWindow() = default;
