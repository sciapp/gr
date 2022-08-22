#include <QApplication>
#include "main_window.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  MainWindow window;

  window.resize(500, 500);
  window.setWindowTitle("qtTerm");

  return QApplication::exec();
}
