#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <list>
#include <sstream>

#include <QInputDialog>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QtWidgets>
#include <QFileDialog>
#include <QMessageBox>
#include <cfloat>
#include <QtGlobal>
#include <QApplication>
#include <QToolTip>
#include <QTimer>
#include <QEvent>
#include <QRubberBand>
#include <functional>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

#include "grplot_widget.hxx"
#include "util.hxx"

static std::string file_export;
static bool kind_changed = false;

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

GRPlotWidget::GRPlotWidget(QMainWindow *parent, int argc, char **argv)
    : QWidget(parent), args_(nullptr), rubberBand(nullptr), pixmap(nullptr), tooltip(nullptr)
{
  const char *kind;
  unsigned int z_length;
  double *z = nullptr;
  int error = 0;
  args_ = grm_args_new();

  enable_editor = false;
  highlightBoundingObjects = false;
  bounding_logic = new Bounding_logic();
  current_selection = nullptr;
  amount_scrolled = 0;
  treewidget = new TreeWidget();
  treewidget->hide();

#ifdef _WIN32
  putenv("GKS_WSTYPE=381");
  putenv("GKS_DOUBLE_BUF=True");
#else
  setenv("GKS_WSTYPE", "381", 1);
  setenv("GKS_DOUBLE_BUF", "True", 1);
#endif
  grm_args_push(args_, "keep_aspect_ratio", "i", 1);
  if (!grm_interactive_plot_from_file(args_, argc, argv))
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
  menu->addMenu(export_menu);

  if (getenv("GRPLOT_ENABLE_EDITOR"))
    {
      editor_menu = new QMenu(tr("&Editor"));
      editor_action = new QAction(tr("&Activate Editor"));
      editor_action->setCheckable(true);
      editor_menu->addAction(editor_action);
      QObject::connect(editor_action, SIGNAL(triggered()), this, SLOT(enable_editor_functions()));

      auto file_menu = editor_menu->addMenu(tr("&File"));
      save_file_action = new QAction("&Save Plot");
      save_file_action->setShortcut(Qt::CTRL | Qt::Key_S);
      file_menu->addAction(save_file_action);
      QObject::connect(save_file_action, SIGNAL(triggered()), this, SLOT(save_file_slot()));

      open_file_action = new QAction("&Open Plot");
      open_file_action->setShortcut(Qt::CTRL | Qt::Key_O);
      file_menu->addAction(open_file_action);
      QObject::connect(open_file_action, SIGNAL(triggered()), this, SLOT(open_file_slot()));

      auto configuration_menu = editor_menu->addMenu(tr("&Show"));
      show_container_action = new QAction(tr("&GRM Container"));
      show_container_action->setCheckable(true);
      show_container_action->setShortcut(Qt::CTRL | Qt::Key_C);
      configuration_menu->addAction(show_container_action);
      QObject::connect(show_container_action, SIGNAL(triggered()), this, SLOT(show_container_slot()));

      show_bounding_boxes_action = new QAction(tr("&Bounding Boxes"));
      show_bounding_boxes_action->setCheckable(true);
      show_bounding_boxes_action->setShortcut(Qt::CTRL | Qt::Key_B);
      configuration_menu->addAction(show_bounding_boxes_action);
      QObject::connect(show_bounding_boxes_action, SIGNAL(triggered()), this, SLOT(show_bounding_boxes_slot()));

      menu->addMenu(editor_menu);
    }
}

GRPlotWidget::~GRPlotWidget()
{
  grm_args_delete(args_);
  grm_finalize();
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
  if (kind_changed)
    {
      grm_plot(args_);
      kind_changed = false;
    }
  else
    {
      grm_plot(nullptr);
    }
}

void GRPlotWidget::redraw()
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

void GRPlotWidget::paintEvent(QPaintEvent *event)
{
  util::unused(event);
  QPainter painter;
  std::stringstream addresses;
  const char *kind;

  if (!pixmap)
    {
      pixmap = new QPixmap((int)(geometry().width() * this->devicePixelRatioF()),
                           (int)(geometry().height() * this->devicePixelRatioF()));
      pixmap->setDevicePixelRatio(this->devicePixelRatioF());

#ifdef _WIN32
      addresses << "GKS_CONID=";
#endif
      addresses << static_cast<void *>(this) << "!" << static_cast<void *>(&painter);
#ifdef _WIN32
      putenv(addresses.str().c_str());
#else
      setenv("GKS_CONID", addresses.str().c_str(), 1);
#endif

      painter.begin(pixmap);

      painter.fillRect(0, 0, width(), height(), QColor("white"));
      painter.save();
      draw();
      painter.restore();
      painter.end();

      treewidget->updateData(grm_get_document_root());
    }
  if (pixmap)
    {
      painter.begin(this);
      painter.drawPixmap(0, 0, *pixmap);
      bounding_logic->clear();
      extract_bounding_boxes_from_grm((QPainter &)painter);
      highlight_current_selection((QPainter &)painter);
      if (tooltip != nullptr && !enable_editor)
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
              std::snprintf(c_info, BUFSIZ, tooltipTemplate, tooltip->label, x_label.c_str(), tooltip->x,
                            y_label.c_str(), tooltip->y);
              std::string info(c_info);
              label.setDefaultStyleSheet(style);
              label.setHtml(info.c_str());
              grm_args_values(args_, "kind", "s", &kind);
              if (strcmp(kind, "heatmap") == 0 || strcmp(kind, "marginalheatmap") == 0)
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

void GRPlotWidget::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_R)
    {
      if (enable_editor)
        {
          reset_pixmap();
        }
      else
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
  if (enable_editor)
    {
      if (event->key() == Qt::Key_Escape)
        {
          current_selection = nullptr;
          mouse_move_selection = nullptr;
          update();
        }
      else if (event->key() == Qt::Key_Space)
        {
          show_bounding_boxes_action->trigger();
        }
      else if (event->key() == Qt::Key_Return)
        {
          if (current_selection == nullptr)
            {
              return;
            }
          std::string currently_clicked_name = current_selection->get_ref()->localName();

          QDialog dialog(this);
          QFormLayout form(&dialog);
          QString title("Selected: ");
          title.append(currently_clicked_name.c_str());
          dialog.setWindowTitle(title);
          auto changeParametersLabel = new QLabel("Change Parameters:");
          changeParametersLabel->setStyleSheet("font-weight: bold");
          form.addRow(changeParametersLabel);

          QList<QString> labels;
          QList<QWidget *> fields;
          QWidget *lineEdit;

          for (const auto &cur_attr_name : current_selection->get_ref()->getAttributeNames())
            {
              if (cur_attr_name == "bbox_id" || cur_attr_name == "bbox_xmin" || cur_attr_name == "bbox_xmax" ||
                  cur_attr_name == "bbox_ymin" || cur_attr_name == "bbox_ymax")
                {
                  continue;
                }
              if (cur_attr_name == "render_method")
                {
                  lineEdit = new QComboBox(&dialog);
                  QString tooltipString =
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
                  tooltipString.append(" Default: ");
                  tooltipString.append(
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
                  ((QComboBox *)lineEdit)->setToolTip(tooltipString);
                  ((QComboBox *)lineEdit)->addItem("gr_text");     // 0
                  ((QComboBox *)lineEdit)->addItem("gr_mathtext"); // 1
                  ((QComboBox *)lineEdit)->addItem("gr_textext");  // 2
                  ((QComboBox *)lineEdit)
                      ->setCurrentIndex(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
                }
              else if (cur_attr_name == "textalign_vertical")
                {
                  lineEdit = new QComboBox(&dialog);
                  QString tooltipString =
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
                  tooltipString.append(" Default: ");
                  tooltipString.append(
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
                  ((QComboBox *)lineEdit)->setToolTip(tooltipString);
                  ((QComboBox *)lineEdit)->addItem("normal"); // 0
                  ((QComboBox *)lineEdit)->addItem("top");    // 1
                  ((QComboBox *)lineEdit)->addItem("cap");    // 2
                  ((QComboBox *)lineEdit)->addItem("half");   // 3
                  ((QComboBox *)lineEdit)->addItem("base");   // 4
                  ((QComboBox *)lineEdit)->addItem("bottom"); // 5
                  ((QComboBox *)lineEdit)
                      ->setCurrentIndex(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
                }
              else if (cur_attr_name == "textalign_horizontal")
                {
                  lineEdit = new QComboBox(&dialog);
                  QString tooltipString =
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
                  tooltipString.append(" Default: ");
                  tooltipString.append(
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
                  ((QComboBox *)lineEdit)->setToolTip(tooltipString);
                  ((QComboBox *)lineEdit)->addItem("normal"); // 0
                  ((QComboBox *)lineEdit)->addItem("left");   // 1
                  ((QComboBox *)lineEdit)->addItem("center"); // 2
                  ((QComboBox *)lineEdit)->addItem("right");  // 3
                  ((QComboBox *)lineEdit)
                      ->setCurrentIndex(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
                }
              else if (cur_attr_name == "textalign")
                {
                  lineEdit = new QCheckBox(&dialog);
                  QString tooltipString =
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
                  tooltipString.append(" Default: ");
                  tooltipString.append(
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
                  ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                  ((QCheckBox *)lineEdit)
                      ->setChecked(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)) == 1);
                }
              else
                {
                  lineEdit = new QLineEdit(&dialog);
                  ((QLineEdit *)lineEdit)
                      ->setText(
                          static_cast<std::string>(current_selection->get_ref()->getAttribute(cur_attr_name)).c_str());
                  QString tooltipString =
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
                  tooltipString.append(" Default: ");
                  tooltipString.append(
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
                  ((QLineEdit *)lineEdit)->setToolTip(tooltipString);
                }
              QString label = QString(cur_attr_name.c_str());
              form.addRow(label, lineEdit);

              labels << label;
              fields << lineEdit;
            }

          QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
          form.addRow(&buttonBox);
          QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
          QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

          if (dialog.exec() == QDialog::Accepted)
            {
              for (int i = 0; i < labels.count(); i++)
                {
                  qDebug() << typeid(fields[i]).name();
                  auto &field = *fields[i]; // because typeid(*fields[i]) is bad :(
                  if (typeid(field) == typeid(QLineEdit))
                    {
                      std::string name = std::string(current_selection->get_ref()->getAttribute("name"));
                      if (labels[i].toStdString() == "text" &&
                          (name == "title" || name == "xlabel" || name == "ylabel"))
                        {
                          current_selection->get_ref()->parentElement()->setAttribute(
                              name, ((QLineEdit *)fields[i])->text().toStdString());
                        }
                      current_selection->get_ref()->setAttribute(labels[i].toStdString(),
                                                                 ((QLineEdit *)fields[i])->text().toStdString());
                    }
                  else if (typeid(field) == typeid(QComboBox))
                    {
                      current_selection->get_ref()->setAttribute(labels[i].toStdString(),
                                                                 ((QComboBox *)fields[i])->currentIndex());
                    }
                  else if (typeid(field) == typeid(QCheckBox))
                    {
                      current_selection->get_ref()->setAttribute(labels[i].toStdString(),
                                                                 ((QCheckBox *)fields[i])->isChecked());
                    }
                }
              current_selection = nullptr;
              mouse_move_selection = nullptr;
              amount_scrolled = 0;
              clicked.clear();
              std::cerr << GRM::toXML(grm_get_document_root()) << std::endl;
              reset_pixmap();
            }
        }
      else if (event->key() == Qt::Key_Delete)
        {
          if (current_selection == nullptr)
            {
              return;
            }
          amount_scrolled = 0;
          // to remove title, xlabel and ylabel from axis Node
          current_selection->get_ref()->parentElement()->removeAttribute(
              (std::string)current_selection->get_ref()->getAttribute("name"));
          current_selection->get_ref()->remove();
          std::cerr << GRM::toXML(grm_get_document_root()) << std::endl;
          mouse_move_selection = nullptr;
          reset_pixmap();
        }
    }
}

void GRPlotWidget::mouseMoveEvent(QMouseEvent *event)
{
  const char *kind;
  amount_scrolled = 0;

  if (enable_editor)
    {
      int x, y;
      getMousePos(event, &x, &y);
      auto cur_clicked = bounding_logic->get_bounding_objects_at_point(x, y);

      if (current_selection == nullptr)
        {
          mouse_move_selection = &cur_clicked[0];
          update();
        }
    }
  else
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
          int x, y;
          getMousePos(event, &x, &y);
          tooltip = grm_get_tooltip(x, y);

          grm_args_values(args_, "kind", "s", &kind);
          if (strcmp(kind, "marginalheatmap") == 0)
            {
              grm_args_t *input_args;
              input_args = grm_args_new();

              grm_args_push(input_args, "x", "i", x);
              grm_args_push(input_args, "y", "i", y);
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

      if (enable_editor)
        {
          amount_scrolled = 0;
          int x, y;
          getMousePos(event, &x, &y);
          auto cur_clicked = bounding_logic->get_bounding_objects_at_point(x, y);
          if (cur_clicked.empty())
            {
              clicked.clear();
              current_selection = nullptr;
              update();
              return;
            }
          else
            {
              clicked = cur_clicked;
            }
          current_selection = &clicked[0];
        }
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

  current_selection = nullptr;
  mouse_move_selection = nullptr;
  amount_scrolled = 0;
  clicked.clear();
  pixmap = nullptr;

  redraw();
}

void GRPlotWidget::wheelEvent(QWheelEvent *event)
{
  int x, y;
  getWheelPos(event, &x, &y);
  QPoint numPixels = event->pixelDelta();
  QPoint numDegrees = event->angleDelta();

  if (enable_editor)
    {
      if (!numPixels.isNull())
        {
          // Scrolling with pixels (For high-res scrolling like on macOS)

          // Prevent flickering when scrolling fast
          if (numPixels.y() > 0)
            {
              amount_scrolled += numPixels.y() < 10 ? numPixels.y() : 10;
            }
          else if (numPixels.y() < 0)
            {
              amount_scrolled += numPixels.y() > -10 ? numPixels.y() : -10;
            }
        }
      else if (!numDegrees.isNull())
        {
          QPoint numSteps = numDegrees / 16;
          // Scrolling with degrees
          if (numSteps.y() != 0)
            {
              amount_scrolled += numSteps.y();
            }
        }

      if (amount_scrolled > 50)
        {
          if (!clicked.empty() && current_selection != nullptr)
            {
              for (int i = 0; i < clicked.size(); i++)
                {
                  if (clicked[i].boundingRect() == current_selection->boundingRect())
                    {
                      if (i + 1 < clicked.size())
                        {
                          current_selection = &clicked[i + 1];
                          break;
                        }
                      else
                        {
                          current_selection = &clicked[0];
                        }
                    }
                }
            }
          amount_scrolled = 0;
        }
      else if (amount_scrolled < -50)
        {
          if (!clicked.empty() && current_selection != nullptr)
            {
              for (int i = clicked.size() - 1; i >= 0; i--)
                {
                  if (clicked[i].boundingRect() == current_selection->boundingRect())
                    {
                      if (i - 1 < 0)
                        {
                          current_selection = &clicked[clicked.size() - 1];
                        }
                      else
                        {
                          current_selection = &clicked[i - 1];
                          break;
                        }
                    }
                }
            }
          amount_scrolled = 0;
        }
    }
  else
    {
      grm_args_t *args = grm_args_new();
      grm_args_push(args, "x", "i", x);
      grm_args_push(args, "y", "i", y);
      grm_args_push(args, "angle_delta", "d", (double)event->angleDelta().y());
      grm_input(args);
      grm_args_delete(args);
    }

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
  kind_changed = true;
  redraw();
}

void GRPlotWidget::marginalheatmapall()
{
  grm_args_push(args_, "kind", "s", "marginalheatmap");
  grm_args_push(args_, "marginalheatmap_kind", "s", "all");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::marginalheatmapline()
{
  grm_args_push(args_, "kind", "s", "marginalheatmap");
  grm_args_push(args_, "marginalheatmap_kind", "s", "line");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::line()
{
  grm_args_push(args_, "kind", "s", "line");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::sumalgorithm()
{
  grm_args_push(args_, "algorithm", "s", "sum");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::maxalgorithm()
{
  grm_args_push(args_, "algorithm", "s", "max");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::volume()
{
  grm_args_push(args_, "kind", "s", "volume");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}
void GRPlotWidget::isosurface()
{
  grm_args_push(args_, "kind", "s", "isosurface");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::surface()
{
  grm_args_push(args_, "kind", "s", "surface");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}
void GRPlotWidget::wireframe()
{
  grm_args_push(args_, "kind", "s", "wireframe");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::contour()
{
  grm_args_push(args_, "kind", "s", "contour");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::imshow()
{
  grm_args_push(args_, "kind", "s", "imshow");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::plot3()
{
  grm_args_push(args_, "kind", "s", "plot3");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::contourf()
{
  grm_args_push(args_, "kind", "s", "contourf");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::trisurf()
{
  grm_args_push(args_, "kind", "s", "trisurf");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::tricont()
{
  grm_args_push(args_, "kind", "s", "tricont");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::scatter3()
{
  grm_args_push(args_, "kind", "s", "scatter3");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::scatter()
{
  grm_args_push(args_, "kind", "s", "scatter");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::hist()
{
  grm_args_push(args_, "kind", "s", "hist");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::barplot()
{
  grm_args_push(args_, "kind", "s", "barplot");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::stairs()
{
  grm_args_push(args_, "kind", "s", "stairs");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::stem()
{
  grm_args_push(args_, "kind", "s", "stem");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::shade()
{
  grm_args_push(args_, "kind", "s", "shade");
  grm_merge(args_);
  kind_changed = true;
  redraw();
}

void GRPlotWidget::hexbin()
{
  grm_args_push(args_, "kind", "s", "hexbin");
  grm_merge(args_);
  kind_changed = true;
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

void GRPlotWidget::moveEvent(QMoveEvent *event)
{
  if (enable_editor)
    {
      treewidget->move(this->width() + event->pos().x(),
                       event->pos().y() - (treewidget->geometry().y() - treewidget->pos().y()));
    }
}

void GRPlotWidget::extract_bounding_boxes_from_grm(QPainter &painter)
{
  auto global_root = grm_get_document_root();
  double xmin, xmax, ymin, ymax;
  int id;

  if (enable_editor)
    {
      painter.setPen(QPen(QColor(255, 0, 0, 100)));

      for (const auto &cur_child : global_root->querySelectorsAll("[bbox_id]"))
        {
          id = static_cast<int>(cur_child->getAttribute("bbox_id"));
          xmin = static_cast<double>(cur_child->getAttribute("bbox_xmin"));
          xmax = static_cast<double>(cur_child->getAttribute("bbox_xmax"));
          ymin = static_cast<double>(cur_child->getAttribute("bbox_ymin"));
          ymax = static_cast<double>(cur_child->getAttribute("bbox_ymax"));

          if (xmin == DBL_MAX || xmax == -DBL_MAX || ymin == DBL_MAX || ymax == -DBL_MAX)
            {
              qDebug() << "skipping" << cur_child->localName().c_str();
            }
          else
            {
              auto b = Bounding_object(id, xmin, xmax, ymin, ymax, cur_child);
              bounding_logic->add_bounding_object(b);
              auto bounding_rect = b.boundingRect();
              if (highlightBoundingObjects)
                {
                  painter.drawRect(bounding_rect);
                  painter.drawText(bounding_rect.bottomRight() + QPointF(5, 0), cur_child->localName().c_str());
                }
            }
        }
    }
}

void GRPlotWidget::highlight_current_selection(QPainter &painter)
{
  if (enable_editor)
    {
      if (current_selection != nullptr)
        {
          painter.fillRect(current_selection->boundingRect(), QBrush(QColor("blue"), Qt::Dense5Pattern));
        }
      else if (mouse_move_selection != nullptr)
        {
          painter.fillRect(mouse_move_selection->boundingRect(), QBrush(QColor("blue"), Qt::Dense7Pattern));
        }
    }
}

void GRPlotWidget::leaveEvent(QEvent *event)
{
  // Prevent highlighting if mouse leaves the widget without a mouseMoveEvent
  if (enable_editor)
    {
      mouse_move_selection = nullptr;
      update();
    }
}

void GRPlotWidget::reset_pixmap()
{
  pixmap = nullptr;
  current_selection = nullptr;
  update();
}

void GRPlotWidget::show_bounding_boxes_slot()
{
  if (enable_editor)
    {
      highlightBoundingObjects = show_bounding_boxes_action->isChecked();
      update();
    }
}

void GRPlotWidget::open_file_slot()
{
  if (enable_editor) fprintf(stderr, "This functionality isnt`t implemented yet\n");
  // probably want to be able to load a new dataset as a subplot
}

void GRPlotWidget::save_file_slot()
{
  if (enable_editor)
    {
      if (grm_get_render() == nullptr)
        {
          QApplication::beep();
          return;
        }
      std::string save_file_name =
          QFileDialog::getSaveFileName(this, "Save XML", QDir::homePath(), "XML files (*.xml)").toStdString();
      if (save_file_name.empty())
        {
          return;
        }
      std::ofstream save_file_stream(save_file_name);
      if (!save_file_stream)
        {
          std::stringstream text_stream;
          text_stream << "Could not save the graphics tree to the XML file \"" << save_file_name << "\".";
          QMessageBox::critical(this, "File save not possible", QString::fromStdString(text_stream.str()));
          return;
        }
      save_file_stream << GRM::toXML(grm_get_render()) << std::endl;
      save_file_stream.close();
    }
}

void GRPlotWidget::show_container_slot()
{
  if (enable_editor)
    {
      if (show_container_action->isChecked())
        {
          treewidget->show();
        }
      else
        {
          treewidget->hide();
        }
      treewidget->resize(400, height());
      treewidget->move(this->pos().x() + this->width(),
                       this->pos().y() - 28 + (treewidget->geometry().y() - treewidget->pos().y()));
    }
}

void GRPlotWidget::enable_editor_functions()
{
  if (editor_action->isChecked())
    {
      enable_editor = true;

      // dirty reset of screen position
      grm_args_t *args = grm_args_new();
      QPoint pos = mapFromGlobal(QCursor::pos());
      grm_args_push(args, "key", "s", "r");
      grm_args_push(args, "x", "i", pos.x());
      grm_args_push(args, "y", "i", pos.y());
      grm_input(args);
      grm_args_delete(args);

      redraw();
    }
  else
    {
      enable_editor = false;
    }
}
