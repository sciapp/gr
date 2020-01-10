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
