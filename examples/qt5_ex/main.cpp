#include <QApplication>
#include "main_window.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    MainWindow window;

    window.resize(800, 800);
    window.setWindowTitle("Minimal Qt5 GR example");
    window.show();

    return app.exec();
}
