#ifndef MAIN_WINDOW_H_INCLUDED
#define MAIN_WINDOW_H_INCLUDED

#include "grplot_widget.hxx"

class MainWindow : public QMainWindow
{
public:
  MainWindow();

private:
  GRPlotWidget *grplot_widget_;
};

#endif /* ifndef MAIN_WINDOW_H_INCLUDED */
