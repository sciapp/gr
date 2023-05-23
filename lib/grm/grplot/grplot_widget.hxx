#ifndef GRPLOT_WIDGET_H_INCLUDED
#define GRPLOT_WIDGET_H_INCLUDED

#include <memory>
#include <variant>

#include <QMenu>
#include <QMenuBar>
#include <QRubberBand>
#include <QTextDocument>
#include <QWidget>
#include <QMainWindow>

#include "qtterm/receiver_thread.h"
#include "qtterm/grm_args_t_wrapper.h"
#include "util.hxx"
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
  void collectTooltips();
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
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
  void volume();
  void isosurface();
  void surface();
  void wireframe();
  void contour();
  void imshow();
  void plot3();
  void contourf();
  void trisurf();
  void tricont();
  void scatter3();
  void scatter();
  void hist();
  void barplot();
  void stairs();
  void stem();
  void shade();
  void hexbin();
  void pdf();
  void png();
  void jpeg();
  void svg();
  void received(grm_args_t_wrapper args);
  void screenChanged();

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

  class TooltipWrapper
  {
  public:
    TooltipWrapper(grm_tooltip_info_t *tooltip) : tooltip_(tooltip) {}
    TooltipWrapper(grm_accumulated_tooltip_info_t *accumulated_tooltip) : tooltip_(accumulated_tooltip) {}

    TooltipWrapper(const TooltipWrapper &) = delete;
    TooltipWrapper &operator=(const TooltipWrapper &) = delete;

    TooltipWrapper(TooltipWrapper &&tooltip_wrapper) { *this = std::move(tooltip_wrapper); }
    TooltipWrapper &operator=(TooltipWrapper &&tooltip_wrapper)
    {
      tooltip_ = std::move(tooltip_wrapper.tooltip_);
      tooltip_wrapper.tooltip_ = static_cast<grm_tooltip_info_t *>(nullptr);
      return *this;
    }

    ~TooltipWrapper()
    {
      if (holds_alternative<grm_accumulated_tooltip_info_t>())
        {
          auto accumulated_tooltip = get<grm_accumulated_tooltip_info_t>();
          std::free(accumulated_tooltip->y);
          std::free(accumulated_tooltip->ylabels);
        }
      std::visit([](auto *x) { std::free(x); }, tooltip_);
    }

    template <typename T> T *get() { return std::get<T *>(tooltip_); };
    template <typename T> const T *get() const { return std::get<T *>(tooltip_); };

    template <typename T> bool holds_alternative() const { return std::holds_alternative<T *>(tooltip_); };

    int n() const
    {
      return std::visit(util::overloaded{[](const grm_tooltip_info_t *) { return 1; },
                                         [](const grm_accumulated_tooltip_info_t *tooltip) { return tooltip->n; }},
                        tooltip_);
    };

    double x() const
    {
      return std::visit([](const auto *tooltip) { return tooltip->x; }, tooltip_);
    };

    int x_px() const
    {
      return std::visit([](const auto *tooltip) { return tooltip->x_px; }, tooltip_);
    };

    int y_px() const
    {
      return std::visit([](const auto *tooltip) { return tooltip->y_px; }, tooltip_);
    };

    const char *xlabel() const
    {
      return std::visit([](const auto *tooltip) { return tooltip->xlabel; }, tooltip_);
    };

  private:
    std::variant<grm_tooltip_info_t *, grm_accumulated_tooltip_info_t *> tooltip_;
  };

  QPixmap pixmap;
  bool redraw_pixmap;
  grm_args_t *args_;
  MouseState mouseState;
  QRubberBand *rubberBand;
  std::vector<TooltipWrapper> tooltips;
  QTextDocument label;
  Receiver_Thread *receiver_thread;

  QMenuBar *menu;
  QMenu *type;
  QMenu *algo;
  QMenu *export_menu;
  QAction *heatmapAct;
  QAction *marginalheatmapAllAct;
  QAction *marginalheatmapLineAct;
  QAction *lineAct;
  QAction *sumAct;
  QAction *maxAct;
  QAction *volumeAct;
  QAction *isosurfaceAct;
  QAction *surfaceAct;
  QAction *wireframeAct;
  QAction *contourAct;
  QAction *imshowAct;
  QAction *plot3Act;
  QAction *contourfAct;
  QAction *trisurfAct;
  QAction *tricontAct;
  QAction *scatter3Act;
  QAction *scatterAct;
  QAction *histAct;
  QAction *barplotAct;
  QAction *stairsAct;
  QAction *stemAct;
  QAction *shadeAct;
  QAction *hexbinAct;
  QAction *PdfAct;
  QAction *PngAct;
  QAction *JpegAct;
  QAction *SvgAct;

  void showEvent(QShowEvent *) override;
  void closeEvent(QCloseEvent *event) override;
  void size_callback(const grm_event_t *);
  void cmd_callback(const grm_cmd_event_t *);
};

#endif /* ifndef GRPLOT_WIDGET_H_INCLUDED */
