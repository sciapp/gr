#include <unistd.h>

#include <QtGlobal>
#if QT_VERSION >= 0x050000
    #include <QtWidgets/QApplication>
#else
    #include <QtGui/QApplication>
#endif

#include "gkswidget.h"

int main(int argc, char *argv[])
{
    daemon(0, 0);
    QApplication app(argc, argv);
    GKSWidget widget;
    return app.exec();
}
