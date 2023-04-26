#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <list>
#include <sstream>
#include <QtGlobal>
#include <QApplication>
#include <QWindow>
#include <QToolTip>
#include <QTimer>
#include <QEvent>
#include <QRubberBand>
#include <functional>

#include "grplot_widget.hxx"
#include "util.hxx"

static std::string file_export;

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

std::function<void(const grm_event_t *)> size_callback;
extern "C" void size_callback_wrapper(const grm_event_t *cb)
{
  size_callback(cb);
}

std::function<void(const grm_cmd_event_t *)> cmd_callback;
extern "C" void cmd_callback_wrapper(const grm_event_t *event)
{
  cmd_callback(reinterpret_cast<const grm_cmd_event_t *>(event));
}


GRPlotWidget::GRPlotWidget(QMainWindow *parent, int argc, char **argv)
    : QWidget(parent), args_(nullptr), rubberBand(nullptr), pixmap(), redraw_pixmap(false), tooltip(nullptr)
{
  const char *kind;
  unsigned int z_length;
  double *z = nullptr;
  int error = 0;
  args_ = grm_args_new();

#ifdef _WIN32
  putenv("GKS_WSTYPE=381");
  putenv("GKS_DOUBLE_BUF=True");
#else
  setenv("GKS_WSTYPE", "381", 1);
  setenv("GKS_DOUBLE_BUF", "True", 1);
#endif

  rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
  setFocusPolicy(Qt::FocusPolicy::StrongFocus); // needed to receive key press events
  setMouseTracking(true);
  mouseState.mode = MouseState::Mode::normal;
  mouseState.pressed = {0, 0};
  mouseState.anchor = {0, 0};

  menu = parent->menuBar();
  export_menu = new QMenu("&Export");
  PdfAct = new QAction(tr("&PDF"), this);
  connect(PdfAct, &QAction::triggered, this, &GRPlotWidget::pdf);
  export_menu->addAction(PdfAct);
  PngAct = new QAction(tr("&PNG"), this);
  connect(PngAct, &QAction::triggered, this, &GRPlotWidget::png);
  export_menu->addAction(PngAct);
  JpegAct = new QAction(tr("&JPEG"), this);
  connect(JpegAct, &QAction::triggered, this, &GRPlotWidget::jpeg);
  export_menu->addAction(JpegAct);
  SvgAct = new QAction(tr("&SVG"), this);
  connect(SvgAct, &QAction::triggered, this, &GRPlotWidget::svg);
  export_menu->addAction(SvgAct);

  if (strcmp(argv[1], "--listen") == 0)
    {
      qRegisterMetaType<grm_args_t_wrapper>("grm_args_t_wrapper");
      receiver_thread = new Receiver_Thread();
      QObject::connect(receiver_thread, SIGNAL(resultReady(grm_args_t_wrapper)), this,
                       SLOT(received(grm_args_t_wrapper)), Qt::QueuedConnection);
      receiver_thread->start();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      ::size_callback = [this](auto &&PH1) { size_callback(std::forward<decltype(PH1)>(PH1)); };
      ::cmd_callback = [this](auto &&PH1) { cmd_callback(std::forward<decltype(PH1)>(PH1)); };
#else
      ::size_callback = std::bind(&GRPlotWidget::size_callback, this, std::placeholders::_1);
      ::cmd_callback = std::bind(&GRPlotWidget::cmd_callback, this, std::placeholders::_1);
#endif

      grm_register(GRM_EVENT_SIZE, size_callback_wrapper);
      grm_register(GRM_EVENT_CMD, cmd_callback_wrapper);
      grm_args_t_wrapper configuration;
      configuration.set_wrapper(grm_args_new());
      grm_args_push(configuration.get_wrapper(), "hold_plots", "i", 0);
      grm_merge(configuration.get_wrapper());
      grm_args_delete(configuration.get_wrapper());
    }
  else
    {
      grm_args_push(args_, "keep_aspect_ratio", "i", 1);
      if (!grm_interactive_plot_from_file(args_, argc, argv))
        {
          exit(0);
        }

      type = new QMenu("&Plot type");
      algo = new QMenu("&Algorithm");

      grm_args_values(args_, "kind", "s", &kind);
      if (grm_args_contains(args_, "error"))
        {
          error = 1;
          fprintf(stderr, "Plot types are not compatible with errorbars. The menu got disabled\n");
        }
      if (strcmp(kind, "contour") == 0 || strcmp(kind, "heatmap") == 0 || strcmp(kind, "imshow") == 0 ||
          strcmp(kind, "marginalheatmap") == 0 || strcmp(kind, "surface") == 0 || strcmp(kind, "wireframe") == 0 ||
          strcmp(kind, "contourf") == 0)
        {
          auto submenu = type->addMenu("&Marginalheatmap");

          heatmapAct = new QAction(tr("&Heatmap"), this);
          connect(heatmapAct, &QAction::triggered, this, &GRPlotWidget::heatmap);
          marginalheatmapAllAct = new QAction(tr("&Type 1 all"), this);
          connect(marginalheatmapAllAct, &QAction::triggered, this, &GRPlotWidget::marginalheatmapall);
          marginalheatmapLineAct = new QAction(tr("&Type 2 line"), this);
          connect(marginalheatmapLineAct, &QAction::triggered, this, &GRPlotWidget::marginalheatmapline);
          surfaceAct = new QAction(tr("&Surface"), this);
          connect(surfaceAct, &QAction::triggered, this, &GRPlotWidget::surface);
          wireframeAct = new QAction(tr("&Wireframe"), this);
          connect(wireframeAct, &QAction::triggered, this, &GRPlotWidget::wireframe);
          contourAct = new QAction(tr("&Contour"), this);
          connect(contourAct, &QAction::triggered, this, &GRPlotWidget::contour);
          imshowAct = new QAction(tr("&Imshow"), this);
          connect(imshowAct, &QAction::triggered, this, &GRPlotWidget::imshow);
          sumAct = new QAction(tr("&Sum"), this);
          connect(sumAct, &QAction::triggered, this, &GRPlotWidget::sumalgorithm);
          maxAct = new QAction(tr("&Maximum"), this);
          connect(maxAct, &QAction::triggered, this, &GRPlotWidget::maxalgorithm);
          contourfAct = new QAction(tr("&Contourf"), this);
          connect(contourfAct, &QAction::triggered, this, &GRPlotWidget::contourf);

          submenu->addAction(marginalheatmapAllAct);
          submenu->addAction(marginalheatmapLineAct);
          type->addAction(heatmapAct);
          type->addAction(surfaceAct);
          type->addAction(wireframeAct);
          type->addAction(contourAct);
          type->addAction(imshowAct);
          type->addAction(contourfAct);
          algo->addAction(sumAct);
          algo->addAction(maxAct);
        }
      else if (strcmp(kind, "line") == 0 ||
               (strcmp(kind, "scatter") == 0 && !grm_args_values(args_, "z", "D", &z, &z_length)))
        {
          lineAct = new QAction(tr("&Line"), this);
          connect(lineAct, &QAction::triggered, this, &GRPlotWidget::line);
          scatterAct = new QAction(tr("&Scatter"), this);
          connect(scatterAct, &QAction::triggered, this, &GRPlotWidget::scatter);
          type->addAction(lineAct);
          type->addAction(scatterAct);
        }
      else if (strcmp(kind, "volume") == 0 || strcmp(kind, "isosurface") == 0)
        {
          volumeAct = new QAction(tr("&Volume"), this);
          connect(volumeAct, &QAction::triggered, this, &GRPlotWidget::volume);
          isosurfaceAct = new QAction(tr("&Isosurface"), this);
          connect(isosurfaceAct, &QAction::triggered, this, &GRPlotWidget::isosurface);
          type->addAction(volumeAct);
          type->addAction(isosurfaceAct);
        }
      else if (strcmp(kind, "plot3") == 0 || strcmp(kind, "trisurf") == 0 || strcmp(kind, "tricont") == 0 ||
               strcmp(kind, "scatter3") == 0 || strcmp(kind, "scatter") == 0)
        {
          plot3Act = new QAction(tr("&Plot3"), this);
          connect(plot3Act, &QAction::triggered, this, &GRPlotWidget::plot3);
          trisurfAct = new QAction(tr("&Trisurf"), this);
          connect(trisurfAct, &QAction::triggered, this, &GRPlotWidget::trisurf);
          tricontAct = new QAction(tr("&Tricont"), this);
          connect(tricontAct, &QAction::triggered, this, &GRPlotWidget::tricont);
          scatter3Act = new QAction(tr("&Scatter3"), this);
          connect(scatter3Act, &QAction::triggered, this, &GRPlotWidget::scatter3);
          scatterAct = new QAction(tr("&Scatter"), this);
          connect(scatterAct, &QAction::triggered, this, &GRPlotWidget::scatter);
          type->addAction(plot3Act);
          type->addAction(trisurfAct);
          type->addAction(tricontAct);
          type->addAction(scatter3Act);
          type->addAction(scatterAct);
        }
      else if ((strcmp(kind, "hist") == 0 || strcmp(kind, "barplot") == 0 || strcmp(kind, "stairs") == 0 ||
                strcmp(kind, "stem") == 0) &&
               !error)
        {
          histAct = new QAction(tr("&Hist"), this);
          connect(histAct, &QAction::triggered, this, &GRPlotWidget::hist);
          barplotAct = new QAction(tr("&Barplot"), this);
          connect(barplotAct, &QAction::triggered, this, &GRPlotWidget::barplot);
          stairsAct = new QAction(tr("&Step"), this);
          connect(stairsAct, &QAction::triggered, this, &GRPlotWidget::stairs);
          stemAct = new QAction(tr("&Stem"), this);
          connect(stemAct, &QAction::triggered, this, &GRPlotWidget::stem);
          type->addAction(histAct);
          type->addAction(barplotAct);
          type->addAction(stairsAct);
          type->addAction(stemAct);
        }
      else if (strcmp(kind, "shade") == 0 || strcmp(kind, "hexbin") == 0)
        {
          shadeAct = new QAction(tr("&Shade"), this);
          connect(shadeAct, &QAction::triggered, this, &GRPlotWidget::shade);
          hexbinAct = new QAction(tr("&Hexbin"), this);
          connect(hexbinAct, &QAction::triggered, this, &GRPlotWidget::hexbin);
          type->addAction(shadeAct);
          type->addAction(hexbinAct);
        }
      menu->addMenu(type);
      menu->addMenu(algo);
    }
  menu->addMenu(export_menu);
}

GRPlotWidget::~GRPlotWidget()
{
  grm_args_delete(args_);
}

void GRPlotWidget::draw()
{
  if (!file_export.empty())
    {
      static char file[50];
      const char *kind;

      grm_args_values(args_, "kind", "s", &kind);
      snprintf(file, 50, "grplot_%s.%s", kind, file_export.c_str());
      grm_export(file);
    }
  grm_plot(nullptr);
}

void GRPlotWidget::redraw()
{
  redraw_pixmap = true;

  update();
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

void GRPlotWidget::paintEvent(QPaintEvent *event)
{
  util::unused(event);
  QPainter painter;
  std::stringstream addresses;
  const char *kind;

  QSize needed_pixmap_size = QSize((int)(geometry().width() * this->devicePixelRatioF()),
                                   (int)(geometry().height() * this->devicePixelRatioF()));

  if (pixmap.isNull() || pixmap.size() != needed_pixmap_size)
    {
      pixmap = QPixmap(needed_pixmap_size);
      pixmap.setDevicePixelRatio(this->devicePixelRatioF());
      redraw_pixmap = true;
    }

  if (redraw_pixmap)
    {
#ifdef _WIN32
      addresses << "GKS_CONID=";
#endif
      addresses << static_cast<void *>(this) << "!" << static_cast<void *>(&painter);
#ifdef _WIN32
      putenv(addresses.str().c_str());
#else
      setenv("GKS_CONID", addresses.str().c_str(), 1);
#endif

      painter.begin(&pixmap);

      painter.fillRect(0, 0, width(), height(), QColor("white"));
      draw();

      painter.end();
      redraw_pixmap = false;
    }

  painter.begin(this);
  painter.drawPixmap(0, 0, pixmap);
  if (tooltip != nullptr)
    {
      if (tooltip->x_px > 0 && tooltip->y_px > 0)
        {
          QColor background(224, 224, 224, 128);
          char c_info[BUFSIZ];
          QPainterPath triangle;
          std::string x_label = tooltip->xlabel, y_label = tooltip->ylabel;

          if (util::startsWith(x_label, "$") && util::endsWith(x_label, "$"))
            {
              x_label = "x";
            }
          if (util::startsWith(y_label, "$") && util::endsWith(y_label, "$"))
            {
              y_label = "y";
            }
          std::snprintf(c_info, BUFSIZ, tooltipTemplate, tooltip->label, x_label.c_str(), tooltip->x, y_label.c_str(),
                        tooltip->y);
          std::string info(c_info);
          label.setDefaultStyleSheet(style);
          label.setHtml(info.c_str());
          grm_args_values(args_, "kind", "s", &kind);
          if (strcmp(kind, "heatmap") == 0 || strcmp(kind, "marginalheatmap") == 0)
            {
              background.setAlpha(224);
            }
          painter.fillRect(tooltip->x_px + 8, (int)(tooltip->y_px - label.size().height() / 2),
                           (int)label.size().width(), (int)label.size().height(), QBrush(background, Qt::SolidPattern));

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

void GRPlotWidget::keyPressEvent(QKeyEvent *event)
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

void GRPlotWidget::mouseMoveEvent(QMouseEvent *event)
{
  const char *kind;
  if (!args_)
    {
      return;
    }
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
      if (grm_args_values(args_, "kind", "s", &kind))
        {
          tooltip = grm_get_tooltip(event->pos().x(), event->pos().y());

          if (strcmp(kind, "marginalheatmap") == 0)
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
}

void GRPlotWidget::mousePressEvent(QMouseEvent *event)
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

void GRPlotWidget::mouseReleaseEvent(QMouseEvent *event)
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

void GRPlotWidget::resizeEvent(QResizeEvent *event)
{
  grm_args_push(args_, "size", "dd", (double)event->size().width(), (double)event->size().height());
  grm_merge(args_);

  redraw();
}

void GRPlotWidget::wheelEvent(QWheelEvent *event)
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

void GRPlotWidget::mouseDoubleClickEvent(QMouseEvent *event)
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

void GRPlotWidget::heatmap()
{
  grm_args_push(args_, "kind", "s", "heatmap");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::marginalheatmapall()
{
  grm_args_push(args_, "kind", "s", "marginalheatmap");
  grm_args_push(args_, "marginalheatmap_kind", "s", "all");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::marginalheatmapline()
{
  grm_args_push(args_, "kind", "s", "marginalheatmap");
  grm_args_push(args_, "marginalheatmap_kind", "s", "line");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::line()
{
  grm_args_push(args_, "kind", "s", "line");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::sumalgorithm()
{
  grm_args_push(args_, "algorithm", "s", "sum");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::maxalgorithm()
{
  grm_args_push(args_, "algorithm", "s", "max");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::volume()
{
  grm_args_push(args_, "kind", "s", "volume");
  grm_merge(args_);
  redraw();
}
void GRPlotWidget::isosurface()
{
  grm_args_push(args_, "kind", "s", "isosurface");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::surface()
{
  grm_args_push(args_, "kind", "s", "surface");
  grm_merge(args_);
  redraw();
}
void GRPlotWidget::wireframe()
{
  grm_args_push(args_, "kind", "s", "wireframe");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::contour()
{
  grm_args_push(args_, "kind", "s", "contour");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::imshow()
{
  grm_args_push(args_, "kind", "s", "imshow");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::plot3()
{
  grm_args_push(args_, "kind", "s", "plot3");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::contourf()
{
  grm_args_push(args_, "kind", "s", "contourf");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::trisurf()
{
  grm_args_push(args_, "kind", "s", "trisurf");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::tricont()
{
  grm_args_push(args_, "kind", "s", "tricont");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::scatter3()
{
  grm_args_push(args_, "kind", "s", "scatter3");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::scatter()
{
  grm_args_push(args_, "kind", "s", "scatter");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::hist()
{
  grm_args_push(args_, "kind", "s", "hist");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::barplot()
{
  grm_args_push(args_, "kind", "s", "barplot");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::stairs()
{
  grm_args_push(args_, "kind", "s", "stairs");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::stem()
{
  grm_args_push(args_, "kind", "s", "stem");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::shade()
{
  grm_args_push(args_, "kind", "s", "shade");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::hexbin()
{
  grm_args_push(args_, "kind", "s", "hexbin");
  grm_merge(args_);
  redraw();
}

void GRPlotWidget::pdf()
{
  file_export = "pdf";
  redraw();
}

void GRPlotWidget::png()
{
  file_export = "png";
  redraw();
}

void GRPlotWidget::jpeg()
{
  file_export = "jpeg";
  redraw();
}

void GRPlotWidget::svg()
{
  file_export = "svg";
  redraw();
}

void GRPlotWidget::received(grm_args_t_wrapper args)
{
  if (!isVisible())
    {
      window()->show();
    }
  if (args_)
    {
      grm_args_delete(args_);
    }
  grm_switch(1);
  args_ = args.get_wrapper();
  grm_merge(args_);

  redraw();
}

void GRPlotWidget::closeEvent(QCloseEvent *event)
{
  event->ignore();
  hide();
  if (args_)
    {
      grm_args_delete(args_);
      args_ = nullptr;
    }
}

void GRPlotWidget::showEvent(QShowEvent *)
{
  QObject::connect(window()->windowHandle(), SIGNAL(screenChanged(QScreen *)), this, SLOT(screenChanged()));
}

void GRPlotWidget::screenChanged()
{
  redraw();
}

void GRPlotWidget::size_callback(const grm_event_t *new_size_object)
{
  // TODO: Get Plot ID
  if (this->size() != QSize(new_size_object->size_event.width, new_size_object->size_event.height))
    {
      this->window()->resize(new_size_object->size_event.width, new_size_object->size_event.height);
    }
}

void GRPlotWidget::cmd_callback(const grm_cmd_event_t *event)
{
  if (strcmp(event->cmd, "close") == 0)
    {
      QApplication::quit();
    }
}

void GRPlotWidget::cmd_callback(const grm_cmd_event_t *event)
{
  if (strcmp(event->cmd, "close") == 0)
    {
      QApplication::quit();
    }
}
