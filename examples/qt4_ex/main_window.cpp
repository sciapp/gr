#include "main_window.h"
#include "gr.h"

MainWindow::MainWindow() : GRWidget() {

}

void MainWindow::draw() {
    double x[] = {0.1, 0.9, 0.9, 0.1, 0.1};
    double y[] = {0.1, 0.1, 0.9, 0.9, 0.1};

    gr_polyline(5, x, y);
}
