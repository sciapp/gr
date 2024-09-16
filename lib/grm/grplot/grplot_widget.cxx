#include <QFile>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <list>
#include <sstream>
#include <functional>
#include <cassert>

#include <gr.h>

#include <QInputDialog>
#include <QtWidgets>
#include <QFileDialog>
#include <QMessageBox>
#include <cfloat>
#include <QtGlobal>
#include <QApplication>
#include <QTimer>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QWindow>
#include <QCompleter>
#include <QCursor>

#include "grplot_widget.hxx"
#include "util.hxx"

#ifndef GR_UNUSED
#define GR_UNUSED(x) (void)(x)
#endif

static std::string file_export;
static QString test_commands_file_path = "";
static QFile *test_commands_file = nullptr;
static QTextStream *test_commands_stream = nullptr;
static Qt::KeyboardModifiers modifiers = Qt::NoModifier;
static std::vector<Bounding_object> cur_moved;
static bool disable_movable_xform = false;
static std::shared_ptr<GRM::Element> global_root;
static bool ctrl_key_mode = false;

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
  treewidget = new TreeWidget(this);
  treewidget->hide();
  table_widget = new TableWidget(this);
  table_widget->hide();
  selected_parent = nullptr;
  csr = new QCursor(Qt::ArrowCursor);
  setCursor(*csr);

  combo_box_attr = QStringList{
      "algorithm",
      "axis_type",
      "clip_region",
      "colormap",
      "error_bar_style",
      "font",
      "font_precision",
      "kind",
      "line_spec",
      "line_type",
      "location",
      "marginal_heatmap_kind",
      "marker_type",
      "model",
      "norm",
      "orientation",
      "projection_type",
      "resample_method",
      "scientific_format",
      "size_x_type",
      "size_y_type",
      "size_x_unit",
      "size_y_unit",
      "style",
      "text_encoding",
      "text_align_horizontal",
      "text_align_vertical",
      "tick_orientation",
      "x_org_pos",
      "y_org_pos",
      "z_org_pos",
  };
  check_box_attr = QStringList{
      "accelerate",
      "active",
      "adjust_x_lim",
      "adjust_y_lim",
      "adjust_z_lim",
      "clip_negative",
      "disable_x_trans",
      "disable_y_trans",
      "draw_grid",
      "grplot",
      "hide",
      "is_major",
      "is_mirrored",
      "keep_aspect_ratio",
      "keep_radii_axes",
      "keep_window",
      "marginal_heatmap_side_plot",
      "mirrored_axis",
      "movable",
      "only_quadratic_aspect_ratio",
      "phi_flip",
      "polar_with_pan",
      "set_text_color_for_background",
      "space",
      "stairs",
      "text_is_title",
      "x_flip",
      "x_grid",
      "x_log",
      "y_flip",
      "y_grid",
      "y_line",
      "y_log",
      "z_flip",
      "z_grid",
      "z_log",
  };

  // add context attributes to combobox list
  auto context_attributes = getContextAttributes();
  for (const auto &attr : context_attributes)
    {
      combo_box_attr.push_back(attr.c_str());
    }

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
  file_menu = new QMenu("&File");
  export_menu = file_menu->addMenu("&Export");

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
      in_listen_mode = true;
      qRegisterMetaType<grm_args_t_wrapper>("grm_args_t_wrapper");
      receiver = new Receiver();
      QObject::connect(receiver, SIGNAL(resultReady(grm_args_t_wrapper)), this, SLOT(received(grm_args_t_wrapper)),
                       Qt::QueuedConnection);
      QObject::connect(this, SIGNAL(pixmapRedrawn()), receiver, SLOT(dataProcessed()), Qt::QueuedConnection);
      receiver->start();

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
      modi_menu = new QMenu("&Modi");

      grm_args_values(args_, "kind", "s", &kind);
      if (grm_args_contains(args_, "error"))
        {
          error = 1;
          fprintf(stderr, "Plot types are not compatible with error-bars. The menu got disabled\n");
        }
      if (strcmp(kind, "contour") == 0 || strcmp(kind, "heatmap") == 0 || strcmp(kind, "imshow") == 0 ||
          strcmp(kind, "marginal_heatmap") == 0 || strcmp(kind, "surface") == 0 || strcmp(kind, "wireframe") == 0 ||
          strcmp(kind, "contourf") == 0)
        {
          auto submenu = type->addMenu("&Marginal-heatmap");

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
          if (strcmp(kind, "marginal_heatmap") == 0)
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
      else if (strcmp(kind, "plot3") == 0 || strcmp(kind, "trisurface") == 0 || strcmp(kind, "tricontour") == 0 ||
               strcmp(kind, "scatter3") == 0 || strcmp(kind, "scatter") == 0)
        {
          plot3Act = new QAction(tr("&Plot3"), this);
          connect(plot3Act, &QAction::triggered, this, &GRPlotWidget::plot3);
          trisurfAct = new QAction(tr("&Trisurface"), this);
          connect(trisurfAct, &QAction::triggered, this, &GRPlotWidget::trisurf);
          tricontAct = new QAction(tr("&Tricontour"), this);
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
          stairsAct = new QAction(tr("&Stairs"), this);
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
      else if (strcmp(kind, "polar_line") == 0 || strcmp(kind, "polar_scatter") == 0)
        {
          polarLineAct = new QAction(tr("&Polar Line"), this);
          connect(polarLineAct, &QAction::triggered, this, &GRPlotWidget::polar_line);
          polarScatterAct = new QAction(tr("&Polar Scatter"), this);
          connect(polarScatterAct, &QAction::triggered, this, &GRPlotWidget::polar_scatter);
          type->addAction(polarLineAct);
          type->addAction(polarScatterAct);
        }
      moveableModeAct = new QAction(tr("&Disable movable transformation"), this);
      connect(moveableModeAct, &QAction::triggered, this, &GRPlotWidget::moveableMode);
      modi_menu->addAction(moveableModeAct);

      if (strcmp(argv[1], "--test") != 0 && !test_commands_stream)
        {
          menu->addMenu(file_menu);
          menu->addMenu(type);
          menu->addMenu(algo);
          menu->addMenu(modi_menu);
        }
    }

  if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
    {
#if !defined(NO_XERCES_C)
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

      save_file_action = new QAction("&Save");
      save_file_action->setShortcut(Qt::CTRL | Qt::Key_S);
      file_menu->addAction(save_file_action);
      QObject::connect(save_file_action, SIGNAL(triggered()), this, SLOT(save_file_slot()));

      load_file_action = new QAction("&Load");
      load_file_action->setShortcut(Qt::CTRL | Qt::Key_O);
      file_menu->addAction(load_file_action);
      QObject::connect(load_file_action, SIGNAL(triggered()), this, SLOT(load_file_slot()));

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

      context_menu = new QMenu("&Data");
      add_context_data = new QMenu("Add Data-Context");
      show_context_action = new QAction(tr("&Display Data-Context"));
      show_context_action->setCheckable(true);
      QObject::connect(show_context_action, SIGNAL(triggered()), this, SLOT(showContextSlot()));
      context_menu->addAction(show_context_action);
      add_context_action = new QAction(tr("&Column files"));
      QObject::connect(add_context_action, SIGNAL(triggered()), this, SLOT(addContextSlot()));
      add_context_data->addAction(add_context_action);
      add_grplot_data_context = new QAction(tr("&Interpret matrix as 1 column data"));
      QObject::connect(add_grplot_data_context, SIGNAL(triggered()), this, SLOT(addGRPlotDataContextSlot()));
      add_context_data->addAction(add_grplot_data_context);
      generate_linear_context_action = new QAction(tr("&Generate linear Data-Context"));
      QObject::connect(generate_linear_context_action, SIGNAL(triggered()), this, SLOT(generateLinearContextSlot()));
      add_context_data->addAction(generate_linear_context_action);

      if (strcmp(argv[1], "--test") != 0 && !test_commands_stream)
        {
          menu->addMenu(editor_menu);
          menu->addMenu(context_menu);
          context_menu->addMenu(add_context_data);
        }
    }
  global_root = grm_get_document_root();
}

GRPlotWidget::~GRPlotWidget()
{
  /*
   * TODO: Delete the receiver. Currently, this is not possible, since the underlying network thread is caught in
   * blocking networking routines and cannot be signaled to exit.
   * if (receiver != nullptr)
   *   {
   *     delete receiver;
   *   }
   */
  grm_args_delete(args_);
  grm_finalize();
}

void GRPlotWidget::attributeComboBoxHandler(const std::string &cur_attr_name, std::string cur_elem_name,
                                            QWidget **lineEdit)
{
  QStringList size_unit_list, colormap_list, font_list, font_precision_list, line_type_list, location_list,
      marker_type_list, text_align_horizontal_list, text_align_vertical_list, algorithm_volume_list, model_list,
      context_attr_list;
  auto size_unit_vec = getSizeUnits();
  size_unit_list.reserve((int)size_unit_vec.size());
  for (auto &i : size_unit_vec)
    {
      size_unit_list.push_back(i.c_str());
    }
  auto colormap_vec = getColormaps();
  colormap_list.reserve((int)colormap_vec.size());
  for (auto &i : colormap_vec)
    {
      colormap_list.push_back(i.c_str());
    }
  auto font_vec = getFonts();
  font_list.reserve((int)font_vec.size());
  for (auto &i : font_vec)
    {
      font_list.push_back(i.c_str());
    }
  auto font_precision_vec = getFontPrecisions();
  font_precision_list.reserve((int)font_precision_vec.size());
  for (auto &i : font_precision_vec)
    {
      font_precision_list.push_back(i.c_str());
    }
  auto line_type_vec = getLineTypes();
  line_type_list.reserve((int)line_type_vec.size());
  for (auto &i : line_type_vec)
    {
      line_type_list.push_back(i.c_str());
    }
  auto location_vec = getLocations();
  location_list.reserve((int)location_vec.size());
  for (auto &i : location_vec)
    {
      location_list.push_back(i.c_str());
    }
  auto marker_type_vec = getMarkerTypes();
  marker_type_list.reserve((int)marker_type_vec.size());
  for (auto &i : marker_type_vec)
    {
      marker_type_list.push_back(i.c_str());
    }
  auto text_align_horizontal_vec = getTextAlignHorizontal();
  text_align_horizontal_list.reserve((int)text_align_horizontal_vec.size());
  for (auto &i : text_align_horizontal_vec)
    {
      text_align_horizontal_list.push_back(i.c_str());
    }
  auto text_align_vertical_vec = getTextAlignVertical();
  text_align_vertical_list.reserve((int)text_align_vertical_vec.size());
  for (auto &i : text_align_vertical_vec)
    {
      text_align_vertical_list.push_back(i.c_str());
    }
  auto algorithm_volume_vec = getAlgorithm();
  algorithm_volume_list.reserve((int)algorithm_volume_vec.size());
  for (auto &i : algorithm_volume_vec)
    {
      algorithm_volume_list.push_back(i.c_str());
    }
  auto model_vec = getModel();
  model_list.reserve((int)model_vec.size());
  for (auto &i : model_vec)
    {
      model_list.push_back(i.c_str());
    }
  table_widget->extractContextNames(grm_get_render()->getContext());
  auto context_attr_vec = table_widget->getContextNames();
  context_attr_list.reserve((int)context_attr_vec.size());
  for (auto &i : context_attr_vec)
    {
      context_attr_list.push_back(i.c_str());
    }

  QStringList axis_type_list{
      "x",
      "y",
  };
  QStringList clip_region_list{
      "quadratic",
      "elliptic",
  };
  QStringList orientation_list{
      "vertical",
      "horizontal",
  };
  QStringList algorithm_marginal_heatmap_list{
      "sum",
      "max",
  };
  QStringList error_bar_style_list{
      "line",
      "area",
  };
  QStringList marginal_heatmap_kind_list{
      "all",
      "line",
  };
  QStringList norm_list{
      "count", "countdensity", "pdf", "probability", "cumcount", "cdf",
  };
  QStringList plot_type_list{
      "2d",
      "3d",
      "polar",
  };
  QStringList projection_type_list{
      "default",
      "orthographic",
      "perspective",
  };
  QStringList resample_method_list{
      "default",
      "nearest",
      "linear",
      "lanczos",
  };
  QStringList size_type_list{
      "double",
      "int",
  };
  QStringList style_list{
      "default",
      "lined",
      "stacked",
  };
  QStringList text_encoding_list{
      "latin1",
      "utf8",
  };
  QStringList line_spec_list{
      "-", ":", ".", "+", "o", "*", "x", "s", "d", "^", "v", "<", ">", "p", "h", "r", "g", "b", "c", "m", "y", "k", "w",
  };
  QStringList org_pos_list{
      "low",
      "high",
  };
  QStringList scientific_format_list{
      "textex",
      "mathtex",
  };
  QStringList tick_orientation_list{
      "up",
      "down",
  };
  QStringList side_region_location_list{"top", "right", "bottom", "left"};
  static std::map<std::string, QStringList> attributeToList{
      {"axis_type", axis_type_list},
      {"error_bar_style", error_bar_style_list},
      {"clip_region", clip_region_list},
      {"size_x_unit", size_unit_list},
      {"size_y_unit", size_unit_list},
      {"colormap", colormap_list},
      {"font", font_list},
      {"font_precision", font_precision_list},
      {"line_type", line_type_list},
      {"marker_type", marker_type_list},
      {"text_align_vertical", text_align_vertical_list},
      {"text_align_horizontal", text_align_horizontal_list},
      {"orientation", orientation_list},
      {"marginal_heatmap_kind", marginal_heatmap_kind_list},
      {"model", model_list},
      {"norm", norm_list},
      {"plot_type", plot_type_list},
      {"projection_type", projection_type_list},
      {"resample_method", resample_method_list},
      {"scientific_format", scientific_format_list},
      {"size_x_type", size_type_list},
      {"size_y_type", size_type_list},
      {"style", style_list},
      {"text_encoding", text_encoding_list},
      {"line_spec", line_spec_list},
      {"x_org_pos", org_pos_list},
      {"y_org_pos", org_pos_list},
      {"z_org_pos", org_pos_list},
      {"tick_orientation", tick_orientation_list},
  };
  // add for all context attributes all possible values
  for (const auto &attr : getContextAttributes())
    {
      if (attributeToList.count(attr) >= 1)
        attributeToList[attr] = context_attr_list;
      else
        attributeToList.emplace(attr, context_attr_list);
    }

  ((QComboBox *)*lineEdit)->setEditable(true);
  if (attributeToList.count(cur_attr_name))
    {
      QStringList list = attributeToList[cur_attr_name];
      for (const auto &elem : list)
        {
          ((QComboBox *)*lineEdit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*lineEdit)->setCompleter(completer);
    }
  else if (cur_attr_name == "algorithm" && cur_elem_name == "series_volume")
    {
      for (const auto &elem : algorithm_volume_list)
        {
          ((QComboBox *)*lineEdit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(algorithm_volume_list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*lineEdit)->setCompleter(completer);
    }
  else if (cur_attr_name == "algorithm" && cur_elem_name == "marginal_heatmap_plot")
    {
      for (const auto &elem : algorithm_marginal_heatmap_list)
        {
          ((QComboBox *)*lineEdit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(algorithm_marginal_heatmap_list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*lineEdit)->setCompleter(completer);
    }
  else if (cur_attr_name == "kind")
    {
      QStringList list;
      QStringList line_group = {"line", "scatter"};
      QStringList heatmap_group = {"contour",          "contourf", "heatmap",  "imshow",
                                   "marginal_heatmap", "surface",  "wireframe"};
      QStringList isosurface_group = {"isosurface", "volume"};
      QStringList plot3_group = {"plot3", "scatter", "scatter3", "tricontour", "trisurface"};
      QStringList barplot_group = {"barplot", "hist", "stem", "stairs"};
      QStringList hexbin_group = {"hexbin", "shade"};
      QStringList other_kinds = {"pie", "polar_heatmap", "polar_histogram", "polar_line", "polar_scatter", "quiver"};
      std::string kind;

      if (util::startsWith(cur_elem_name, "series_")) kind = cur_elem_name.erase(0, 7);
      if (util::endsWith(cur_elem_name, "_plot"))
        kind = cur_elem_name.erase(cur_elem_name.size() - 6, cur_elem_name.size() - 1);

      if (heatmap_group.contains(kind.c_str()))
        {
          list = heatmap_group;
        }
      else if (kind == "line" || kind == "scatter")
        {
          list = line_group;
        }
      else if (kind == "isosurface" || kind == "volume")
        {
          list = isosurface_group;
        }
      else if (plot3_group.contains(kind.c_str()))
        {
          list = plot3_group;
        }
      else if (barplot_group.contains(kind.c_str()))
        {
          list = barplot_group;
        }
      else if (other_kinds.contains(kind.c_str()))
        {
          list.push_back(kind.c_str());
        }
      else if (kind == "hexbin" || kind == "shade")
        {
          list = hexbin_group;
        }

      for (const auto &elem : list)
        {
          ((QComboBox *)*lineEdit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*lineEdit)->setCompleter(completer);
    }
  else if (cur_attr_name == "location" &&
           (cur_elem_name == "side_region" || cur_elem_name == "side_plot_region" || cur_elem_name == "text_region"))
    {
      for (const auto &elem : side_region_location_list)
        {
          ((QComboBox *)*lineEdit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(side_region_location_list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*lineEdit)->setCompleter(completer);
    }
  else if (cur_attr_name == "location" &&
           !(cur_elem_name == "side_region" || cur_elem_name == "side_plot_region" || cur_elem_name == "text_region"))
    {
      for (const auto &elem : location_list)
        {
          ((QComboBox *)*lineEdit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(location_list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*lineEdit)->setCompleter(completer);
    }
}

void GRPlotWidget::advancedAttributeComboBoxHandler(const std::string &cur_attr_name, std::string cur_elem_name,
                                                    QWidget **lineEdit)
{
  if (cur_attr_name == "kind")
    cur_elem_name = "series_" + static_cast<std::string>(current_selection->get_ref()->getAttribute(cur_attr_name));
  attributeComboBoxHandler(cur_attr_name, cur_elem_name, lineEdit);
  ((QComboBox *)*lineEdit)->addItem(""); // entry to remove the attribute

  auto current_text = static_cast<std::string>(current_selection->get_ref()->getAttribute(cur_attr_name));
  if (cur_attr_name == "text_align_vertical" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          textAlignVerticalIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "text_align_horizontal" &&
           current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          textAlignHorizontalIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "algorithm" && current_selection->get_ref()->localName() == "series_volume" &&
           current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text = algorithmIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "model" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text = modelIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "projection_type" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          projectionTypeIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "location" &&
           !(current_selection->get_ref()->localName() == "side_region" ||
             current_selection->get_ref()->localName() == "side_plot_region" ||
             current_selection->get_ref()->localName() == "text_region") &&
           current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text = locationIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "clip_region" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text = clipRegionIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "colormap" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text = colormapIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "text_encoding" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          textEncodingIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "marker_type" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text = markerTypeIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "font" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text = fontIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "font_precision" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          fontPrecisionIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "line_type" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text = lineTypeIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "resample_method" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          resampleMethodIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "scientific_format" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          scientificFormatIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "tick_orientation" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          tickOrientationIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "error_bar_style" && current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          errorBarStyleIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  int index = ((QComboBox *)*lineEdit)->findText(current_text.c_str());
  if (index == -1) index += ((QComboBox *)*lineEdit)->count();
  ((QComboBox *)*lineEdit)->setCurrentIndex(index);
}

void GRPlotWidget::attributeSetForComboBox(const std::string &attr_type, std::shared_ptr<GRM::Element> element,
                                           const std::string &value, const std::string &label)
{
  if (attr_type == "xs:string" || (attr_type == "strint" && !util::is_digits(value)))
    {
      element->setAttribute(label, value);
    }
  else if (attr_type == "xs:integer" || (attr_type == "strint" && util::is_digits(value)))
    {
      if (label == "text_align_vertical")
        {
          element->setAttribute(label, textAlignVerticalStringToInt(value));
        }
      else if (label == "text_align_horizontal")
        {
          element->setAttribute(label, textAlignHorizontalStringToInt(value));
        }
      else if (label == "algorithm")
        {
          element->setAttribute(label, algorithmStringToInt(value));
        }
      else if (label == "model")
        {
          element->setAttribute(label, modelStringToInt(value));
        }
      else if (label == "projection_type")
        {
          element->setAttribute(label, projectionTypeStringToInt(value));
        }
      else if (label == "location")
        {
          element->setAttribute(label, locationStringToInt(value));
        }
      else if (label == "clip_region")
        {
          element->setAttribute(label, clipRegionStringToInt(value));
        }
      else if (label == "colormap")
        {
          element->setAttribute(label, colormapStringToInt(value));
        }
      else if (label == "text_encoding")
        {
          element->setAttribute(label, textEncodingStringToInt(value));
        }
      else if (label == "marker_type")
        {
          element->setAttribute(label, markerTypeStringToInt(value));
        }
      else if (label == "font")
        {
          element->setAttribute(label, fontStringToInt(value));
        }
      else if (label == "font_precision")
        {
          element->setAttribute(label, fontPrecisionStringToInt(value));
        }
      else if (label == "line_type")
        {
          element->setAttribute(label, lineTypeStringToInt(value));
        }
      else if (label == "resample_method")
        {
          element->setAttribute(label, resampleMethodStringToInt(value));
        }
      else if (label == "tick_orientation")
        {
          element->setAttribute(label, tickOrientationStringToInt(value));
        }
      else if (label == "scientific_format")
        {
          element->setAttribute(label, scientificFormatStringToInt(value));
        }
      else if (label == "error_bar_style")
        {
          element->setAttribute(label, errorBarStyleStringToInt(value));
        }
      else
        {
          element->setAttribute(label, std::stoi(value));
        }
    }
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
  std::unordered_map<std::string, std::string> attr_type;

  std::vector<std::string> sorted_names;
  for (const auto &cur_attr_name : current_selection->get_ref()->getAttributeNames())
    {
      sorted_names.push_back(cur_attr_name);
    }
  std::sort(sorted_names.begin(), sorted_names.end());
  for (const auto &cur_attr_name : sorted_names)
    {
      if (util::startsWith(cur_attr_name, "_"))
        {
          continue;
        }
      QString tooltipString = GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
      tooltipString.append(".  Default: ");
      tooltipString.append(GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());

      if (combo_box_attr.contains(cur_attr_name.c_str()))
        {
          lineEdit = new QComboBox(&dialog);
          advancedAttributeComboBoxHandler(cur_attr_name, current_selection->get_ref()->localName(), &lineEdit);
          if (current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
            {
              attr_type.emplace(cur_attr_name, "xs:integer");
            }
          else if (current_selection->get_ref()->getAttribute(cur_attr_name).isDouble())
            {
              attr_type.emplace(cur_attr_name, "xs:double");
            }
          else
            {
              attr_type.emplace(cur_attr_name, "xs:string");
            }
          ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
        }
      else if (check_box_attr.contains(cur_attr_name.c_str()))
        {
          lineEdit = new QCheckBox(&dialog);
          ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
          ((QCheckBox *)lineEdit)
              ->setChecked(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)) == 1);
        }
      else
        {
          if (current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
            {
              attr_type.emplace(cur_attr_name, "xs:integer");
            }
          else if (current_selection->get_ref()->getAttribute(cur_attr_name).isDouble())
            {
              attr_type.emplace(cur_attr_name, "xs:double");
            }
          else
            {
              attr_type.emplace(cur_attr_name, "xs:string");
            }
          lineEdit = new QLineEdit(&dialog);
          ((QLineEdit *)lineEdit)
              ->setText(static_cast<std::string>(current_selection->get_ref()->getAttribute(cur_attr_name)).c_str());
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
                  attr_type.emplace(attr_name, type_name);
                  QString tooltipString =
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), attr_name)[1].c_str();
                  tooltipString.append(".  Default: ");
                  tooltipString.append(
                      GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), attr_name)[0].c_str());

                  if (combo_box_attr.contains(attr_name.c_str()))
                    {
                      lineEdit = new QComboBox(&dialog);
                      advancedAttributeComboBoxHandler(attr_name, current_selection->get_ref()->localName(), &lineEdit);
                      ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                    }
                  else if (check_box_attr.contains(attr_name.c_str()))
                    {
                      lineEdit = new QCheckBox(&dialog);
                      ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                      ((QCheckBox *)lineEdit)
                          ->setChecked(static_cast<int>(current_selection->get_ref()->getAttribute(attr_name)) == 1);
                    }
                  else
                    {
                      lineEdit = new QLineEdit(&dialog);
                      ((QLineEdit *)lineEdit)->setToolTip(tooltipString);
                      ((QLineEdit *)lineEdit)->setText("");
                    }
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
                              attr_type.emplace(attr_name, type_name);
                              QString tooltipString =
                                  GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), attr_name)[1].c_str();
                              tooltipString.append(".  Default: ");
                              tooltipString.append(
                                  GRM::Render::getDefaultAndTooltip(current_selection->get_ref(), attr_name)[0]
                                      .c_str());

                              if (combo_box_attr.contains(attr_name.c_str()))
                                {
                                  lineEdit = new QComboBox(&dialog);
                                  advancedAttributeComboBoxHandler(attr_name, current_selection->get_ref()->localName(),
                                                                   &lineEdit);
                                  ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                                }
                              else if (check_box_attr.contains(attr_name.c_str()))
                                {
                                  lineEdit = new QCheckBox(&dialog);
                                  ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                                  ((QCheckBox *)lineEdit)
                                      ->setChecked(
                                          static_cast<int>(current_selection->get_ref()->getAttribute(attr_name)) == 1);
                                }
                              else
                                {
                                  lineEdit = new QLineEdit(&dialog);
                                  ((QLineEdit *)lineEdit)->setToolTip(tooltipString);
                                  ((QLineEdit *)lineEdit)->setText("");
                                }
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

                  attr_type.emplace("Colorrep-index", "xs:string");
                  labels << text_label;
                  fields << lineEdit;

                  lineEdit = new QLineEdit(&dialog);
                  ((QLineEdit *)lineEdit)->setText("");
                  text_label = QString("<span style='color:#ff0000;'>%1</span>").arg("Colorrep-value");
                  form->addRow(text_label, lineEdit);

                  attr_type.emplace("Colorrep-value", "xs:string");
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
          if (util::startsWith(labels[i].toStdString(), "<span style='color:#ff0000;'>") &&
              util::endsWith(labels[i].toStdString(), "</span>"))
            {
              labels[i].remove(0, 29);
              labels[i].remove(labels[i].size() - 7, 7);
            }
          std::string attr_name = labels[i].toStdString();
          if (typeid(field) == typeid(QLineEdit) && ((QLineEdit *)fields[i])->isModified())
            {
              std::string name = std::string(current_selection->get_ref()->getAttribute("name"));
              if (((QLineEdit *)fields[i])->text().toStdString().empty() && labels[i].toStdString() != "tick_label" &&
                  labels[i].toStdString() != "text")
                {
                  /* remove attributes from tree when the value got removed */
                  current_selection->get_ref()->removeAttribute(labels[i].toStdString());
                }
              else
                {
                  if (labels[i].toStdString() == "text")
                    {
                      const std::string value = ((QLineEdit *)fields[i])->text().toStdString();
                      if (attr_type[attr_name] == "xs:string" ||
                          (attr_type[attr_name] == "strint" && !util::is_digits(value)))
                        {
                          if (current_selection->get_ref()->parentElement()->localName() == "text_region")
                            {
                              current_selection->get_ref()->parentElement()->parentElement()->setAttribute(
                                  "text_content", value);
                            }
                          else if (name == "xlabel" || name == "ylabel")
                            {
                              current_selection->get_ref()
                                  ->parentElement()
                                  ->parentElement()
                                  ->querySelectors(name)
                                  ->setAttribute(name, value);
                            }
                        }
                      else if (attr_type[attr_name] == "xs:double")
                        {
                          current_selection->get_ref()->parentElement()->setAttribute(labels[i].toStdString(),
                                                                                      std::stod(value));
                        }
                      else if (attr_type[attr_name] == "xs:integer" ||
                               (attr_type[attr_name] == "strint" && util::is_digits(value)))
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
                      if (attr_type[attr_name] == "xs:string" ||
                          (attr_type[attr_name] == "strint" && !util::is_digits(value)))
                        {
                          current_selection->get_ref()->setAttribute(labels[i].toStdString(), value);
                        }
                      else if (attr_type[attr_name] == "xs:double")
                        {
                          current_selection->get_ref()->setAttribute(labels[i].toStdString(), std::stod(value));
                        }
                      else if (attr_type[attr_name] == "xs:integer" ||
                               (attr_type[attr_name] == "strint" && util::is_digits(value)))
                        {
                          current_selection->get_ref()->setAttribute(labels[i].toStdString(), std::stoi(value));
                        }
                    }
                }
            }
          else if (typeid(field) == typeid(QComboBox))
            {
              int index = ((QComboBox *)fields[i])->currentIndex();
              if (((QComboBox *)fields[i])->itemText(index).toStdString().empty())
                {
                  /* remove attributes from tree when the value got removed */
                  current_selection->get_ref()->removeAttribute(labels[i].toStdString());
                }
              else
                {
                  const std::string value = ((QComboBox *)fields[i])->itemText(index).toStdString();
                  attributeSetForComboBox(attr_type[attr_name], current_selection->get_ref(), value,
                                          (labels[i]).toStdString());
                }
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
      tree_update = true;
      clicked.clear();
      if (getenv("GRM_DEBUG"))
        {
          std::cerr << toXML(grm_get_document_root(),
                             GRM::SerializerOptions{std::string(2, ' '),
                                                    GRM::SerializerOptions::InternalAttributesFormat::Plain})
                    << "\n";
        }
      reset_pixmap();
    }
  else
    {
      tree_update = false;
    }
}

void GRPlotWidget::draw()
{
  static bool called_at_least_once = false;
  if (!file_export.empty())
    {
      static char file[50];

      if (global_root == nullptr) global_root = grm_get_document_root();
      auto plot_elem = global_root->querySelectors("plot");
      auto kind = static_cast<std::string>(plot_elem->getAttribute("kind"));
      snprintf(file, 50, "grplot_%s.%s", kind.c_str(), file_export.c_str());
      grm_export(file);
    }
  bool was_successful;
  if (!called_at_least_once || in_listen_mode)
    {
      /* Call `grm_plot` at least once to initialize the internal argument container structure,
       * but use `grm_render` afterwards, so the graphics tree is not deleted every time. */
      was_successful = grm_plot(nullptr);
    }
  else
    {
      was_successful = grm_render();
    }
  assert(was_successful);
  called_at_least_once = true;
}

void GRPlotWidget::redraw(bool update_tree)
{
  redraw_pixmap = true;
  tree_update = update_tree;

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
      if (accumulated_tooltip != nullptr) tooltips.emplace_back(accumulated_tooltip);
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
              tooltips.emplace_back(current_tooltip);
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
  if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
    {
      if (!table_widget->isVisible() && show_context_action->isChecked()) show_context_action->setChecked(false);
      if (!treewidget->isVisible() && show_container_action->isChecked()) show_container_action->setChecked(false);
      if (!add_element_widget->isVisible() && add_element_action->isChecked()) add_element_action->setChecked(false);
    }
  util::unused(event);
  paint(this);
}

void GRPlotWidget::paint(QPaintDevice *paint_device)
{
  QPainter painter;
  std::stringstream addresses;
  std::string kind;

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

      if (tree_update) treewidget->updateData(grm_get_document_root());
      emit pixmapRedrawn();
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
                  const auto *single_tooltip = tooltip.get<grm_tooltip_info_t>();
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
                  const auto *accumulated_tooltip = tooltip.get<grm_accumulated_tooltip_info_t>();
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
                      info_parts.emplace_back("<br>\n");
                      info_parts.push_back(util::string_format(accumulatedTooltipTemplate, y_label.c_str(), y));
                    }
                  std::ostringstream info_stream;
                  std::copy(info_parts.begin(), info_parts.end(), std::ostream_iterator<std::string>(info_stream));
                  info = info_stream.str();
                }
              label.setDefaultStyleSheet(QString::fromStdString(tooltipStyle));
              label.setHtml(QString::fromStdString(info));
              if (global_root == nullptr) global_root = grm_get_document_root();
              auto plot_elem = global_root->querySelectors("plot");
              kind = static_cast<std::string>(plot_elem->getAttribute("kind"));
              if (kind == "heatmap" || kind == "marginal_heatmap")
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
          if (!current_selections.empty())
            {
              for (const auto &selection : current_selections)
                {
                  selection->get_ref()->setAttribute("_selected", 0);
                }
              current_selections.clear();
            }
          current_selection = nullptr;
          mouse_move_selection = nullptr;
          treewidget->updateData(grm_get_document_root());
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
          // to remove y_line, title, xlabel and ylabel from axis Node
          auto elem_name = (std::string)current_selection->get_ref()->getAttribute("name");
          if (current_selection->get_ref()->parentElement()->hasAttribute(elem_name))
            {
              if (elem_name == "y_line")
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
              // to remove x_tick_labels, y_tick_labels from coordinate_system
              elem_name = (std::string)parent->getAttribute("name");
              if (tmp_parent->hasAttribute(elem_name)) tmp_parent->removeAttribute(elem_name);
              parent->remove();
              parent = tmp_parent;
            }
          current_selection->get_ref()->remove();
          // to prevent recreation of the tree a new flag is introduced
          if (parent->localName() == "root" && !parent->hasChildNodes())
            parent->setAttribute("_removed_children", true);
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
          treewidget->updateData(grm_get_document_root());
          treewidget->selectItem(current_selection->get_ref());
        }
      else if (event->key() == Qt::Key_Control)
        {
          ctrl_key_mode = true;
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
  if (enable_editor)
    {
      if (event->key() == Qt::Key_Control) ctrl_key_mode = false;
    }
  else
    {
      collectTooltips();
    }
  update();
}

void GRPlotWidget::mouseMoveEvent(QMouseEvent *event)
{
  amount_scrolled = 0;

  if (enable_editor)
    {
      int x, y;
      getMousePos(event, &x, &y);
      if (mouseState.mode == MouseState::Mode::move_selected && !ctrl_key_mode)
        {
          grm_args_t *args = grm_args_new();

          grm_args_push(args, "x", "i", mouseState.anchor.x());
          grm_args_push(args, "y", "i", mouseState.anchor.y());
          grm_args_push(args, "x_shift", "i", x - mouseState.anchor.x());
          grm_args_push(args, "y_shift", "i", y - mouseState.anchor.y());

          /* get the correct cursor and sets it */
          int cursor_state = grm_get_hover_mode(x, y, disable_movable_xform);
          grm_args_push(args, "move_selection", "i", 1);

          grm_input(args);
          grm_args_delete(args);

          mouseState.anchor = event->pos();
          redraw();
        }
      else
        {
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
          grm_args_push(args, "x_shift", "i", x - mouseState.anchor.x());
          grm_args_push(args, "y_shift", "i", y - mouseState.anchor.y());

          grm_input(args);
          grm_args_delete(args);

          mouseState.anchor = event->pos();

          redraw();
        }
      else if (mouseState.mode == MouseState::Mode::movable_xform)
        {
          int x, y;
          getMousePos(event, &x, &y);
          grm_args_t *args = grm_args_new();

          grm_args_push(args, "x", "i", mouseState.anchor.x());
          grm_args_push(args, "y", "i", mouseState.anchor.y());
          grm_args_push(args, "x_shift", "i", x - mouseState.anchor.x());
          grm_args_push(args, "y_shift", "i", y - mouseState.anchor.y());
          if (disable_movable_xform) grm_args_push(args, "disable_movable_trans", "i", disable_movable_xform);

          /* get the correct cursor and sets it */
          int cursor_state = grm_get_hover_mode(x, y, disable_movable_xform);
          grm_args_push(args, "movable_state", "i", cursor_state);

          grm_input(args);
          grm_args_delete(args);

          mouseState.anchor = event->pos();
          redraw();
        }
      else
        {
          std::string kind;
          int x, y;
          getMousePos(event, &x, &y);
          collectTooltips();
          if (global_root == nullptr) global_root = grm_get_document_root();
          auto plot_elem = global_root->querySelectors("plot");
          if (plot_elem)
            {
              kind = static_cast<std::string>(plot_elem->getAttribute("kind"));
              if (kind == "marginal_heatmap")
                {
                  grm_args_t *input_args;
                  input_args = grm_args_new();

                  grm_args_push(input_args, "x", "i", x);
                  grm_args_push(input_args, "y", "i", y);
                  grm_input(input_args);
                }

              /* get the correct cursor and sets it */
              int cursor_state = grm_get_hover_mode(x, y, disable_movable_xform);
              if (cursor_state == DEFAULT_HOVER_MODE)
                {
                  csr->setShape(Qt::ArrowCursor);
                }
              else if (cursor_state == MOVABLE_HOVER_MODE)
                {
                  csr->setShape(Qt::OpenHandCursor);
                }
              else if (cursor_state == INTEGRAL_SIDE_HOVER_MODE)
                {
                  csr->setShape(Qt::SizeHorCursor);
                }
              setCursor(*csr);

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
      int x, y;
      getMousePos(event, &x, &y);
      if (current_selections.empty() || ctrl_key_mode)
        {
          mouseState.mode = MouseState::Mode::pan;
        }
      else
        {
          mouseState.mode = MouseState::Mode::move_selected;
        }
      mouseState.anchor = event->pos();

      int cursor_state = grm_get_hover_mode(x, y, disable_movable_xform);
      if (cursor_state != DEFAULT_HOVER_MODE)
        {
          grm_args_t *args = grm_args_new();
          grm_args_push(args, "clear_locked_state", "i", 1);
          grm_input(args);
          grm_args_delete(args);

          mouseState.mode = MouseState::Mode::movable_xform;
        }

      if (enable_editor)
        {
          amount_scrolled = 0;
          auto cur_clicked = bounding_logic->get_bounding_objects_at_point(x, y);
          if (cur_clicked.empty())
            {
              clicked.clear();
              current_selection = nullptr;
              treewidget->updateData(grm_get_document_root());
              update();
              return;
            }
          else
            {
              clicked = cur_clicked;
            }
          current_selection = &clicked[0];
          if (ctrl_key_mode)
            {
              bool removed_selection = false;
              std::unique_ptr<Bounding_object> tmp(new Bounding_object(clicked[0]));
              for (auto it = std::begin(current_selections); it != std::end(current_selections); ++it)
                {
                  if ((*it)->get_ref() == tmp->get_ref())
                    {
                      (*it)->get_ref()->setAttribute("_selected", 0);
                      it = current_selections.erase(it);
                      removed_selection = true;
                      break;
                    }
                }
              if (!removed_selection)
                {
                  tmp->get_ref()->setAttribute("_selected", 1);
                  add_current_selection(std::move(tmp));
                }
              mouseState.mode = MouseState::Mode::move_selected;
            }
          treewidget->updateData(grm_get_document_root());
          treewidget->selectItem(current_selection->get_ref());
          mouse_move_selection = nullptr;
        }
      else
        {
          if (cursor_state == DEFAULT_HOVER_MODE)
            {
              csr->setShape(Qt::ArrowCursor);
            }
          else if (cursor_state == MOVABLE_HOVER_MODE)
            {
              csr->setShape(Qt::ClosedHandCursor);
            }
          else if (cursor_state == INTEGRAL_SIDE_HOVER_MODE)
            {
              csr->setShape(Qt::SizeHorCursor);
            }
          setCursor(*csr);
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
  else if (mouseState.mode == MouseState::Mode::movable_xform)
    {
      mouseState.mode = MouseState::Mode::normal;

      if (!enable_editor)
        {
          int cursor_state = grm_get_hover_mode(x, y, disable_movable_xform);
          if (cursor_state == DEFAULT_HOVER_MODE)
            {
              csr->setShape(Qt::ArrowCursor);
            }
          else if (cursor_state == MOVABLE_HOVER_MODE)
            {
              csr->setShape(Qt::OpenHandCursor);
            }
          else if (cursor_state == INTEGRAL_SIDE_HOVER_MODE)
            {
              csr->setShape(Qt::SizeHorCursor);
            }
          setCursor(*csr);
        }
    }
  else if (mouseState.mode == MouseState::Mode::move_selected)
    {
      mouseState.mode = MouseState::Mode::normal;
    }

  grm_input(args);
  grm_args_delete(args);

  redraw();
}

void GRPlotWidget::resizeEvent(QResizeEvent *event)
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  auto figure = global_root->querySelectors("[active=1]");
  if (figure != nullptr)
    {
      figure->setAttribute("size_x", (double)event->size().width());
      figure->setAttribute("size_y", (double)event->size().height());
    }

  current_selection = nullptr;
  for (const auto &selection : current_selections)
    {
      selection->get_ref()->setAttribute("_selected", 0);
    }
  current_selections.clear();
  mouse_move_selection = nullptr;
  amount_scrolled = 0;
  clicked.clear();
  tooltips.clear();
  reset_pixmap();
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
                              treewidget->updateData(grm_get_document_root());
                              treewidget->selectItem(current_selection->get_ref());
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
                  for (int i = (int)clicked.size() - 1; i >= 0; i--)
                    {
                      if (clicked[i].get_id() == current_selection->get_id())
                        {
                          if (i - 1 > 0)
                            {
                              current_selection = &clicked[i - 1];
                              treewidget->updateData(grm_get_document_root());
                              treewidget->selectItem(current_selection->get_ref());
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
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface",  "series_wireframe",
                                                 "series_contour",        "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "heatmap");
        }
    }
  redraw();
}

void GRPlotWidget::marginalheatmapall()
{
  algo->menuAction()->setVisible(true);
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"series_heatmap", "series_surface",  "series_wireframe",
                                                 "series_contour", "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          if (series_elem->parentElement()->parentElement()->localName() != "marginal_heatmap_plot")
            series_elem->setAttribute("kind", "marginal_heatmap");
        }
    }
  for (const auto &series_elem : global_root->querySelectorsAll("marginal_heatmap_plot"))
    {
      series_elem->setAttribute("marginal_heatmap_kind", "all");
    }
  redraw();
}

void GRPlotWidget::marginalheatmapline()
{
  algo->menuAction()->setVisible(true);
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"series_heatmap", "series_surface",  "series_wireframe",
                                                 "series_contour", "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          if (series_elem->parentElement()->parentElement()->localName() != "marginal_heatmap_plot")
            series_elem->setAttribute("kind", "marginal_heatmap");
        }
    }
  for (const auto &series_elem : global_root->querySelectorsAll("marginal_heatmap_plot"))
    {
      series_elem->setAttribute("marginal_heatmap_kind", "line");
    }
  redraw();
}

void GRPlotWidget::line()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  for (const auto &elem : global_root->querySelectorsAll("series_scatter"))
    {
      elem->setAttribute("kind", "line");
    }

  // to get the same lines then before all lines have to exist during render call so that the linespec work properly
  for (const auto &elem : global_root->querySelectorsAll("series_line"))
    {
      elem->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::sumalgorithm()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  for (const auto &elem : global_root->querySelectorsAll("marginal_heatmap_plot"))
    {
      elem->setAttribute("algorithm", "sum");
    }
  redraw();
}

void GRPlotWidget::maxalgorithm()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  for (const auto &elem : global_root->querySelectorsAll("marginal_heatmap_plot"))
    {
      elem->setAttribute("algorithm", "max");
    }
  redraw();
}

void GRPlotWidget::volume()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  for (const auto &elem : global_root->querySelectorsAll("series_isosurface"))
    {
      elem->setAttribute("kind", "volume");
    }
  redraw();
}
void GRPlotWidget::isosurface()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  for (const auto &elem : global_root->querySelectorsAll("series_volume"))
    {
      elem->setAttribute("kind", "isosurface");
    }
  redraw();
}

void GRPlotWidget::surface()
{
  algo->menuAction()->setVisible(false);
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_heatmap",  "series_wireframe",
                                                 "series_contour",        "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "surface");
        }
    }
  redraw();
}
void GRPlotWidget::wireframe()
{
  algo->menuAction()->setVisible(false);
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface",  "series_heatmap",
                                                 "series_contour",        "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "wireframe");
        }
    }
  redraw();
}

void GRPlotWidget::contour()
{
  algo->menuAction()->setVisible(false);
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface",  "series_wireframe",
                                                 "series_heatmap",        "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "contour");
        }
    }
  redraw();
}

void GRPlotWidget::imshow()
{
  algo->menuAction()->setVisible(false);
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface",  "series_wireframe",
                                                 "series_contour",        "series_contourf", "series_heatmap"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "imshow");
        }
    }
  redraw();
}

void GRPlotWidget::plot3()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"series_scatter3", "series_tricontour", "series_trisurface",
                                                 "series_scatter"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "plot3");
        }
    }
  redraw();
}

void GRPlotWidget::contourf()
{
  algo->menuAction()->setVisible(false);
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface", "series_wireframe",
                                                 "series_contour",        "series_heatmap", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "contourf");
        }
    }
  redraw();
}

void GRPlotWidget::trisurf()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"series_scatter3", "series_tricontour", "series_plot3",
                                                 "series_scatter"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "trisurface");
        }
    }
  redraw();
}

void GRPlotWidget::tricont()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"series_scatter3", "series_plot3", "series_trisurface",
                                                 "series_scatter"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "tricontour");
        }
    }
  redraw();
}

void GRPlotWidget::scatter3()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"series_plot3", "series_tricontour", "series_trisurface",
                                                 "series_scatter"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "scatter3");
        }
    }
  redraw();
}

void GRPlotWidget::scatter()
{
  auto root = grm_get_document_root();
  for (const auto &elem : root->querySelectorsAll("series_line"))
    {
      elem->setAttribute("kind", "scatter");
    }
  redraw();
}

void GRPlotWidget::hist()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"series_barplot", "series_stem", "series_stairs"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "hist");
        }
    }

  // to get the same bars then before all bars have to exist during render call so that the linespec work properly
  for (const auto &elem : global_root->querySelectorsAll("series_hist"))
    {
      elem->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::barplot()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"series_hist", "series_stem", "series_stairs"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "barplot");
        }
    }

  // to get the same bars then before all bars have to exist during render call so that the linespec work properly
  for (const auto &elem : global_root->querySelectorsAll("series_barplot"))
    {
      elem->removeAttribute("fill_color_ind");
      elem->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::stairs()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"series_barplot", "series_stem", "series_hist"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "stairs");
        }
    }

  // to get the same lines then before all lines have to exist during render call so that the linespec work properly
  for (const auto &elem : global_root->querySelectorsAll("series_stairs"))
    {
      elem->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::stem()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  std::vector<std::string> valid_series_names = {"series_barplot", "series_hist", "series_stairs"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = global_root->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "stem");
        }
    }
  redraw();
}

void GRPlotWidget::shade()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  for (const auto &elem : global_root->querySelectorsAll("series_hexbin"))
    {
      elem->setAttribute("kind", "shade");
    }
  redraw();
}

void GRPlotWidget::hexbin()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  for (const auto &elem : global_root->querySelectorsAll("series_shade"))
    {
      elem->setAttribute("kind", "hexbin");
    }
  redraw();
}

void GRPlotWidget::polar_line()
{
  if (global_root == nullptr) global_root = grm_get_document_root();
  for (const auto &elem : global_root->querySelectorsAll("series_polar_scatter"))
    {
      elem->setAttribute("kind", "polar_line");
    }
  redraw();
}

void GRPlotWidget::polar_scatter()
{
  auto root = grm_get_document_root();
  for (const auto &elem : root->querySelectorsAll("series_polar_line"))
    {
      elem->setAttribute("kind", "polar_scatter");
    }
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

void GRPlotWidget::moveableMode()
{
  disable_movable_xform = !disable_movable_xform;
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
  if (global_root == nullptr) global_root = grm_get_document_root();
  double xmin, xmax, ymin, ymax;
  int id;

  if (enable_editor)
    {
      painter.setPen(QPen(QColor(255, 0, 0, 100)));

      for (const auto &cur_child : global_root->querySelectorsAll("[_bbox_id]"))
        {
          id = static_cast<int>(cur_child->getAttribute("_bbox_id"));
          xmin = static_cast<double>(cur_child->getAttribute("_bbox_x_min"));
          xmax = static_cast<double>(cur_child->getAttribute("_bbox_x_max"));
          ymin = static_cast<double>(cur_child->getAttribute("_bbox_y_min"));
          ymax = static_cast<double>(cur_child->getAttribute("_bbox_y_max"));

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
      if (!current_selections.empty())
        {
          for (const auto &selection : current_selections)
            {
              painter.fillRect(selection->boundingRect(), QBrush(QColor(190, 210, 232, 150), Qt::Dense7Pattern));
              if (selection->get_ref() != nullptr)
                painter.drawText(selection->boundingRect().topLeft() + QPointF(5, 10),
                                 selection->get_ref()->localName().c_str());
            }
        }

      // only highlight current_selection when ctrl isn't pressed, else an element doesn't get visually removed from the
      // list cause it still gets highlighted
      if (!ctrl_key_mode && mouseState.mode != MouseState::Mode::move_selected)
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
              painter.fillRect(mouse_move_selection->boundingRect(),
                               QBrush(QColor(190, 210, 232, 150), Qt::SolidPattern));
              if (mouse_move_selection->get_ref() != nullptr)
                painter.drawText(mouse_move_selection->boundingRect().topLeft() + QPointF(5, 10),
                                 mouse_move_selection->get_ref()->localName().c_str());
            }
        }
      if (selected_parent != nullptr)
        {
          auto rect = selected_parent->boundingRect();
          if (selected_parent->get_ref() != nullptr)
            {
              auto bbox_xmin = static_cast<double>(selected_parent->get_ref()->getAttribute("_bbox_x_min"));
              auto bbox_xmax = static_cast<double>(selected_parent->get_ref()->getAttribute("_bbox_x_max"));
              auto bbox_ymin = static_cast<double>(selected_parent->get_ref()->getAttribute("_bbox_y_min"));
              auto bbox_ymax = static_cast<double>(selected_parent->get_ref()->getAttribute("_bbox_y_max"));
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
  for (const auto &selection : current_selections)
    {
      selection->get_ref()->setAttribute("_selected", 0);
    }
  current_selections.clear();
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

void GRPlotWidget::load_file_slot()
{
  if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
    {
#ifndef NO_XERCES_C
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
      global_root = grm_get_document_root();
      redraw();
      if (table_widget->isVisible()) table_widget->updateData(grm_get_render()->getContext());
#else
      std::stringstream text_stream;
      text_stream << "XML support not compiled in. Please recompile GRPlot with libxml2 support.";
      QMessageBox::critical(this, "File open not possible", QString::fromStdString(text_stream.str()));
      return;
#endif
    }
}

void GRPlotWidget::save_file_slot()
{
  if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
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
      auto graphics_tree_str = std::unique_ptr<char, decltype(&std::free)>(grm_dump_graphics_tree_str(), std::free);
      save_file_stream << graphics_tree_str.get() << std::endl;
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
      treewidget->resize(350, height());
      treewidget->move((int)(this->pos().x() + 0.5 * this->width() - 61),
                       this->pos().y() - 28 + treewidget->geometry().y());
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

QSize GRPlotWidget::sizeHint() const
{
  if (size_hint.isValid())
    {
      return size_hint;
    }
  else
    {
      return QWidget::sizeHint();
    }
}

void GRPlotWidget::screenChanged()
{
  gr_configurews();
  redraw();
}

void GRPlotWidget::showContextSlot()
{
  if (show_context_action->isChecked())
    {
      auto context = grm_get_render()->getContext();
      table_widget->updateData(context);
      table_widget->show();
    }
  else
    {
      table_widget->hide();
    }
  table_widget->resize(width(), 350);
  table_widget->move((int)(this->pos().x() + 0.5 * this->width() - 61),
                     this->pos().y() - 28 + table_widget->geometry().y());
}

void GRPlotWidget::addContextSlot()
{
  std::string path =
      QFileDialog::getOpenFileName(this, "Open column data file", QDir::homePath(), "(*.dat *.csv *.xyz)")
          .toStdString();
  if (path.empty()) return;

  // convert the data
  if (!grm_context_data_from_file(grm_get_render()->getContext(), path))
    {
      fprintf(stderr, "Could not interpret the file to context data\n");
      return;
    }
  auto context = grm_get_render()->getContext();
  table_widget->updateData(context);
}

void GRPlotWidget::addGRPlotDataContextSlot()
{
  std::string path =
      QFileDialog::getOpenFileName(this, "Interpret matrix as 1 column data", QDir::homePath(), "(*.dat *.csv *.xyz)")
          .toStdString();
  if (path.empty()) return;

  // convert the data
  if (!grm_context_data_from_file(grm_get_render()->getContext(), path, true))
    {
      fprintf(stderr, "Could not interpret the file to context data\n");
      return;
    }
  auto context = grm_get_render()->getContext();
  table_widget->updateData(context);
}

void GRPlotWidget::generateLinearContextSlot()
{
  QList<QWidget *> fields;
  std::vector<std::string> label = {"Context-Data key:", "Min value:", "Max value", "Number of data"};
  QDialog dialog(this);
  QString title("Generate linear context entry");
  dialog.setWindowTitle(title);
  auto form = new QFormLayout;

  // needed information to generate linear data for the context
  for (int i = 0; i < 4; i++)
    {
      auto lineEdit = new QLineEdit(&dialog);
      ((QLineEdit *)lineEdit)->setText("");
      auto text_label = QString(label[i].c_str());
      form->addRow(text_label, lineEdit);
      fields << lineEdit;
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
      int n;
      double start, end;
      std::vector<std::string> values;
      std::vector<double> data_vec;
      std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();

      for (int i = 0; i < 4; i++)
        {
          auto &field = *fields[i];
          auto value = ((QLineEdit *)fields[i])->text().toStdString();
          if (value.empty())
            {
              fprintf(stderr, "All fields must be filled to generate linear context data\n");
              return;
            }
          values.push_back(value);
        }

      // convert entries into linear data vec
      try
        {
          start = std::stod(values[1]);
          end = std::stod(values[2]);
          n = std::stoi(values[3]);
          for (int i = 0; i < n; i++)
            {
              data_vec.push_back(start + i * (end - start) / (n - 1));
            }

          (*context)[values[0]] = data_vec;
          table_widget->updateData(context);
        }
      catch (std::invalid_argument &e)
        {
          fprintf(stderr, "Invalid argument for generate linear context parameter\n");
          return;
        }
    }
}

void GRPlotWidget::size_callback(const grm_event_t *new_size_object)
{
  // TODO: Get Plot ID
  auto event_size = QSize(new_size_object->size_event.width, new_size_object->size_event.height);
  if (this->size() != event_size)
    {
      size_hint = event_size;
      setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      window()->adjustSize();
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
      if (line.startsWith("#")) continue;
      QStringList words = line.split(",");
      if (!words.empty())
        {
          if (words[0] == "keyPressEvent" && words.size() == 2 && words[1].size() == 1 && words[1][0] >= 'A' &&
              words[1][0] <= 'Z')
            {
              QKeyEvent event(QEvent::KeyPress, words[1][0].unicode(), modifiers);
              keyPressEvent(&event);

              QTimer::singleShot(100, this, &GRPlotWidget::processTestCommandsFile);
              return;
            }
          else if (words[0] == "setArg" && words.size() >= 4)
            {
              int n = words.size();
              std::string element_name;
              for (int k = 1; k < n - 2; k++)
                {
                  element_name += words[k].toUtf8().constData();
                  if (k < n - 3) element_name += " ";
                }
              if (global_root == nullptr) global_root = grm_get_document_root();
              auto series_elements = global_root->querySelectorsAll(element_name);
              for (const auto &elem : series_elements)
                {
                  elem->setAttribute(words[n - 2].toUtf8().constData(), words[n - 1].toUtf8().constData());
                }
              auto value = words[n - 1].toUtf8().constData();

              if (strcmp(value, "line") == 0)
                {
                  // to get the same lines then before all lines have to exist during render call so that the linespec
                  // work properly
                  for (const auto &elem : global_root->querySelectorsAll("series_line"))
                    {
                      elem->setAttribute("_update_required", true);
                    }
                }
              if (strcmp(value, "barplot") == 0 || strcmp(value, "hist") == 0 || strcmp(value, "stairs") == 0)
                {
                  // to get the same barplots then before all lines have to exist during render call so that the
                  // linespec work properly
                  for (const auto &elem : global_root->querySelectorsAll("series_" + std::string(value)))
                    {
                      if (strcmp(value, "barplot") == 0) elem->removeAttribute("fill_color_ind");
                      elem->setAttribute("_update_required", true);
                    }
                }
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
                  tooltips.clear();

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
          else if (words[0] == "openXML" && words.size() == 2)
            {
#ifndef NO_XERCES_C
              auto file = fopen(words[1].toStdString().c_str(), "r");
              if (file)
                {
                  grm_load_graphics_tree(file);
                  global_root = grm_get_document_root();
                  tooltips.clear();
                  redraw();
                  QTimer::singleShot(100, this, &GRPlotWidget::processTestCommandsFile);
                  return;
                }
#else
              std::cerr << "Xerces-C++ support not compiled in. XML files cannot be openend." << std::endl;
#endif
            }
          else if (words[0] == "saveXML" && words.size() == 2)
            {
              std::ofstream save_file_stream(words[1].toStdString());
              if (save_file_stream)
                {
                  auto graphics_tree_str =
                      std::unique_ptr<char, decltype(&std::free)>(grm_dump_graphics_tree_str(), std::free);
                  save_file_stream << graphics_tree_str.get() << std::endl;
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

void GRPlotWidget::set_current_selection(Bounding_object *p_current_selection)
{
  this->current_selection = p_current_selection;
}

Bounding_object *GRPlotWidget::get_current_selection()
{
  return this->current_selection;
}

QStringList GRPlotWidget::getCheckBoxAttributes()
{
  return check_box_attr;
}

QStringList GRPlotWidget::getComboBoxAttributes()
{
  return combo_box_attr;
}

void GRPlotWidget::add_current_selection(std::unique_ptr<Bounding_object> curr_selection)
{
  current_selections.emplace_back(std::move(curr_selection));
}

std::list<std::unique_ptr<Bounding_object>>::iterator
GRPlotWidget::erase_current_selection(std::list<std::unique_ptr<Bounding_object>>::const_iterator current_selection)
{
  return current_selections.erase(current_selection);
}

const std::list<std::unique_ptr<Bounding_object>> &GRPlotWidget::get_current_selections() const
{
  return current_selections;
}
