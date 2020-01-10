#include <QApplication>
#include "main_window.hxx"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  MainWindow window;

  window.show();

  return app.exec();
}
