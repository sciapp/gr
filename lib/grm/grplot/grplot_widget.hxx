#ifndef GRPLOT_WIDGET_H_INCLUDED
#define GRPLOT_WIDGET_H_INCLUDED

#include <QMenu>
#include <QMenuBar>
#include <QRubberBand>
#include <QTextDocument>
#include <QWidget>
#include <QMainWindow>

#include <grm.h>

class GRPlotWidget : public QWidget
{
  Q_OBJECT

public:
  explicit GRPlotWidget(QMainWindow *parent, int argc, char **argv);
  ~GRPlotWidget() override;

protected:
  virtual void draw();
  void redraw();
  void keyPressEvent(QKeyEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
  void heatmap();
  void marginalheatmapall();
  void marginalheatmapline();
  void line();
  void sumalgorithm();
  void maxalgorithm();

private:
  struct MouseState
  {
    enum class Mode
    {
      normal,
      pan,
      boxzoom
    };
    Mode mode;
    QPoint pressed;
    QPoint anchor;
  };
  QPixmap *pixmap;
  grm_args_t *args_;
  MouseState mouseState;
  QRubberBand *rubberBand;
  grm_tooltip_info_t *tooltip;
  QTextDocument label;

  QMenuBar *menu;
  QMenu *type;
  QMenu *algo;
  QAction *heatmapAct;
  QAction *marginalheatmapAllAct;
  QAction *marginalheatmapLineAct;
  QAction *lineAct;
  QAction *sumAct;
  QAction *maxAct;
};

#endif /* ifndef GRPLOT_WIDGET_H_INCLUDED */
