#include "grplot_mainwindow.hxx"
#include <QApplication>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  GRPlotMainWindow window(argc, argv);

  window.show();

  return app.exec();
}
