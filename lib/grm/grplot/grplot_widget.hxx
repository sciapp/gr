#ifndef GR_WIDGET_H_INCLUDED
#define GR_WIDGET_H_INCLUDED

#include <QMenu>
#include <QMenuBar>
#include <QRubberBand>
#include <QTextDocument>
#include <QWidget>
#include <QMainWindow>

#include <grm.h>

class GRWidget : public QWidget
{
  Q_OBJECT

public:
  explicit GRWidget(QMainWindow *parent, const char *csv_file, const char *plot_type, const char *colms);
  ~GRWidget() override;

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

  const char *csv_file_;
  const char *plot_type_;
  const char *heatmap_type_ = "all";
  const char *heatmap_algorithm_ = "sum";
  const char *colms_;
};

#endif /* ifndef GR_WIDGET_H_INCLUDED */
