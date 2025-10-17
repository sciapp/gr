#include "GRPlotDockWidget.hxx"

GRPlotDockWidget::GRPlotDockWidget(const QString &title, int width, int height, GRPlotMainWindow *parent)
    : QDockWidget(parent), parent(parent)
{
  this->setWindowTitle(title);
  this->setFixedWidth(width);
  this->setMinimumHeight(height);
  this->setMinimumHeight(0);
}

GRPlotDockWidget::~GRPlotDockWidget() = default;

void GRPlotDockWidget::closeEvent(QCloseEvent *event)
{
  emit resizeMainWindow();
  QDockWidget::closeEvent(event);
}
