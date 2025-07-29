#ifndef GRPLOTDOCKWIDGET_HXX
#define GRPLOTDOCKWIDGET_HXX

#include <QDockWidget>

class GRPlotMainWindow;
#include "grplotMainwindow.hxx"

class GRPlotDockWidget : public QDockWidget
{

  Q_OBJECT

public:
  explicit GRPlotDockWidget(const QString &title = "", int width = 200, int height = 450,
                            GRPlotMainWindow *parent = nullptr);
  ~GRPlotDockWidget() override;
  void closeEvent(QCloseEvent *event) override;

protected:
signals:
  void resizeMainWindow();

private:
  GRPlotMainWindow *parent;
};


#endif // GRPLOTDOCKWIDGET_HXX
