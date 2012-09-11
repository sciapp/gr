#include <QtGui>
#include "GR3Widget.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QMainWindow *window = new QMainWindow();

    window->setWindowTitle(QString::fromUtf8("GR3 in a QGLWidget"));

    GR3Widget* gr3widget = new GR3Widget(window);
    window->setCentralWidget(gr3widget);

    window->setMinimumSize(400,400);
    window->show();
    return app.exec();
}
