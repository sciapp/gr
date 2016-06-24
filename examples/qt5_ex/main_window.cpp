#include <QPainter>
#include <cstdlib>
#include "main_window.h"
#include "gr.h"

MainWindow::MainWindow() : QMainWindow() {
#ifdef _WIN32
    putenv("GKS_WSTYPE=381");
    putenv("GKS_DOUBLE_BUF=True");
#else
    setenv("GKS_WSTYPE", "381", 1);
    setenv("GKS_DOUBLE_BUF", "True", 1);
#endif
}

void MainWindow::paintEvent(QPaintEvent* event) {
    QPainter painter;
    char buf[100];

#ifdef _WIN32
    sprintf(buf, "GKS_CONID=%p!%p", this, &painter);
    putenv(buf);
#else
    sprintf(buf, "%p!%p", this, &painter);
    setenv("GKS_CONID", buf, 1);
#endif

    painter.begin(this);
    painter.fillRect(0, 0, width(), height(), QColor("white"));

    draw();

    painter.end();
}

void MainWindow::draw() {
    double x[] = {0.1, 0.9, 0.9, 0.1, 0.1};
    double y[] = {0.1, 0.1, 0.9, 0.9, 0.1};

    gr_clearws();
    gr_polyline(5, x, y);
    gr_updatews();
}
