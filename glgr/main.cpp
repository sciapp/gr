#include <qapplication.h>

#include "glgr.h"

using namespace std;

int main(int argc, char **argv)
{
  QApplication *app;
  app = new QApplication(argc, argv);
  GLGrWidget *widget = new GLGrWidget();
  widget->show();
  return app->exec();
}
