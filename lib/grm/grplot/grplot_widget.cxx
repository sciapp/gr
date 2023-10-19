#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <list>
#include <sstream>
#include <functional>
#include <cassert>

#include <gr.h>

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
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QWindow>

#include "grplot_widget.hxx"
#include "util.hxx"

#ifndef GR_UNUSED
#define GR_UNUSED(x) (void)(x)
#endif

static std::string file_export;
static bool arguments_changed = false;
static QString test_commands_file_path = "";
static QFile *test_commands_file = nullptr;
static QTextStream *test_commands_stream = nullptr;
static Qt::KeyboardModifiers modifiers = Qt::NoModifier;
static std::vector<Bounding_object> cur_moved;

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

std::function<void(const grm_request_event_t *)> cmd_callback;
extern "C" void cmd_callback_wrapper(const grm_event_t *event)
{
  cmd_callback(reinterpret_cast<const grm_request_event_t *>(event));
}


GRPlotWidget::GRPlotWidget(QMainWindow *parent, int argc, char **argv)
    : QWidget(parent), pixmap(), redraw_pixmap(false), args_(nullptr), rubberBand(nullptr)
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
  selected_parent = nullptr;

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
      grm_register(GRM_EVENT_REQUEST, cmd_callback_wrapper);
      grm_args_t_wrapper configuration;
      configuration.set_wrapper(grm_args_new());
      grm_args_push(configuration.get_wrapper(), "hold_plots", "i", 0);
      grm_merge(configuration.get_wrapper());
      grm_args_delete(configuration.get_wrapper());
    }
  else
    {
      if (strcmp(argv[1], "--test") == 0)
        {
          test_commands_file_path = argv[2];
          argv += 2;
          argc -= 2;
          test_commands_file = new QFile(test_commands_file_path);
          if (test_commands_file->open(QIODevice::ReadOnly))
            {
              test_commands_stream = new QTextStream(test_commands_file);
              QTimer::singleShot(1000, this, &GRPlotWidget::processTestCommandsFile);
            }
          else
            {
              std::cerr << "Unable to open test commands file" << std::endl;
              QApplication::quit();
            }
        }
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
          algo->menuAction()->setVisible(false);
          if (strcmp(kind, "marginalheatmap") == 0)
            {
              algo->menuAction()->setVisible(true);
            }
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
      if (strcmp(argv[1], "--test") != 0 && !test_commands_stream)
        {
          menu->addMenu(type);
          menu->addMenu(algo);
        }
    }
  if (strcmp(argv[1], "--test") != 0 && !test_commands_stream) menu->addMenu(export_menu);

  if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
    {
#if !defined(NO_LIBXML2)
      schema_tree = grm_load_graphics_tree_schema();
#else
      schema_tree = nullptr;
#endif
      add_element_widget = new AddElementWidget(this);
      add_element_widget->hide();

      editor_menu = new QMenu(tr("&Editor"));
      editor_action = new QAction(tr("&Enable Editorview"));
      editor_action->setCheckable(true);
      editor_menu->addAction(editor_action);
      QObject::connect(editor_action, SIGNAL(triggered()), this, SLOT(enable_editor_functions()));

      file_menu = editor_menu->addMenu(tr("&XML-File"));
      save_file_action = new QAction("&Save Plot");
      save_file_action->setShortcut(Qt::CTRL | Qt::Key_S);
      file_menu->addAction(save_file_action);
      QObject::connect(save_file_action, SIGNAL(triggered()), this, SLOT(save_file_slot()));

      open_file_action = new QAction("&Open Plot");
      open_file_action->setShortcut(Qt::CTRL | Qt::Key_O);
      file_menu->addAction(open_file_action);
      QObject::connect(open_file_action, SIGNAL(triggered()), this, SLOT(open_file_slot()));

      configuration_menu = editor_menu->addMenu(tr("&Show"));
      configuration_menu->menuAction()->setVisible(false);
      show_container_action = new QAction(tr("&GRM Container"));
      show_container_action->setCheckable(true);
      show_container_action->setShortcut(Qt::CTRL | Qt::Key_C);
      show_container_action->setVisible(false);
      configuration_menu->addAction(show_container_action);
      QObject::connect(show_container_action, SIGNAL(triggered()), this, SLOT(show_container_slot()));

      show_bounding_boxes_action = new QAction(tr("&Bounding Boxes"));
      show_bounding_boxes_action->setCheckable(true);
      show_bounding_boxes_action->setShortcut(Qt::CTRL | Qt::Key_B);
      show_bounding_boxes_action->setVisible(false);
      configuration_menu->addAction(show_bounding_boxes_action);
      QObject::connect(show_bounding_boxes_action, SIGNAL(triggered()), this, SLOT(show_bounding_boxes_slot()));

      add_element_action = new QAction("&Add Element");
      add_element_action->setCheckable(true);
      add_element_action->setShortcut(Qt::CTRL | Qt::Key_Plus);
      editor_menu->addAction(add_element_action);
      QObject::connect(add_element_action, SIGNAL(triggered()), this, SLOT(add_element_slot()));
      add_element_action->setVisible(false);

      menu->addMenu(editor_menu);
    }
}

GRPlotWidget::~GRPlotWidget()
{
  grm_args_delete(args_);
  grm_finalize();
}

void GRPlotWidget::AttributeEditEvent()
{
  if (current_selection == nullptr)
    {
      return;
    }
  std::string currently_clicked_name = current_selection->get_ref()->localName();

  QDialog dialog(this);
  QString title("Selected: ");
  title.append(currently_clicked_name.c_str());
  dialog.setWindowTitle(title);
  auto changeParametersLabel = new QLabel("Change Parameters:");
  changeParametersLabel->setStyleSheet("font-weight: bold");
  auto form = new QFormLayout;
  form->addRow(changeParametersLabel);

  QList<QString> labels;
  QList<QWidget *> fields;
  QWidget *lineEdit;
  std::vector<std::string> attr_type;

  for (const auto &cur_attr_name : current_selection->get_ref()->getAttributeNames())
    {
      if (util::startsWith(cur_attr_name, "_"))
        {
          continue;
        }
      if (cur_attr_name == "render_method")
        {
          lineEdit = new QComboBox(&dialog);
          QString tooltipString =
              GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
          tooltipString.append(".  Default: ");
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
          tooltipString.append(".  Default: ");
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
          tooltipString.append(".  Default: ");
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
          tooltipString.append(".  Default: ");
          tooltipString.append(
              GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
          ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
          ((QCheckBox *)lineEdit)
              ->setChecked(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)) == 1);
        }
      else
        {
          if (current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
            {
              attr_type.push_back("xs:integer");
            }
          else if (current_selection->get_ref()->getAttribute(cur_attr_name).isDouble())
            {
              attr_type.push_back("xs:double");
            }
          else
            {
              attr_type.push_back("xs:string");
            }
          lineEdit = new QLineEdit(&dialog);
          ((QLineEdit *)lineEdit)
              ->setText(static_cast<std::string>(current_selection->get_ref()->getAttribute(cur_attr_name)).c_str());
          QString tooltipString =
              GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
          tooltipString.append(".  Default: ");
          tooltipString.append(
              GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
          ((QLineEdit *)lineEdit)->setToolTip(tooltipString);
        }
      QString text_label = QString(cur_attr_name.c_str());
      form->addRow(text_label, lineEdit);

      labels << text_label;
      fields << lineEdit;
    }

  if (schema_tree != nullptr)
    {
      std::shared_ptr<GRM::Element> element;
      auto selections = schema_tree->querySelectorsAll("[name=" + currently_clicked_name + "]");
      for (const auto &selection : selections)
        {
          if (selection->localName() == "xs:element") element = selection->children()[0];
        }

      /* iterate through complextype elements */
      for (const auto &child : element->children())
        {
          if (child->localName() == "xs:attribute")
            {
              auto attr_name = static_cast<std::string>(child->getAttribute("name"));
              if (!current_selection->get_ref()->hasAttribute(attr_name))
                {
                  /* attributes of an element which aren't already in the tree getting added with red text color
                   */
                  auto type_name = static_cast<std::string>(child->getAttribute("type"));
                  attr_type.push_back(type_name);

                  lineEdit = new QLineEdit(&dialog);
                  QString tooltipString =
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), attr_name)[1].c_str();
                  tooltipString.append(".  Default: ");
                  tooltipString.append(
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), attr_name)[0].c_str());
                  ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                  ((QLineEdit *)lineEdit)->setText("");
                  QString text_label = QString("<span style='color:#ff0000;'>%1</span>").arg(attr_name.c_str());
                  form->addRow(text_label, lineEdit);

                  labels << text_label;
                  fields << lineEdit;
                }
            }
          else if (child->localName() == "xs:attributegroup")
            {
              /* when an element contains one or more attributegroups all attributes from these groups must be
               * added */
              std::shared_ptr<GRM::Element> group;
              auto group_name = static_cast<std::string>(child->getAttribute("ref"));

              if (group_name != "colorrep")
                {
                  auto attr_group_selections = schema_tree->querySelectorsAll("[name=" + group_name + "]");
                  for (const auto &selection : attr_group_selections)
                    {
                      if (selection->localName() == "xs:attributegroup") group = selection;
                    }

                  /* iterate through attribute elements */
                  for (const auto &childchild : group->children())
                    {
                      if (childchild->localName() == "xs:attribute")
                        {
                          auto attr_name = static_cast<std::string>(childchild->getAttribute("name"));
                          if (!current_selection->get_ref()->hasAttribute(attr_name))
                            {
                              /* attributes of an element which aren't already in the tree getting added with
                               * red text color */
                              auto type_name = static_cast<std::string>(childchild->getAttribute("type"));
                              attr_type.push_back(type_name);

                              lineEdit = new QLineEdit(&dialog);
                              QString tooltipString =
                                  GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), attr_name)[1].c_str();
                              tooltipString.append(".  Default: ");
                              tooltipString.append(
                                  GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), attr_name)[0]
                                      .c_str());
                              ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                              ((QLineEdit *)lineEdit)->setText("");
                              QString text_label =
                                  QString("<span style='color:#ff0000;'>%1</span>").arg(attr_name.c_str());
                              form->addRow(text_label, lineEdit);

                              labels << text_label;
                              fields << lineEdit;
                            }
                        }
                    }
                }
              else
                {
                  /* special case for colorrep cause there are way to many attributes inside the attributegroup
                   */
                  lineEdit = new QLineEdit(&dialog);
                  ((QLineEdit *)lineEdit)->setText("");
                  QString text_label = QString("<span style='color:#ff0000;'>%1</span>").arg("Colorrep-index");
                  form->addRow(text_label, lineEdit);

                  attr_type.push_back("xs:string");
                  labels << text_label;
                  fields << lineEdit;

                  lineEdit = new QLineEdit(&dialog);
                  ((QLineEdit *)lineEdit)->setText("");
                  text_label = QString("<span style='color:#ff0000;'>%1</span>").arg("Colorrep-value");
                  form->addRow(text_label, lineEdit);

                  attr_type.push_back("xs:string");
                  labels << text_label;
                  fields << lineEdit;
                }
            }
        }
    }

  QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
  form->addRow(&buttonBox);
  QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
  QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

  auto scrollAreaContent = new QWidget;
  scrollAreaContent->setLayout(form);
  auto scrollArea = new QScrollArea;
  scrollArea = new QScrollArea;
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrollArea->setWidgetResizable(true);
  scrollArea->setWidget(scrollAreaContent);

  auto groupBoxLayout = new QVBoxLayout;
  groupBoxLayout->addWidget(scrollArea);
  dialog.setLayout(groupBoxLayout);

  if (dialog.exec() == QDialog::Accepted)
    {
      for (int i = 0; i < labels.count(); i++)
        {
          qDebug() << typeid(fields[i]).name();
          auto &field = *fields[i]; // because typeid(*fields[i]) is bad :(
          if (typeid(field) == typeid(QLineEdit) && ((QLineEdit *)fields[i])->isModified())
            {
              std::string name = std::string(current_selection->get_ref()->getAttribute("name"));
              if (((QLineEdit *)fields[i])->text().toStdString().empty())
                {
                  /* remove attributes from tree when the value got removed */
                  current_selection->get_ref()->removeAttribute(labels[i].toStdString());
                }
              else
                {
                  if (util::startsWith(labels[i].toStdString(), "<span style='color:#ff0000;'>") &&
                      util::endsWith(labels[i].toStdString(), "</span>"))
                    {
                      labels[i].remove(0, 29);
                      labels[i].remove(labels[i].size() - 7, 7);
                    }
                  if (labels[i].toStdString() == "text" && (name == "title" || name == "xlabel" || name == "ylabel"))
                    {
                      const std::string value = ((QLineEdit *)fields[i])->text().toStdString();
                      if (attr_type[i] == "xs:string" || (attr_type[i] == "strint" && !util::is_digits(value)))
                        {
                          current_selection->get_ref()->parentElement()->setAttribute(name, value);
                        }
                      else if (attr_type[i] == "xs:double")
                        {
                          current_selection->get_ref()->parentElement()->setAttribute(labels[i].toStdString(),
                                                                                      std::stod(value));
                        }
                      else if (attr_type[i] == "xs:integer" || (attr_type[i] == "strint" && util::is_digits(value)))
                        {
                          current_selection->get_ref()->parentElement()->setAttribute(labels[i].toStdString(),
                                                                                      std::stoi(value));
                        }
                    }
                  if (labels[i].toStdString() == "Colorrep-index")
                    {
                      /* special case for colorrep attribute */
                      current_selection->get_ref()->setAttribute("colorrep." +
                                                                     ((QLineEdit *)fields[i])->text().toStdString(),
                                                                 ((QLineEdit *)fields[i + 1])->text().toStdString());
                    }
                  else if (labels[i].toStdString() != "Colorrep-value")
                    {
                      const std::string value = ((QLineEdit *)fields[i])->text().toStdString();
                      if (attr_type[i] == "xs:string" || (attr_type[i] == "strint" && !util::is_digits(value)))
                        {
                          current_selection->get_ref()->setAttribute(labels[i].toStdString(), value);
                        }
                      else if (attr_type[i] == "xs:double")
                        {
                          current_selection->get_ref()->setAttribute(labels[i].toStdString(), std::stod(value));
                        }
                      else if (attr_type[i] == "xs:integer" || (attr_type[i] == "strint" && util::is_digits(value)))
                        {
                          current_selection->get_ref()->setAttribute(labels[i].toStdString(), std::stoi(value));
                        }
                    }
                }
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
  if (arguments_changed)
    {
      assert(grm_plot(args_));
      arguments_changed = false;
    }
  else
    {
      assert(grm_plot(nullptr));
    }
}

void GRPlotWidget::redraw()
{
  redraw_pixmap = true;

  update();
}

void GRPlotWidget::collectTooltips()
{
  QPoint mouse_pos = this->mapFromGlobal(QCursor::pos());
  Qt::KeyboardModifiers keyboard_modifiers = GRPlotWidget::queryKeyboardModifiers();

  if (keyboard_modifiers == Qt::ShiftModifier)
    {
      auto accumulated_tooltip = grm_get_accumulated_tooltip_x(mouse_pos.x(), mouse_pos.y());
      tooltips.clear();
      if (accumulated_tooltip != nullptr) tooltips.push_back(accumulated_tooltip);
    }
  else
    {
      if (keyboard_modifiers != Qt::AltModifier)
        {
          tooltips.clear();
        }
      auto current_tooltip = grm_get_tooltip(mouse_pos.x(), mouse_pos.y());
      bool found_current_tooltip = false;
      if (current_tooltip != nullptr)
        {
          for (const auto &tooltip : tooltips)
            {
              if (tooltip.get<grm_tooltip_info_t>()->x == current_tooltip->x &&
                  tooltip.get<grm_tooltip_info_t>()->y == current_tooltip->y)
                {
                  found_current_tooltip = true;
                  break;
                }
            }
          if (!found_current_tooltip)
            {
              tooltips.push_back(current_tooltip);
            }
        }
    }
}

static const std::string tooltipStyle{"\
    .gr-label {\n\
        color: #26aae1;\n\
        font-size: 11px;\n\
        line-height: 0.8;\n\
    }\n\
    .gr-value {\n\
        color: #3c3c3c;\n\
        font-size: 11px;\n\
        line-height: 0.8;\n\
    }"};

static const std::string tooltipTemplate{"\
    <span class=\"gr-label\">%s</span><br>\n\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span><br>\n\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span>"};

static const std::string accumulatedTooltipTemplate{"\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span>"};

void GRPlotWidget::paintEvent(QPaintEvent *event)
{
  util::unused(event);
  paint(this);
}

void GRPlotWidget::paint(QPaintDevice *paint_device)
{
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
      painter.save();
      draw();
      painter.restore();
      painter.end();
      redraw_pixmap = false;

      treewidget->updateData(grm_get_document_root());
    }

  painter.begin(paint_device);
  painter.drawPixmap(0, 0, pixmap);
  bounding_logic->clear();
  extract_bounding_boxes_from_grm((QPainter &)painter);
  highlight_current_selection((QPainter &)painter);
  if (!tooltips.empty() && !enable_editor)
    {
      for (const auto &tooltip : tooltips)
        {
          if (tooltip.x_px() > 0 && tooltip.y_px() > 0)
            {
              QColor background(224, 224, 224, 128);
              QPainterPath triangle;
              std::string x_label = tooltip.xlabel();
              std::string info;

              if (util::startsWith(x_label, "$") && util::endsWith(x_label, "$"))
                {
                  x_label = "x";
                }

              if (tooltip.holds_alternative<grm_tooltip_info_t>())
                {
                  const grm_tooltip_info_t *single_tooltip = tooltip.get<grm_tooltip_info_t>();
                  std::string y_label = single_tooltip->ylabel;

                  if (util::startsWith(y_label, "$") && util::endsWith(y_label, "$"))
                    {
                      y_label = "y";
                    }
                  info = util::string_format(tooltipTemplate, single_tooltip->label, x_label.c_str(), single_tooltip->x,
                                             y_label.c_str(), single_tooltip->y);
                }
              else
                {
                  const grm_accumulated_tooltip_info_t *accumulated_tooltip =
                      tooltip.get<grm_accumulated_tooltip_info_t>();
                  std::vector<std::string> y_labels(accumulated_tooltip->ylabels,
                                                    accumulated_tooltip->ylabels + accumulated_tooltip->n);

                  std::vector<std::string> info_parts;
                  info_parts.push_back(
                      util::string_format(accumulatedTooltipTemplate, x_label.c_str(), accumulated_tooltip->x));
                  for (int i = 0; i < y_labels.size(); ++i)
                    {
                      auto &y = accumulated_tooltip->y[i];
                      auto &y_label = y_labels[i];
                      if (util::startsWith(y_label, "$") && util::endsWith(y_label, "$"))
                        {
                          y_label = "y";
                        }
                      info_parts.push_back("<br>\n");
                      info_parts.push_back(util::string_format(accumulatedTooltipTemplate, y_label.c_str(), y));
                    }
                  std::ostringstream info_stream;
                  std::copy(info_parts.begin(), info_parts.end(), std::ostream_iterator<std::string>(info_stream));
                  info = info_stream.str();
                }
              label.setDefaultStyleSheet(QString::fromStdString(tooltipStyle));
              label.setHtml(QString::fromStdString(info));
              grm_args_values(args_, "kind", "s", &kind);
              if (strcmp(kind, "heatmap") == 0 || strcmp(kind, "marginalheatmap") == 0)
                {
                  background.setAlpha(224);
                }
              painter.fillRect(tooltip.x_px() + 8, (int)(tooltip.y_px() - label.size().height() / 2),
                               (int)label.size().width(), (int)label.size().height(),
                               QBrush(background, Qt::SolidPattern));

              triangle.moveTo(tooltip.x_px(), tooltip.y_px());
              triangle.lineTo(tooltip.x_px() + 8, tooltip.y_px() + 6);
              triangle.lineTo(tooltip.x_px() + 8, tooltip.y_px() - 6);
              triangle.closeSubpath();
              background.setRgb(128, 128, 128, 128);
              painter.fillPath(triangle, QBrush(background, Qt::SolidPattern));

              painter.save();
              painter.translate(tooltip.x_px() + 8, tooltip.y_px() - label.size().height() / 2);
              label.drawContents(&painter);
              painter.restore();
            }
        }
    }
  painter.end();
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
          AttributeEditEvent();
        }
      else if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
        {
          if (current_selection == nullptr)
            {
              return;
            }
          amount_scrolled = 0;
          // to remove yline, title, xlabel and ylabel from axis Node
          auto elem_name = (std::string)current_selection->get_ref()->getAttribute("name");
          if (current_selection->get_ref()->parentElement()->hasAttribute(elem_name))
            {
              if (elem_name == "yline")
                {
                  current_selection->get_ref()->parentElement()->setAttribute(elem_name, false);
                }
              else
                {
                  current_selection->get_ref()->parentElement()->removeAttribute(elem_name);
                }
            }
          auto parent = current_selection->get_ref()->parentElement();
          while (parent != nullptr && parent->localName() != "root" && parent->childElementCount() <= 1)
            {
              auto tmp_parent = parent->parentElement();
              // to remove xticklabels, yticklabels from coordinate_system
              elem_name = (std::string)parent->getAttribute("name");
              if (tmp_parent->hasAttribute(elem_name)) tmp_parent->removeAttribute(elem_name);
              parent->remove();
              parent = tmp_parent;
            }
          current_selection->get_ref()->remove();
          mouse_move_selection = nullptr;
          reset_pixmap();
        }
      else if (event->key() == Qt::Key_Shift)
        {
          if (!clicked.empty() && current_selection != nullptr)
            {
              for (int i = 0; i < clicked.size(); i++)
                {
                  if (clicked[i].get_id() == current_selection->get_id())
                    {
                      if (i + 1 < clicked.size())
                        {
                          current_selection = &clicked[i + 1];
                          break;
                        }
                      else
                        {
                          current_selection = &clicked[i + 1 - clicked.size()];
                          break;
                        }
                    }
                }
            }
        }
    }
  else
    {
      collectTooltips();
      update();
    }
}

void GRPlotWidget::keyReleaseEvent(QKeyEvent *event)
{
  GR_UNUSED(event);
  if (!enable_editor) collectTooltips();
  update();
}

void GRPlotWidget::mouseMoveEvent(QMouseEvent *event)
{
  amount_scrolled = 0;

  if (enable_editor)
    {
      int x, y;
      getMousePos(event, &x, &y);
      cur_moved = bounding_logic->get_bounding_objects_at_point(x, y);

      if (current_selection == nullptr)
        {
          if (cur_moved.empty())
            {
              mouse_move_selection = nullptr;
            }
          else
            {
              mouse_move_selection = &cur_moved[0];
            }
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
          const char *kind;
          collectTooltips();
          if (args_ && grm_args_values(args_, "kind", "s", &kind))
            {
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
          update();
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
          mouse_move_selection = nullptr;
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

  update();
}

void GRPlotWidget::resizeEvent(QResizeEvent *event)
{
  grm_args_push(args_, "size", "dd", (double)event->size().width(), (double)event->size().height());
  grm_merge_hold(args_);

  auto root = grm_get_document_root();
  auto figure = root->querySelectors("[active=1]");
  if (figure != nullptr)
    {
      figure->setAttribute("size_x", (double)event->size().width());
      figure->setAttribute("size_y", (double)event->size().height());
    }
  else
    {
      arguments_changed = true;
    }

  current_selection = nullptr;
  mouse_move_selection = nullptr;
  amount_scrolled = 0;
  clicked.clear();
  reset_pixmap();

  redraw();
}

void GRPlotWidget::wheelEvent(QWheelEvent *event)
{
  if (event->angleDelta().y() != 0)
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
                      if (clicked[i].get_id() == current_selection->get_id())
                        {
                          if (i + 1 < clicked.size())
                            {
                              current_selection = &clicked[i + 1];
                              break;
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
                      if (clicked[i].get_id() == current_selection->get_id())
                        {
                          if (i - 1 > 0)
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
}

void GRPlotWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (enable_editor)
    {
      if (event->button() == Qt::LeftButton)
        {
          AttributeEditEvent();
        }
    }
  else
    {
      GR_UNUSED(event);
      grm_args_t *args = grm_args_new();
      QPoint pos = mapFromGlobal(QCursor::pos());
      grm_args_push(args, "key", "s", "r");
      grm_args_push(args, "x", "i", pos.x());
      grm_args_push(args, "y", "i", pos.y());
      grm_input(args);
      grm_args_delete(args);

      redraw();
    }
}

void GRPlotWidget::heatmap()
{
  algo->menuAction()->setVisible(false);
  grm_args_push(args_, "kind", "s", "heatmap");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::marginalheatmapall()
{
  algo->menuAction()->setVisible(true);
  grm_args_push(args_, "kind", "s", "marginalheatmap");
  grm_args_push(args_, "marginalheatmap_kind", "s", "all");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::marginalheatmapline()
{
  algo->menuAction()->setVisible(true);
  grm_args_push(args_, "kind", "s", "marginalheatmap");
  grm_args_push(args_, "marginalheatmap_kind", "s", "line");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::line()
{
  grm_args_push(args_, "kind", "s", "line");
  grm_merge(args_);
  auto root = grm_get_document_root();
  for (const auto &child : root->querySelectorsAll("series_scatter"))
    {
      child->setAttribute("kind", "line");
    }

  // to get the same lines then before all lines have to exist during render call so that the linespec work properly
  for (const auto &child : root->querySelectorsAll("series_line"))
    {
      child->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::sumalgorithm()
{
  grm_args_push(args_, "algorithm", "s", "sum");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::maxalgorithm()
{
  grm_args_push(args_, "algorithm", "s", "max");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::volume()
{
  grm_args_push(args_, "kind", "s", "volume");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}
void GRPlotWidget::isosurface()
{
  grm_args_push(args_, "kind", "s", "isosurface");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::surface()
{
  algo->menuAction()->setVisible(false);
  grm_args_push(args_, "kind", "s", "surface");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}
void GRPlotWidget::wireframe()
{
  algo->menuAction()->setVisible(false);
  grm_args_push(args_, "kind", "s", "wireframe");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::contour()
{
  algo->menuAction()->setVisible(false);
  grm_args_push(args_, "kind", "s", "contour");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::imshow()
{
  algo->menuAction()->setVisible(false);
  grm_args_push(args_, "kind", "s", "imshow");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::plot3()
{
  grm_args_push(args_, "kind", "s", "plot3");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::contourf()
{
  algo->menuAction()->setVisible(false);
  grm_args_push(args_, "kind", "s", "contourf");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::trisurf()
{
  grm_args_push(args_, "kind", "s", "trisurf");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::tricont()
{
  grm_args_push(args_, "kind", "s", "tricont");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::scatter3()
{
  grm_args_push(args_, "kind", "s", "scatter3");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::scatter()
{
  grm_args_push(args_, "kind", "s", "scatter");
  grm_merge(args_);
  auto root = grm_get_document_root();
  for (const auto &child : root->querySelectorsAll("series_line"))
    {
      child->setAttribute("kind", "scatter");
    }
  redraw();
}

void GRPlotWidget::hist()
{
  grm_args_push(args_, "kind", "s", "hist");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::barplot()
{
  grm_args_push(args_, "kind", "s", "barplot");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::stairs()
{
  grm_args_push(args_, "kind", "s", "stairs");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::stem()
{
  grm_args_push(args_, "kind", "s", "stem");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::shade()
{
  grm_args_push(args_, "kind", "s", "shade");
  grm_merge(args_);
  arguments_changed = true;
  redraw();
}

void GRPlotWidget::hexbin()
{
  grm_args_push(args_, "kind", "s", "hexbin");
  grm_merge(args_);
  arguments_changed = true;
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

      for (const auto &cur_child : global_root->querySelectorsAll("[_bbox_id]"))
        {
          id = static_cast<int>(cur_child->getAttribute("_bbox_id"));
          xmin = static_cast<double>(cur_child->getAttribute("_bbox_xmin"));
          xmax = static_cast<double>(cur_child->getAttribute("_bbox_xmax"));
          ymin = static_cast<double>(cur_child->getAttribute("_bbox_ymin"));
          ymax = static_cast<double>(cur_child->getAttribute("_bbox_ymax"));

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
                  painter.drawText(bounding_rect.topLeft() + QPointF(5, 10), cur_child->localName().c_str());
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
          painter.fillRect(current_selection->boundingRect(), QBrush(QColor(190, 210, 232, 150), Qt::SolidPattern));
          if (current_selection->get_ref() != nullptr)
            painter.drawText(current_selection->boundingRect().topLeft() + QPointF(5, 10),
                             current_selection->get_ref()->localName().c_str());
        }
      else if (mouse_move_selection != nullptr)
        {
          painter.fillRect(mouse_move_selection->boundingRect(), QBrush(QColor(190, 210, 232, 150), Qt::SolidPattern));
          if (mouse_move_selection->get_ref() != nullptr)
            painter.drawText(mouse_move_selection->boundingRect().topLeft() + QPointF(5, 10),
                             mouse_move_selection->get_ref()->localName().c_str());
        }
      if (selected_parent != nullptr)
        {
          auto rect = selected_parent->boundingRect();
          if (selected_parent->get_ref() != nullptr)
            {
              auto bbox_xmin = static_cast<double>(selected_parent->get_ref()->getAttribute("_bbox_xmin"));
              auto bbox_xmax = static_cast<double>(selected_parent->get_ref()->getAttribute("_bbox_xmax"));
              auto bbox_ymin = static_cast<double>(selected_parent->get_ref()->getAttribute("_bbox_ymin"));
              auto bbox_ymax = static_cast<double>(selected_parent->get_ref()->getAttribute("_bbox_ymax"));
              rect = QRectF(bbox_xmin, bbox_ymin, bbox_xmax - bbox_xmin, bbox_ymax - bbox_ymin);
              painter.drawText(rect.topLeft() + QPointF(5, 10), selected_parent->get_ref()->localName().c_str());
            }
          painter.fillRect(rect, QBrush(QColor(255, 0, 0, 30), Qt::SolidPattern));
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
  redraw_pixmap = true;
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
  if (enable_editor)
    {
      std::string path =
          QFileDialog::getOpenFileName(this, "Open XML", QDir::homePath(), "XML files (*.xml)").toStdString();
      if (path.empty())
        {
          return;
        }

      auto file = fopen(path.c_str(), "r");
      if (!file)
        {
          std::stringstream text_stream;
          text_stream << "Could not open the XML file \"" << path << "\".";
          QMessageBox::critical(this, "File open not possible", QString::fromStdString(text_stream.str()));
          return;
        }
      grm_load_graphics_tree(file);
      redraw();
      grm_render();
    }
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
      treewidget->move(this->pos().x() + 0.5 * this->width() - 61, this->pos().y() - 28 + treewidget->geometry().y());
    }
}

void GRPlotWidget::enable_editor_functions()
{
  if (editor_action->isChecked())
    {
      enable_editor = true;
      add_element_action->setVisible(true);
      show_bounding_boxes_action->setVisible(true);
      show_bounding_boxes_action->setChecked(false);
      show_container_action->setVisible(true);
      show_container_action->setChecked(false);
      configuration_menu->menuAction()->setVisible(true);
      editor_action->setText(tr("&Disable Editorview"));

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
      add_element_action->setVisible(false);
      show_bounding_boxes_action->setVisible(false);
      show_bounding_boxes_action->setChecked(false);
      show_container_action->setVisible(false);
      show_container_action->setChecked(false);
      configuration_menu->menuAction()->setVisible(false);
      treewidget->hide();
      add_element_widget->hide();
      editor_action->setText(tr("&Enable Editorview"));
    }
}

void GRPlotWidget::add_element_slot()
{
  if (enable_editor)
    {
      if (add_element_action->isChecked())
        {
          add_element_widget->show();
        }
      else
        {
          add_element_widget->hide();
        }
      add_element_widget->resize(400, height());
      add_element_widget->move(this->pos().x() + this->width() + add_element_widget->width(),
                               this->pos().y() + add_element_widget->geometry().y() - 28);
    }
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
  gr_configurews();
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

void GRPlotWidget::cmd_callback(const grm_request_event_t *event)
{
  if (strcmp(event->request_string, "close") == 0)
    {
      QApplication::quit();
    }
}

Qt::KeyboardModifiers GRPlotWidget::queryKeyboardModifiers()
{
  return modifiers | QApplication::queryKeyboardModifiers();
}

void GRPlotWidget::processTestCommandsFile()
{
  while (test_commands_stream && !test_commands_stream->atEnd())
    {
      QString line = test_commands_stream->readLine();
      QStringList words = line.split(",");
      if (words.size())
        {
          if (words[0] == "keyPressEvent" && words.size() == 2 && words[1].size() == 1 && words[1][0] >= 'A' &&
              words[1][0] <= 'Z')
            {
              QKeyEvent event(QEvent::KeyPress, words[1][0].unicode(), modifiers);
              keyPressEvent(&event);

              QTimer::singleShot(100, this, &GRPlotWidget::processTestCommandsFile);
              return;
            }
          else if (words[0] == "setArg" && words.size() == 3)
            {
              grm_args_push(args_, words[1].toUtf8().constData(), "s", words[2].toUtf8().constData());
              grm_merge(args_);
              arguments_changed = true;
              redraw();
            }
          else if (words[0] == "mouseMoveEvent" && words.size() == 3)
            {
              bool x_flag;
              bool y_flag;
              int x = words[1].toInt(&x_flag);
              int y = words[2].toInt(&y_flag);
              if (x_flag && y_flag)
                {
                  QPoint local_position(x, y);
                  QPoint global_position = mapToGlobal(local_position);
                  QMouseEvent event(QEvent::MouseMove, local_position, global_position, Qt::NoButton, Qt::NoButton,
                                    modifiers);
                  QCursor::setPos(global_position.x(), global_position.y());
                  mouseMoveEvent(&event);

                  QTimer::singleShot(100, this, &GRPlotWidget::processTestCommandsFile);
                  return;
                }
              else
                {
                  std::cerr << "Failed to parse mouseMoveEvent: " << line.toStdString() << std::endl;
                  break;
                }
            }
          else if (words[0] == "mousePressEvent" && words.size() == 2)
            {
              Qt::MouseButton button;
              if (words[1] == "left")
                {
                  button = Qt::MouseButton::LeftButton;
                }
              else if (words[1] == "right")
                {
                  button = Qt::MouseButton::RightButton;
                }
              else
                {
                  std::cerr << "Failed to parse mousePressEvent: " << line.toStdString() << std::endl;
                  break;
                }
              QPoint global_position = QCursor::pos();
              QPoint local_position = mapFromGlobal(global_position);
              QMouseEvent event(QEvent::MouseButtonPress, local_position, global_position, button, Qt::NoButton,
                                modifiers);
              mousePressEvent(&event);
            }
          else if (words[0] == "mouseReleaseEvent" && words.size() == 2)
            {
              Qt::MouseButton button;
              if (words[1] == "left")
                {
                  button = Qt::MouseButton::LeftButton;
                }
              else if (words[1] == "right")
                {
                  button = Qt::MouseButton::RightButton;
                }
              else
                {
                  std::cerr << "Failed to parse mousePressEvent: " << line.toStdString() << std::endl;
                  break;
                }
              QPoint global_position = QCursor::pos();
              QPoint local_position = mapFromGlobal(global_position);
              QMouseEvent event(QEvent::MouseButtonRelease, local_position, global_position, button, Qt::NoButton,
                                modifiers);
              mouseReleaseEvent(&event);
            }
          else if (words[0] == "wheelEvent" && words.size() == 2)
            {
              bool delta_flag;
              int delta = words[1].toInt(&delta_flag);
              if (delta_flag)
                {
                  QPoint global_position = QCursor::pos();
                  QPoint local_position = mapFromGlobal(global_position);
#if QT_VERSION >= 0x060000
                  QWheelEvent event(local_position, global_position, QPoint{}, QPoint{0, delta}, Qt::NoButton,
                                    modifiers, Qt::NoScrollPhase, false, Qt::MouseEventSynthesizedByApplication);
#else
                  QWheelEvent event(local_position, global_position, QPoint{}, QPoint{0, delta}, delta, Qt::Vertical,
                                    Qt::NoButton, modifiers, Qt::NoScrollPhase, Qt::MouseEventSynthesizedByApplication,
                                    false);
#endif
                  wheelEvent(&event);

                  QTimer::singleShot(100, this, &GRPlotWidget::processTestCommandsFile);
                  return;
                }
              else
                {
                  std::cerr << "Failed to parse wheelEvent: " << line.toStdString() << std::endl;
                  break;
                }
            }
          else if (words[0] == "modifiers")
            {
              bool parsing_failed = false;
              modifiers = Qt::NoModifier;
              for (int i = 1; i < words.size(); i++)
                {
                  if (words[i] == "shift")
                    {
                      modifiers |= Qt::ShiftModifier;
                    }
                  else if (words[i] == "alt")
                    {
                      modifiers |= Qt::AltModifier;
                    }
                  else if (words[i] == "control")
                    {
                      modifiers |= Qt::ControlModifier;
                    }
                  else
                    {
                      parsing_failed = true;
                    }
                }
              if (parsing_failed)
                {
                  std::cerr << "Failed to parse modifiers: " << line.toStdString() << std::endl;
                  break;
                }
            }
          else if (words[0] == "sleep" && words.size() == 2)
            {
              bool ms_flag;
              int ms = words[1].toInt(&ms_flag);
              if (ms_flag)
                {
                  QTimer::singleShot(ms, this, &GRPlotWidget::processTestCommandsFile);
                  return;
                }
              else
                {
                  std::cerr << "Failed to parse sleep: " << line.toStdString() << std::endl;
                  break;
                }
            }
          else if (words[0] == "resize" && words.size() == 3)
            {
              bool width_flag;
              bool height_flag;
              int width = words[1].toInt(&width_flag);
              int height = words[2].toInt(&height_flag);
              if (width_flag && height_flag)
                {
                  window()->resize(width, height);

                  QTimer::singleShot(100, this, &GRPlotWidget::processTestCommandsFile);
                  return;
                }
              else
                {
                  std::cerr << "Failed to parse resize: " << line.toStdString() << std::endl;
                  break;
                }
            }
          else if (words[0] == "widgetToPNG" && words.size() == 2)
            {
              QPixmap pixmap(size());
              pixmap.fill(Qt::transparent);
              paint(&pixmap);
              QFile file(words[1]);
              if (file.open(QIODevice::WriteOnly))
                {
                  pixmap.save(&file, "PNG");
                }
              else
                {
                  std::cerr << "Failed to open: " << words[1].toStdString() << std::endl;
                  break;
                }
            }
          else
            {
              std::cerr << "Unknown test event: " << line.toStdString() << std::endl;
              break;
            }
        }
    }
  test_commands_file->close();
  QApplication::quit();
}

std::shared_ptr<GRM::Document> GRPlotWidget::get_schema_tree()
{
  return this->schema_tree;
}

void GRPlotWidget::set_selected_parent(Bounding_object *parent)
{
  this->selected_parent = parent;
}

Bounding_object *GRPlotWidget::get_selected_parent()
{
  return this->selected_parent;
}