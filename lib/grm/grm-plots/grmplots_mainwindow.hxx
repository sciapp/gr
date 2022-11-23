#ifndef MAIN_WINDOW_H_INCLUDED
#define MAIN_WINDOW_H_INCLUDED
#include "grmplots_widget.hxx"
#include <QMainWindow>

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(const char *csv_file, const char *plot_type, const char *colms);
  ~MainWindow() override;

private:
  GRWidget *gr_widget_;
};

#endif /* ifndef MAIN_WINDOW_H_INCLUDED */
