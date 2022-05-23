#include <sstream>
#include <QStatusBar>
#include "util.hxx"
#include "main_window.hxx"

MainWindow::MainWindow() : QMainWindow()
{
  gr_widget_ = new GRWidget(this);
  QObject::connect(gr_widget_, &GRWidget::mouse_pos_changed, this, &MainWindow::mouse_pos);
  setCentralWidget(gr_widget_);
  setWindowTitle("Qt GR test program");
  resize(800, 800);
  statusBar();
}

MainWindow::~MainWindow() {}

void MainWindow::mouse_pos(const QPoint &pos)
{
  std::stringstream pos_stream;
  pos_stream << pos;
  statusBar()->showMessage(pos_stream.str().c_str());
}
