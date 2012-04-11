#include <QApplication>

#include "gkswidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GKSQtWindow mainWindow;
    mainWindow.setWindowIcon(QIcon("./images/gksqt.png"));
    mainWindow.show();

    return app.exec();
}
