#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <list>
#include <sstream>

#include "grmplots_widget.hxx"
#include "util.hxx"

void getMousePos(QMouseEvent *event, int *x, int *y)
{
#if QT_VERSION >= 0x060000
  x[0] = (int)event->position().x();
  y[0] = (int)event->position().y();
#else
  x[0] = (int)event->pos().x();
  y[0] = (int)event->pos().y();
#endif
}

void getWheelPos(QWheelEvent *event, int *x, int *y)
{
#if QT_VERSION >= 0x060000
  x[0] = (int)event->position().x();
  y[0] = (int)event->position().y();
#else
  x[0] = (int)event->pos().x();
  y[0] = (int)event->pos().y();
#endif
}

GRWidget::GRWidget(QMainWindow *parent, const char *csv_file, const char *plot_type, const char *colms)
    : QWidget(parent), args_(nullptr), rubberBand(nullptr), pixmap(nullptr), tooltip(nullptr)
{
  csv_file_ = csv_file;
  colms_ = colms;
  plot_type_ = plot_type;
  args_ = grm_args_new();

#ifdef _WIN32
  putenv("GKS_WSTYPE=381");
  putenv("GKS_DOUBLE_BUF=True");
#else
  setenv("GKS_WSTYPE", "381", 1);
  setenv("GKS_DOUBLE_BUF", "True", 1);
#endif
  if (!grm_interactive_plot_from_file(args_, csv_file_, &plot_type_, colms_, heatmap_type_, heatmap_algorithm_))
    {
      exit(0);
    }
  rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
  setFocusPolicy(Qt::FocusPolicy::StrongFocus); // needed to receive key press events
  setMouseTracking(true);
  mouseState.mode = MouseState::Mode::normal;
  mouseState.pressed = {0, 0};
  mouseState.anchor = {0, 0};

  menu = parent->menuBar();
  type = new QMenu("&Plot type");
  algo = new QMenu("&Algorithm");
  if (strcmp(plot_type_, "heatmap") == 0 || strcmp(plot_type_, "marginalheatmap") == 0)
    {
      auto submenu = type->addMenu("&Marginalheatmap");

      heatmapAct = new QAction(tr("&Heatmap"), this);
      connect(heatmapAct, &QAction::triggered, this, &GRWidget::heatmap);
      marginalheatmapAllAct = new QAction(tr("&Type 1 all"), this);
      connect(marginalheatmapAllAct, &QAction::triggered, this, &GRWidget::marginalheatmapall);
      marginalheatmapLineAct = new QAction(tr("&Type 2 line"), this);
      connect(marginalheatmapLineAct, &QAction::triggered, this, &GRWidget::marginalheatmapline);
      sumAct = new QAction(tr("&Sum"), this);
      connect(sumAct, &QAction::triggered, this, &GRWidget::sumalgorithm);
      maxAct = new QAction(tr("&Maximum"), this);
      connect(maxAct, &QAction::triggered, this, &GRWidget::maxalgorithm);

      submenu->addAction(marginalheatmapAllAct);
      submenu->addAction(marginalheatmapLineAct);
      type->addAction(heatmapAct);
      algo->addAction(sumAct);
      algo->addAction(maxAct);
    }
  else if (strcmp(plot_type_, "line") == 0)
    {
      lineAct = new QAction(tr("&Line"), this);
      connect(lineAct, &QAction::triggered, this, &GRWidget::line);
      type->addAction(lineAct);
    }
  menu->addMenu(type);
  menu->addMenu(algo);
}

GRWidget::~GRWidget()
{
  grm_args_delete(args_);
}

void GRWidget::draw()
{
  grm_plot(nullptr);
}

void GRWidget::redraw()
{
  delete pixmap;
  pixmap = nullptr;
  repaint();
}

#define style \
  "\
    .gr-label {\n\
        color: #26aae1;\n\
        font-size: 11px;\n\
        line-height: 0.8;\n\
    }\n\
    .gr-value {\n\
        color: #3c3c3c;\n\
        font-size: 11px;\n\
        line-height: 0.8;\n\
    }"

#define tooltipTemplate \
  "\
    <span class=\"gr-label\">%s</span><br>\n\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span><br>\n\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span>"

void GRWidget::paintEvent(QPaintEvent *event)
{
  util::unused(event);
  QPainter painter;
  std::stringstream addresses;

  if (!pixmap)
    {
      pixmap = new QPixmap((int)(geometry().width() * this->devicePixelRatioF()),
                           (int)(geometry().height() * this->devicePixelRatioF()));
      pixmap->setDevicePixelRatio(this->devicePixelRatioF());

      addresses << static_cast<void *>(this) << "!" << static_cast<void *>(&painter);
#ifdef _WIN32
      putenv(addresses.str().c_str());
#else
      setenv("GKS_CONID", addresses.str().c_str(), 1);
#endif

      painter.begin(pixmap);

      painter.fillRect(0, 0, width(), height(), QColor("white"));
      draw();

      painter.end();
    }
  if (pixmap)
    {
      painter.begin(this);
      painter.drawPixmap(0, 0, *pixmap);
      if (tooltip != nullptr)
        {
          if (tooltip->x_px > 0 && tooltip->y_px > 0)
            {
              QColor background(224, 224, 224, 128);
              char c_info[BUFSIZ];
              QPainterPath triangle;
              std::string x_label = tooltip->xlabel, y_label = tooltip->ylabel;

              if (startsWith(x_label, "$") && endsWith(x_label, "$"))
                {
                  x_label = "x";
                }
              if (startsWith(y_label, "$") && endsWith(y_label, "$"))
                {
                  y_label = "y";
                }
              std::snprintf(c_info, BUFSIZ, tooltipTemplate, tooltip->label, x_label.c_str(), tooltip->x,
                            y_label.c_str(), tooltip->y);
              std::string info(c_info);
              label.setDefaultStyleSheet(style);
              label.setHtml(info.c_str());
              if (strcmp(plot_type_, "heatmap") == 0 || strcmp(plot_type_, "marginalheatmap") == 0)
                {
                  background.setAlpha(224);
                }
              painter.fillRect(tooltip->x_px + 8, (int)(tooltip->y_px - label.size().height() / 2),
                               (int)label.size().width(), (int)label.size().height(),
                               QBrush(background, Qt::SolidPattern));

              triangle.moveTo(tooltip->x_px, tooltip->y_px);
              triangle.lineTo(tooltip->x_px + 8, tooltip->y_px + 6);
              triangle.lineTo(tooltip->x_px + 8, tooltip->y_px - 6);
              triangle.closeSubpath();
              background.setRgb(128, 128, 128, 128);
              painter.fillPath(triangle, QBrush(background, Qt::SolidPattern));

              painter.save();
              painter.translate(tooltip->x_px + 8, tooltip->y_px - label.size().height() / 2);
              label.drawContents(&painter);
              painter.restore();
            }
        }
      painter.end();
    }
}

void GRWidget::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_R)
    {
      grm_args_t *args = grm_args_new();
      QPoint widget_cursor_pos = mapFromGlobal(QCursor::pos());
      grm_args_push(args, "key", "s", "r");
      grm_args_push(args, "x", "i", widget_cursor_pos.x());
      grm_args_push(args, "y", "i", widget_cursor_pos.y());
      grm_input(args);
      grm_args_delete(args);
      redraw();
    }
}

void GRWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (mouseState.mode == MouseState::Mode::boxzoom)
    {
      rubberBand->setGeometry(QRect(mouseState.pressed, event->pos()).normalized());
    }
  else if (mouseState.mode == MouseState::Mode::pan)
    {
      int x, y;
      getMousePos(event, &x, &y);
      grm_args_t *args = grm_args_new();

      grm_args_push(args, "x", "i", mouseState.anchor.x());
      grm_args_push(args, "y", "i", mouseState.anchor.y());
      grm_args_push(args, "xshift", "i", x - mouseState.anchor.x());
      grm_args_push(args, "yshift", "i", y - mouseState.anchor.y());

      grm_input(args);
      grm_args_delete(args);

      mouseState.anchor = event->pos();
      redraw();
    }
  else
    {
      tooltip = grm_get_tooltip(event->pos().x(), event->pos().y());

      if (strcmp(plot_type_, "marginalheatmap") == 0)
        {
          grm_args_t *input_args;
          input_args = grm_args_new();

          grm_args_push(input_args, "x", "i", event->pos().x());
          grm_args_push(input_args, "y", "i", event->pos().y());
          grm_input(input_args);
        }

      redraw();
    }
}

void GRWidget::mousePressEvent(QMouseEvent *event)
{
  mouseState.pressed = event->pos();
  if (event->button() == Qt::MouseButton::RightButton)
    {
      mouseState.mode = MouseState::Mode::boxzoom;
      rubberBand->setGeometry(QRect(mouseState.pressed, QSize()));
      rubberBand->show();
    }
  else if (event->button() == Qt::MouseButton::LeftButton)
    {
      mouseState.mode = MouseState::Mode::pan;
      mouseState.anchor = event->pos();
    }
}

void GRWidget::mouseReleaseEvent(QMouseEvent *event)
{
  grm_args_t *args = grm_args_new();
  int x, y;
  getMousePos(event, &x, &y);

  if (mouseState.mode == MouseState::Mode::boxzoom)
    {
      rubberBand->hide();
      if (std::abs(x - mouseState.pressed.x()) >= 5 && std::abs(y - mouseState.pressed.y()) >= 5)
        {
          grm_args_push(args, "keep_aspect_ratio", "i", event->modifiers() & Qt::ShiftModifier);
          grm_args_push(args, "x1", "i", mouseState.pressed.x());
          grm_args_push(args, "y1", "i", mouseState.pressed.y());
          grm_args_push(args, "x2", "i", x);
          grm_args_push(args, "y2", "i", y);
        }
    }
  else if (mouseState.mode == MouseState::Mode::pan)
    {
      mouseState.mode = MouseState::Mode::normal;
    }

  grm_input(args);
  grm_args_delete(args);

  redraw();
}

void GRWidget::resizeEvent(QResizeEvent *event)
{
  grm_args_push(args_, "size", "dd", (double)event->size().width(), (double)event->size().height());
  grm_merge(args_);

  redraw();
}

void GRWidget::wheelEvent(QWheelEvent *event)
{
  int x, y;
  getWheelPos(event, &x, &y);

  grm_args_t *args = grm_args_new();
  grm_args_push(args, "x", "i", x);
  grm_args_push(args, "y", "i", y);
  grm_args_push(args, "angle_delta", "d", (double)event->angleDelta().y());
  grm_input(args);
  grm_args_delete(args);

  redraw();
}

void GRWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  grm_args_t *args = grm_args_new();
  QPoint pos = mapFromGlobal(QCursor::pos());
  grm_args_push(args, "key", "s", "r");
  grm_args_push(args, "x", "i", pos.x());
  grm_args_push(args, "y", "i", pos.y());
  grm_input(args);
  grm_args_delete(args);

  redraw();
}

void GRWidget::heatmap()
{
  plot_type_ = "heatmap";
  if (!grm_interactive_plot_from_file(args_, csv_file_, &plot_type_, colms_, heatmap_type_, heatmap_algorithm_))
    {
      exit(0);
    }
  redraw();
}

void GRWidget::marginalheatmapall()
{
  plot_type_ = "marginalheatmap";
  heatmap_type_ = "all";
  if (!grm_interactive_plot_from_file(args_, csv_file_, &plot_type_, colms_, heatmap_type_, heatmap_algorithm_))
    {
      exit(0);
    }
  redraw();
}

void GRWidget::marginalheatmapline()
{
  plot_type_ = "marginalheatmap";
  heatmap_type_ = "line";
  if (!grm_interactive_plot_from_file(args_, csv_file_, &plot_type_, colms_, heatmap_type_, heatmap_algorithm_))
    {
      exit(0);
    }
  redraw();
}

void GRWidget::line()
{
  plot_type_ = "line";
  if (!grm_interactive_plot_from_file(args_, csv_file_, &plot_type_, colms_, heatmap_type_, heatmap_algorithm_))
    {
      exit(0);
    }
  redraw();
}

void GRWidget::sumalgorithm()
{
  heatmap_algorithm_ = "sum";
  if (!grm_interactive_plot_from_file(args_, csv_file_, &plot_type_, colms_, heatmap_type_, heatmap_algorithm_))
    {
      exit(0);
    }
  redraw();
}

void GRWidget::maxalgorithm()
{
  heatmap_algorithm_ = "max";
  if (!grm_interactive_plot_from_file(args_, csv_file_, &plot_type_, colms_, heatmap_type_, heatmap_algorithm_))
    {
      exit(0);
    }
  redraw();
}
