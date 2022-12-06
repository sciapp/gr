#ifndef GRPLOT_MAIN_WINDOW_H_INCLUDED
#define GRPLOT_MAIN_WINDOW_H_INCLUDED
#include "grplot_widget.hxx"
#include <QMainWindow>

class GRPlotMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit GRPlotMainWindow(int argc, char **argv);
  ~GRPlotMainWindow() override;

private:
  GRPlotWidget *grplot_widget_;
};

#endif /* ifndef GRPLOT_MAIN_WINDOW_H_INCLUDED */
