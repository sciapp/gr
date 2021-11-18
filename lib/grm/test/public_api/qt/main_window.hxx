#ifndef MAIN_WINDOW_H_INCLUDED
#define MAIN_WINDOW_H_INCLUDED
#include <QMainWindow>
#include "gr_widget.hxx"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  virtual ~MainWindow();

public slots:
  void mouse_pos(const QPoint &pos);

private:
  GRWidget *gr_widget_;
};

#endif /* ifndef MAIN_WINDOW_H_INCLUDED */