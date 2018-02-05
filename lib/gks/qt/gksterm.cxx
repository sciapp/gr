#include <QtWidgets/QApplication>

#include "gkswidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    GKSWidget widget;
    return app.exec();
}
