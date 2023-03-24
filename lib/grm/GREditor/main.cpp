#include <QApplication>
#include "main_window.hxx"


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  MainWindow window;
  window.setWindowTitle("GREditor");
  //    window.resize(1.2 * 640, 1.2 * 480);
  window.show();

  return app.exec();
}
