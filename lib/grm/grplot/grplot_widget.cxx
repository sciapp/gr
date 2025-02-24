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

#ifndef GR_UNUSED
#define GR_UNUSED(x) (void)(x)
#endif

static std::string file_export;
static QFile *test_commands_file = nullptr;
static QTextStream *test_commands_stream = nullptr;
static Qt::KeyboardModifiers modifiers = Qt::NoModifier;
static std::vector<Bounding_object> cur_moved;
static bool disable_movable_xform = false;
static bool ctrl_key_mode = false;
static bool mouse_move_triggert = false;
static std::weak_ptr<GRM::Element> previous_active_plot;
static bool active_plot_changed = false;

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


GRPlotWidget::GRPlotWidget(QMainWindow *parent, int argc, char **argv, bool listen_mode, bool test_mode,
                           QString test_commands)
    : QWidget(parent), pixmap(), redraw_pixmap(RedrawType::none), args_(nullptr), rubberBand(nullptr)
{
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
  edit_element_widget = new EditElementWidget(this);
  edit_element_widget->hide();
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
      "ref_x_axis_location",
      "ref_y_axis_location",
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
      "flip_col_and_row",
      "grplot",
      "hide",
      "is_major",
      "is_mirrored",
      "keep_aspect_ratio",
      "keep_radii_axes",
      "keep_size_if_swapped",
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
      "trim_col",
      "trim_row",
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

  PdfAct = new QAction(tr("&PDF"), this);
  connect(PdfAct, &QAction::triggered, this, &GRPlotWidget::pdf);
  PngAct = new QAction(tr("&PNG"), this);
  connect(PngAct, &QAction::triggered, this, &GRPlotWidget::png);
  JpegAct = new QAction(tr("&JPEG"), this);
  connect(JpegAct, &QAction::triggered, this, &GRPlotWidget::jpeg);
  SvgAct = new QAction(tr("&SVG"), this);
  connect(SvgAct, &QAction::triggered, this, &GRPlotWidget::svg);

  if (listen_mode)
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
      if (test_mode)
        {
          test_commands_file = new QFile(test_commands);
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
      if (!grm_interactive_plot_from_file(args_, argc, argv)) exit(0);

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
      lineAct = new QAction(tr("&Line"), this);
      connect(lineAct, &QAction::triggered, this, &GRPlotWidget::line);
      scatterAct = new QAction(tr("&Scatter"), this);
      connect(scatterAct, &QAction::triggered, this, &GRPlotWidget::scatter);
      volumeAct = new QAction(tr("&Volume"), this);
      connect(volumeAct, &QAction::triggered, this, &GRPlotWidget::volume);
      isosurfaceAct = new QAction(tr("&Isosurface"), this);
      connect(isosurfaceAct, &QAction::triggered, this, &GRPlotWidget::isosurface);
      line3Act = new QAction(tr("&Line3"), this);
      connect(line3Act, &QAction::triggered, this, &GRPlotWidget::line3);
      trisurfAct = new QAction(tr("&Trisurface"), this);
      connect(trisurfAct, &QAction::triggered, this, &GRPlotWidget::trisurf);
      tricontAct = new QAction(tr("&Tricontour"), this);
      connect(tricontAct, &QAction::triggered, this, &GRPlotWidget::tricont);
      scatter3Act = new QAction(tr("&Scatter3"), this);
      connect(scatter3Act, &QAction::triggered, this, &GRPlotWidget::scatter3);
      histogramAct = new QAction(tr("&Histogram"), this);
      connect(histogramAct, &QAction::triggered, this, &GRPlotWidget::histogram);
      barplotAct = new QAction(tr("&Barplot"), this);
      connect(barplotAct, &QAction::triggered, this, &GRPlotWidget::barplot);
      stairsAct = new QAction(tr("&Stairs"), this);
      connect(stairsAct, &QAction::triggered, this, &GRPlotWidget::stairs);
      stemAct = new QAction(tr("&Stem"), this);
      connect(stemAct, &QAction::triggered, this, &GRPlotWidget::stem);
      shadeAct = new QAction(tr("&Shade"), this);
      connect(shadeAct, &QAction::triggered, this, &GRPlotWidget::shade);
      hexbinAct = new QAction(tr("&Hexbin"), this);
      connect(hexbinAct, &QAction::triggered, this, &GRPlotWidget::hexbin);
      polarLineAct = new QAction(tr("&Polar Line"), this);
      connect(polarLineAct, &QAction::triggered, this, &GRPlotWidget::polar_line);
      polarScatterAct = new QAction(tr("&Polar Scatter"), this);
      connect(polarScatterAct, &QAction::triggered, this, &GRPlotWidget::polar_scatter);

      hidePlotTypeMenuElements();
      moveableModeAct = new QAction(tr("&Disable movable transformation"), this);
      connect(moveableModeAct, &QAction::triggered, this, &GRPlotWidget::moveableMode);

      hide_algo_menu_act = new QAction(this);
      show_algo_menu_act = new QAction(this);
      hide_marginal_sub_menu_act = new QAction(this);
      show_marginal_sub_menu_act = new QAction(this);
      hide_configuration_menu_act = new QAction(this);
      show_configuration_menu_act = new QAction(this);
      add_seperator_act = new QAction(this);
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

      editor_action = new QAction(tr("&Enable Editorview"));
      editor_action->setCheckable(true);
      QObject::connect(editor_action, SIGNAL(triggered()), this, SLOT(enable_editor_functions()));

      save_file_action = new QAction("&Save");
      save_file_action->setShortcut(Qt::CTRL | Qt::Key_S);
      QObject::connect(save_file_action, SIGNAL(triggered()), this, SLOT(save_file_slot()));

      load_file_action = new QAction("&Load");
      load_file_action->setShortcut(Qt::CTRL | Qt::Key_O);
      QObject::connect(load_file_action, SIGNAL(triggered()), this, SLOT(load_file_slot()));

      show_container_action = new QAction(tr("&GRM Container"));
      show_container_action->setCheckable(true);
      show_container_action->setShortcut(Qt::CTRL | Qt::Key_C);
      show_container_action->setVisible(false);
      QObject::connect(show_container_action, SIGNAL(triggered()), this, SLOT(show_container_slot()));

      show_bounding_boxes_action = new QAction(tr("&Bounding Boxes"));
      show_bounding_boxes_action->setCheckable(true);
      show_bounding_boxes_action->setShortcut(Qt::CTRL | Qt::Key_B);
      show_bounding_boxes_action->setVisible(false);
      QObject::connect(show_bounding_boxes_action, SIGNAL(triggered()), this, SLOT(show_bounding_boxes_slot()));

      add_element_action = new QAction("&Add Element");
      add_element_action->setCheckable(true);
      add_element_action->setShortcut(Qt::CTRL | Qt::Key_Plus);
      QObject::connect(add_element_action, SIGNAL(triggered()), this, SLOT(add_element_slot()));
      add_element_action->setVisible(false);

      show_context_action = new QAction(tr("&Display Data-Context"));
      show_context_action->setCheckable(true);
      QObject::connect(show_context_action, SIGNAL(triggered()), this, SLOT(showContextSlot()));
      add_context_action = new QAction(tr("&Column files"));
      QObject::connect(add_context_action, SIGNAL(triggered()), this, SLOT(addContextSlot()));
      add_grplot_data_context = new QAction(tr("&Interpret matrix as 1 column data"));
      QObject::connect(add_grplot_data_context, SIGNAL(triggered()), this, SLOT(addGRPlotDataContextSlot()));
      generate_linear_context_action = new QAction(tr("&Generate linear Data-Context"));
      QObject::connect(generate_linear_context_action, SIGNAL(triggered()), this, SLOT(generateLinearContextSlot()));
    }
}

GRPlotWidget::GRPlotWidget(QMainWindow *parent, grm_args_t *args)
    : QWidget(parent), pixmap(), redraw_pixmap(RedrawType::none), args_(nullptr), rubberBand(nullptr)
{
  args_ = args;

  enable_editor = false;
  highlightBoundingObjects = false;
  bounding_logic = new Bounding_logic();
  current_selection = nullptr;
  amount_scrolled = 0;
  selected_parent = nullptr;
  treewidget = nullptr;
  table_widget = nullptr;
  add_element_widget = nullptr;
  edit_element_widget = nullptr;
  csr = new QCursor(Qt::ArrowCursor);
  setCursor(*csr);

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

  PdfAct = new QAction(tr("&PDF"), this);
  connect(PdfAct, &QAction::triggered, this, &GRPlotWidget::pdf);
  PngAct = new QAction(tr("&PNG"), this);
  connect(PngAct, &QAction::triggered, this, &GRPlotWidget::png);
  JpegAct = new QAction(tr("&JPEG"), this);
  connect(JpegAct, &QAction::triggered, this, &GRPlotWidget::jpeg);
  SvgAct = new QAction(tr("&SVG"), this);
  connect(SvgAct, &QAction::triggered, this, &GRPlotWidget::svg);
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
      x_axis_location_list, y_axis_location_list, marker_type_list, text_align_horizontal_list,
      text_align_vertical_list, algorithm_volume_list, model_list, context_attr_list;
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
  auto x_axis_location_vec = getXAxisLocations();
  x_axis_location_list.reserve((int)x_axis_location_vec.size());
  for (auto &i : x_axis_location_vec)
    {
      x_axis_location_list.push_back(i.c_str());
    }
  auto y_axis_location_vec = getYAxisLocations();
  y_axis_location_list.reserve((int)y_axis_location_vec.size());
  for (auto &i : y_axis_location_vec)
    {
      y_axis_location_list.push_back(i.c_str());
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
      {"ref_x_axis_location", x_axis_location_list},
      {"ref_y_axis_location", y_axis_location_list},
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
      QStringList line3_group = {"line3", "scatter", "scatter3", "tricontour", "trisurface"};
      QStringList barplot_group = {"barplot", "histogram", "stem", "stairs"};
      QStringList hexbin_group = {"hexbin", "shade"};
      QStringList polar_line_group = {"polar_line", "polar_scatter"};
      QStringList other_kinds = {"pie", "polar_heatmap", "polar_histogram", "quiver"};
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
      else if (line3_group.contains(kind.c_str()))
        {
          list = line3_group;
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
      else if (kind == "polar_line" || kind == "polar_scatter")
        {
          list = polar_line_group;
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
           (cur_elem_name == "side_region" || cur_elem_name == "side_plot_region" || cur_elem_name == "text_region") &&
           cur_elem_name != "axis")
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
           !(cur_elem_name == "side_region" || cur_elem_name == "side_plot_region" || cur_elem_name == "text_region") &&
           cur_elem_name != "axis")
    {
      for (const auto &elem : location_list)
        {
          ((QComboBox *)*lineEdit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(location_list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*lineEdit)->setCompleter(completer);
    }
  else if (cur_attr_name == "location" &&
           !(cur_elem_name == "side_region" || cur_elem_name == "side_plot_region" || cur_elem_name == "text_region") &&
           cur_elem_name == "axis")
    {
      auto axis_type = static_cast<std::string>(current_selection->get_ref()->getAttribute("axis_type"));
      for (const auto &elem : (axis_type == "x" ? x_axis_location_list : y_axis_location_list))
        {
          ((QComboBox *)*lineEdit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter((axis_type == "x" ? x_axis_location_list : y_axis_location_list), this);
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
           current_selection->get_ref()->localName() != "axis" &&
           current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      current_text = locationIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "location" &&
           !(current_selection->get_ref()->localName() == "side_region" ||
             current_selection->get_ref()->localName() == "side_plot_region" ||
             current_selection->get_ref()->localName() == "text_region") &&
           current_selection->get_ref()->localName() == "axis" &&
           current_selection->get_ref()->getAttribute(cur_attr_name).isInt())
    {
      auto type = static_cast<std::string>(current_selection->get_ref()->getAttribute("axis_type"));
      if (type == "x")
        current_text =
            xAxisLocationIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
      else
        current_text =
            yAxisLocationIntToString(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
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
      else if (label == "location" && element->localName() != "axis")
        {
          element->setAttribute(label, locationStringToInt(value));
        }
      else if (label == "location" && element->localName() == "axis")
        {
          auto type = static_cast<std::string>(current_selection->get_ref()->getAttribute("axis_type"));
          if (type == "x")
            element->setAttribute(label, xAxisLocationStringToInt(value));
          else
            element->setAttribute(label, yAxisLocationStringToInt(value));
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
  edit_element_widget->show();
  edit_element_widget->AttributeEditEvent();
}

void GRPlotWidget::draw()
{
  static bool called_at_least_once = false;
  if (!file_export.empty())
    {
      std::string kind;
      static char file[50];

      auto global_root = grm_get_document_root();
      auto plot_elem = global_root->querySelectorsAll("plot");
      if (plot_elem.size() > 1)
        kind = "multiplot";
      else
        kind = static_cast<std::string>(plot_elem[0]->getAttribute("_kind"));
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

void GRPlotWidget::redraw(bool full_redraw, bool update_tree)
{
  redraw_pixmap = full_redraw ? RedrawType::full : RedrawType::partial;
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
      if (table_widget != nullptr && !table_widget->isVisible() && show_context_action->isChecked())
        show_context_action->setChecked(false);
      if (treewidget != nullptr && !treewidget->isVisible() && show_container_action->isChecked())
        show_container_action->setChecked(false);
      if (add_element_widget != nullptr && !add_element_widget->isVisible() && add_element_action->isChecked())
        add_element_action->setChecked(false);
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
      redraw_pixmap = RedrawType::full;
    }

  if (redraw_pixmap == RedrawType::partial || redraw_pixmap == RedrawType::full)
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
      auto global_root = grm_get_document_root();
      auto active_figure = global_root->querySelectors("figure[active=\"1\"]");
      auto is_multiplot = active_figure != nullptr && active_figure->querySelectors("layout_grid") != nullptr;
      if (is_multiplot)
        {
          if (redraw_pixmap == RedrawType::partial)
            {
              // A partial redraw was requested, but if no plot is marked as active, we need to redraw the whole figure
              auto active_figure = global_root->querySelectors("figure[active=\"1\"]");
              if ((active_figure->querySelectors("layout_grid") == nullptr ||
                   active_figure->querySelectors("plot[_active=\"1\"]") == nullptr) &&
                  (active_figure->querySelectors("plot[_active_through_update=\"1\"]") == nullptr))
                redraw_pixmap = RedrawType::full;
            }
          if (redraw_pixmap == RedrawType::full)
            {
              auto render = grm_get_render();
              bool auto_update;
              render->getAutoUpdate(&auto_update);
              render->setAutoUpdate(false);
              for (const auto &plot : active_figure->querySelectorsAll("plot[_active=\"1\"]"))
                {
                  plot->removeAttribute("_active");
                }
              render->setAutoUpdate(auto_update);
              previous_active_plot.reset();
              gr_clearbackground();
            }
          else if (active_plot_changed ||
                   active_figure->querySelectors("plot[_active_through_update=\"1\"]") != nullptr)
            {
              gr_setbackground();
            }
        }
      active_plot_changed = false;
      painter.fillRect(0, 0, width(), height(), QColor("white"));
      draw();

      active_figure = global_root->querySelectors("figure[active=\"1\"]");
      is_multiplot = active_figure != nullptr && active_figure->querySelectors("layout_grid") != nullptr;
      if (is_multiplot && active_figure->querySelectors("plot[_active_through_update=\"1\"]") != nullptr)
        {
          gr_setbackground();
          auto render = grm_get_render();
          bool auto_update;
          render->getAutoUpdate(&auto_update);
          render->setAutoUpdate(false);
          for (const auto &plot : active_figure->querySelectorsAll("plot[_active_through_update=\"1\"]"))
            {
              plot->removeAttribute("_active_through_update");
            }
          render->setAutoUpdate(auto_update);
        }

      painter.end();
      redraw_pixmap = RedrawType::none;

      if (treewidget != nullptr && tree_update) treewidget->updateData(grm_get_document_root());
      if (mouse_move_triggert) collectTooltips();
      emit pixmapRedrawn();
    }

  painter.begin(paint_device);
  painter.drawPixmap(0, 0, pixmap);
  bounding_logic->clear();
  extract_bounding_boxes_from_grm((QPainter &)painter);
  highlight_current_selection((QPainter &)painter);
  // Todo: only trigger this method in non multiplot case where 1 plot has different series when not all elements has to
  // be processed to figure out which kinds are all used
  auto global_root = grm_get_document_root();
  if (!in_listen_mode && global_root->querySelectors("layout_grid") == nullptr)
    adjustPlotTypeMenu(global_root->querySelectors("figure[active=1]")->querySelectors("plot"));
  if (!tooltips.empty() && !enable_editor)
    {
      for (const auto &tooltip : tooltips)
        {
          if (tooltip.x_px() > 0 && tooltip.y_px() > 0)
            {
              QColor background(224, 224, 224, 128);
              QPainterPath triangle;
              std::string info, x_label = tooltip.xlabel();

              if (util::startsWith(x_label, "$") && util::endsWith(x_label, "$")) x_label = "x";

              if (tooltip.holds_alternative<grm_tooltip_info_t>())
                {
                  const auto *single_tooltip = tooltip.get<grm_tooltip_info_t>();
                  std::string y_label = single_tooltip->ylabel;

                  if (util::startsWith(y_label, "$") && util::endsWith(y_label, "$")) y_label = "y";
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
                      if (util::startsWith(y_label, "$") && util::endsWith(y_label, "$")) y_label = "y";
                      info_parts.emplace_back("<br>\n");
                      info_parts.push_back(util::string_format(accumulatedTooltipTemplate, y_label.c_str(), y));
                    }
                  std::ostringstream info_stream;
                  std::copy(info_parts.begin(), info_parts.end(), std::ostream_iterator<std::string>(info_stream));
                  info = info_stream.str();
                }
              label.setDefaultStyleSheet(QString::fromStdString(tooltipStyle));
              label.setHtml(QString::fromStdString(info));

              /* tooltips for filled plot kinds gets a higher alpha value to make the values more visible */
              int width = this->size().width(), height = this->size().height();
              GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
              auto max_width_height = std::max(width, height);
              auto x = (double)tooltip.x_px() / max_width_height;
              auto y = (double)(height - tooltip.y_px()) / max_width_height;
              auto plot_element = get_subplot_from_ndc_points_using_dom(1, &x, &y);
              if (plot_element != nullptr && plot_element->hasAttribute("_kind"))
                kind = static_cast<std::string>(plot_element->getAttribute("_kind"));
              if (kind == "heatmap" || kind == "marginal_heatmap" || kind == "contourf" || kind == "imshow")
                background.setAlpha(224);

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
  mouse_move_triggert = true;
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
      int mouse_x, mouse_y;
      double x, y;
      int width = this->size().width(), height = this->size().height();
      getMousePos(event, &mouse_x, &mouse_y);

      GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
      auto max_width_height = std::max(width, height);
      x = (double)mouse_x / max_width_height;
      y = (double)(height - mouse_y) / max_width_height;

      auto global_root = grm_get_document_root();
      auto plot_element = get_subplot_from_ndc_points_using_dom(1, &x, &y);
      if (plot_element && global_root->querySelectors("figure[active=\"1\"]")->querySelectors("layout_grid"))
        {
          auto render = grm_get_render();
          bool auto_update;
          render->getAutoUpdate(&auto_update);
          render->setAutoUpdate(false);
          auto previous_active_plot_locked = previous_active_plot.lock();
          if (previous_active_plot_locked == nullptr || previous_active_plot_locked != plot_element)
            {
              plot_element->setAttribute("_active", true);
              active_plot_changed = true;
            }
          if (previous_active_plot_locked != nullptr && previous_active_plot_locked != plot_element)
            {
              previous_active_plot_locked->removeAttribute("_active");
            }
          previous_active_plot = plot_element;
          render->setAutoUpdate(auto_update);
        }
      if (mouseState.mode == MouseState::Mode::boxzoom)
        {
          rubberBand->setGeometry(QRect(mouseState.pressed, event->pos()).normalized());
        }
      else if (mouseState.mode == MouseState::Mode::pan)
        {
          grm_args_t *args = grm_args_new();

          grm_args_push(args, "x", "i", mouseState.anchor.x());
          grm_args_push(args, "y", "i", mouseState.anchor.y());
          grm_args_push(args, "x_shift", "i", mouse_x - mouseState.anchor.x());
          grm_args_push(args, "y_shift", "i", mouse_y - mouseState.anchor.y());

          grm_input(args);
          grm_args_delete(args);

          mouseState.anchor = event->pos();
          redraw();
        }
      else if (mouseState.mode == MouseState::Mode::movable_xform)
        {
          grm_args_t *args = grm_args_new();

          grm_args_push(args, "x", "i", mouseState.anchor.x());
          grm_args_push(args, "y", "i", mouseState.anchor.y());
          grm_args_push(args, "x_shift", "i", mouse_x - mouseState.anchor.x());
          grm_args_push(args, "y_shift", "i", mouse_y - mouseState.anchor.y());
          if (disable_movable_xform) grm_args_push(args, "disable_movable_trans", "i", disable_movable_xform);

          /* get the correct cursor and sets it */
          int cursor_state = grm_get_hover_mode(mouse_x, mouse_y, disable_movable_xform);
          grm_args_push(args, "movable_state", "i", cursor_state);

          grm_input(args);
          grm_args_delete(args);

          mouseState.anchor = event->pos();
          redraw();
        }
      else
        {
          std::string kind;
          collectTooltips();
          if (plot_element)
            {
              kind = static_cast<std::string>(plot_element->getAttribute("_kind"));
              if (kind == "marginal_heatmap")
                {
                  grm_args_t *input_args;
                  input_args = grm_args_new();

                  grm_args_push(input_args, "x", "i", mouse_x);
                  grm_args_push(input_args, "y", "i", mouse_y);
                  grm_input(input_args);
                  redraw();
                }
            }
          else
            {
              for (const auto &plot_elem : global_root->querySelectorsAll("plot"))
                {
                  kind = static_cast<std::string>(plot_elem->getAttribute("_kind"));
                  if (kind == "marginal_heatmap")
                    {
                      grm_args_t *input_args;
                      input_args = grm_args_new();

                      grm_args_push(input_args, "x", "i", mouse_x);
                      grm_args_push(input_args, "y", "i", mouse_y);
                      grm_input(input_args);
                      redraw();
                    }
                }
            }

          cursorHandler(x, y); // get the correct cursor and sets it
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
          clicked = cur_clicked;
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
          auto global_root = grm_get_document_root();
          // mark the last clicked plot to adjust the plot type menu
          if (global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid"))
            {
              int width, height;
              GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
              auto max_width_height = std::max(width, height);
              auto ndc_x = (double)x / max_width_height;
              auto ndc_y = (double)(height - y) / max_width_height;
              auto plot_element = get_subplot_from_ndc_points_using_dom(1, &ndc_x, &ndc_y);
              if (plot_element != nullptr)
                {
                  for (const auto &selected : global_root->querySelectorsAll("[_selected_for_menu]"))
                    {
                      selected->removeAttribute("_selected_for_menu");
                    }
                  plot_element->setAttribute("_selected_for_menu", true);
                  adjustPlotTypeMenu(plot_element);
                }
            }

          cursorHandler(x, y);
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

      cursorHandler(x, y);
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
  auto global_root = grm_get_document_root();
  auto figure = global_root->querySelectors("figure[active=1]");
  if (figure != nullptr)
    {
      figure->setAttribute("size_x", (double)event->size().width());
      figure->setAttribute("size_y", (double)event->size().height());
    }
  else
    {
      grm_args_push(args_, "size", "dd", (double)event->size().width(), (double)event->size().height());
      grm_merge(args_);
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
  hide_algo_menu_act->trigger();
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface",  "series_wireframe",
                                                 "series_contour",        "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "heatmap");
        }
    }
  redraw();
}

void GRPlotWidget::marginalheatmapall()
{
  show_algo_menu_act->trigger();
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"series_heatmap", "series_surface",  "series_wireframe",
                                                 "series_contour", "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          if (series_elem->parentElement()->parentElement()->localName() != "marginal_heatmap_plot")
            series_elem->setAttribute("kind", "marginal_heatmap");
        }
    }
  for (const auto &series_elem : plot_elem->querySelectorsAll("marginal_heatmap_plot"))
    {
      series_elem->setAttribute("marginal_heatmap_kind", "all");
    }
  redraw();
}

void GRPlotWidget::marginalheatmapline()
{
  show_algo_menu_act->trigger();
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"series_heatmap", "series_surface",  "series_wireframe",
                                                 "series_contour", "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          if (series_elem->parentElement()->parentElement()->localName() != "marginal_heatmap_plot")
            series_elem->setAttribute("kind", "marginal_heatmap");
        }
    }
  for (const auto &series_elem : plot_elem->querySelectorsAll("marginal_heatmap_plot"))
    {
      series_elem->setAttribute("marginal_heatmap_kind", "line");
    }
  redraw();
}

void GRPlotWidget::line()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  for (const auto &elem : plot_elem->querySelectorsAll("series_scatter"))
    {
      elem->setAttribute("kind", "line");
    }

  // to get the same lines then before all lines have to exist during render call so that the linespec work properly
  for (const auto &elem : plot_elem->querySelectorsAll("series_line"))
    {
      elem->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::sumalgorithm()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  for (const auto &elem : plot_elem->querySelectorsAll("marginal_heatmap_plot"))
    {
      elem->setAttribute("algorithm", "sum");
    }
  redraw();
}

void GRPlotWidget::maxalgorithm()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  for (const auto &elem : plot_elem->querySelectorsAll("marginal_heatmap_plot"))
    {
      elem->setAttribute("algorithm", "max");
    }
  redraw();
}

void GRPlotWidget::volume()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  for (const auto &elem : plot_elem->querySelectorsAll("series_isosurface"))
    {
      elem->setAttribute("kind", "volume");
    }
  redraw();
}
void GRPlotWidget::isosurface()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  for (const auto &elem : plot_elem->querySelectorsAll("series_volume"))
    {
      elem->setAttribute("kind", "isosurface");
    }
  redraw();
}

void GRPlotWidget::surface()
{
  hide_algo_menu_act->trigger();
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_heatmap",  "series_wireframe",
                                                 "series_contour",        "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "surface");
        }
    }
  redraw();
}
void GRPlotWidget::wireframe()
{
  hide_algo_menu_act->trigger();
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface",  "series_heatmap",
                                                 "series_contour",        "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "wireframe");
        }
    }
  redraw();
}

void GRPlotWidget::contour()
{
  hide_algo_menu_act->trigger();
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface",  "series_wireframe",
                                                 "series_heatmap",        "series_contourf", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "contour");
        }
    }
  redraw();
}

void GRPlotWidget::imshow()
{
  hide_algo_menu_act->trigger();
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface",  "series_wireframe",
                                                 "series_contour",        "series_contourf", "series_heatmap"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "imshow");
        }
    }
  redraw();
}

void GRPlotWidget::line3()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"series_scatter3", "series_tricontour", "series_trisurface",
                                                 "series_scatter"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "line3");
        }
    }
  redraw();
}

void GRPlotWidget::contourf()
{
  hide_algo_menu_act->trigger();
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface", "series_wireframe",
                                                 "series_contour",        "series_heatmap", "series_imshow"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "contourf");
        }
    }
  redraw();
}

void GRPlotWidget::trisurf()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"series_scatter3", "series_tricontour", "series_line3",
                                                 "series_scatter"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "trisurface");
        }
    }
  redraw();
}

void GRPlotWidget::tricont()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"series_scatter3", "series_line3", "series_trisurface",
                                                 "series_scatter"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "tricontour");
        }
    }
  redraw();
}

void GRPlotWidget::scatter3()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"series_line3", "series_tricontour", "series_trisurface",
                                                 "series_scatter"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "scatter3");
        }
    }
  redraw();
}

void GRPlotWidget::scatter()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  for (const auto &elem : plot_elem->querySelectorsAll("series_line"))
    {
      elem->setAttribute("kind", "scatter");
    }
  redraw();
}

void GRPlotWidget::histogram()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"series_barplot", "series_stem", "series_stairs"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "histogram");
        }
    }

  // to get the same bars then before all bars have to exist during render call so that the linespec work properly
  for (const auto &elem : plot_elem->querySelectorsAll("series_histogram"))
    {
      elem->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::barplot()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"series_histogram", "series_stem", "series_stairs"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "barplot");
        }
    }

  // to get the same bars then before all bars have to exist during render call so that the linespec work properly
  for (const auto &elem : plot_elem->querySelectorsAll("series_barplot"))
    {
      elem->removeAttribute("fill_color_ind");
      elem->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::stairs()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"series_barplot", "series_stem", "series_histogram"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "stairs");
        }
    }

  // to get the same lines then before all lines have to exist during render call so that the linespec work properly
  for (const auto &elem : plot_elem->querySelectorsAll("series_stairs"))
    {
      elem->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::stem()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  std::vector<std::string> valid_series_names = {"series_barplot", "series_histogram", "series_stairs"};
  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "stem");
        }
    }
  redraw();
}

void GRPlotWidget::shade()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  for (const auto &elem : plot_elem->querySelectorsAll("series_hexbin"))
    {
      elem->setAttribute("kind", "shade");
    }
  redraw();
}

void GRPlotWidget::hexbin()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  for (const auto &elem : plot_elem->querySelectorsAll("series_shade"))
    {
      elem->setAttribute("kind", "hexbin");
    }
  redraw();
}

void GRPlotWidget::polar_line()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  for (const auto &elem : plot_elem->querySelectorsAll("series_polar_scatter"))
    {
      elem->setAttribute("kind", "polar_line");
    }
  redraw();
}

void GRPlotWidget::polar_scatter()
{
  auto global_root = grm_get_document_root();
  auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                            : global_root->querySelectors("figure[active=1]");
  for (const auto &elem : plot_elem->querySelectorsAll("series_polar_line"))
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
  auto global_root = grm_get_document_root();
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
              if (getenv("GRM_DEBUG")) qDebug() << "skipping" << cur_child->localName().c_str();
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
      if (!referenced_elements.empty())
        {
          for (const auto &elem : referenced_elements)
            {
              auto rect = elem.boundingRect();
              if (elem.get_ref() != nullptr)
                {
                  auto bbox_xmin = static_cast<double>(elem.get_ref()->getAttribute("_bbox_x_min"));
                  auto bbox_xmax = static_cast<double>(elem.get_ref()->getAttribute("_bbox_x_max"));
                  auto bbox_ymin = static_cast<double>(elem.get_ref()->getAttribute("_bbox_y_min"));
                  auto bbox_ymax = static_cast<double>(elem.get_ref()->getAttribute("_bbox_y_max"));
                  rect = QRectF(bbox_xmin, bbox_ymin, bbox_xmax - bbox_xmin, bbox_ymax - bbox_ymin);
                  painter.drawText(rect.topLeft() + QPointF(5, 10), elem.get_ref()->localName().c_str());
                }
              painter.fillRect(rect, QBrush(QColor(243, 224, 59, 40), Qt::SolidPattern));
            }
        }
    }
  else
    {
      auto global_root = grm_get_document_root();
      auto plot_elem = global_root->querySelectors("[_selected_for_menu=1]");
      if (plot_elem != nullptr)
        {
          int width, height;
          double mwidth, mheight;
          double vp_x_min, vp_x_max, vp_y_min, vp_y_max;
          GRM::Render::getFigureSize(&width, &height, &mwidth, &mheight);
          auto aspect_r = mwidth / mheight;

          if (!GRM::Render::getViewport(plot_elem, &vp_x_min, &vp_x_max, &vp_y_min, &vp_y_max))
            throw NotFoundError(plot_elem->localName() + " doesn't have a viewport but it should.\n");
          vp_x_min *= width;
          vp_x_max *= width;
          vp_y_min *= aspect_r * height;
          vp_y_max *= aspect_r * height;

          painter.drawRect(vp_x_min, std::max(0.0, height - vp_y_max), abs(vp_x_max - vp_x_min),
                           abs(vp_y_max - vp_y_min));
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
  redraw_pixmap = RedrawType::full;
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
      show_configuration_menu_act->trigger();
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
      hide_configuration_menu_act->trigger();
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
              auto global_root = grm_get_document_root();
              auto series_elements = global_root->querySelectorsAll(element_name);
              for (const auto &elem : series_elements)
                {
                  elem->setAttribute(words[n - 2].toUtf8().constData(), words[n - 1].toUtf8().constData());
                }
              auto value = words[n - 1].toUtf8().constData();
              if (strcmp(value, "hist") == 0) value = "histogram";
              if (strcmp(value, "plot3") == 0) value = "line3";

              if (strcmp(value, "line") == 0)
                {
                  // to get the same lines then before all lines have to exist during render call so that the linespec
                  // work properly
                  for (const auto &elem : global_root->querySelectorsAll("series_line"))
                    {
                      elem->setAttribute("_update_required", true);
                    }
                }
              if (strcmp(value, "barplot") == 0 || strcmp(value, "histogram") == 0 || strcmp(value, "stairs") == 0)
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
                  this->resize(width, height);
                  this->setMinimumSize(width, height);
                  window()->adjustSize();
                  this->setMinimumSize(0, 0);
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

Bounding_object **GRPlotWidget::get_current_selection()
{
  return &(this->current_selection);
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

void GRPlotWidget::setTreeUpdate(bool status)
{
  this->tree_update = status;
}

void GRPlotWidget::editElementAccepted()
{
  current_selection = nullptr;
  mouse_move_selection = nullptr;
  amount_scrolled = 0;
  clicked.clear();

  current_selection = nullptr;
  for (const auto &selection : current_selections)
    {
      selection->get_ref()->setAttribute("_selected", 0);
    }
  current_selections.clear();
  redraw();
}

void GRPlotWidget::set_referenced_elements(std::vector<Bounding_object> referenced_elements)
{
  this->referenced_elements = referenced_elements;
}

void GRPlotWidget::adjustPlotTypeMenu(std::shared_ptr<GRM::Element> plot_parent)
{
  bool error = false;
  auto central_region = plot_parent->querySelectors("central_region");
  if (central_region == nullptr) return;

  // hide all menu elements
  if (treewidget != nullptr) // dummy elem which only exist in default grplot case
    {
      std::vector<std::string> valid_series_names = {
          "marginal_heatmap_plot",
          "series_barplot",
          "series_contour",
          "series_contourf",
          "series_heatmap",
          "series_hexbin",
          "series_histogram",
          "series_imshow",
          "series_isosurface",
          "series_line",
          "series_nonuniform_heatmap",
          "series_nonuniform_polar_heatmap",
          "series_pie",
          "series_line3",
          "series_polar_heatmap",
          "series_polar_histogram",
          "series_polar_line",
          "series_polar_scatter",
          "series_quiver",
          "series_scatter",
          "series_scatter3",
          "series_shade",
          "series_stairs",
          "series_stem",
          "series_surface",
          "series_tricontour",
          "series_trisurface",
          "series_volume",
          "series_wireframe",
      };

      hidePlotTypeMenuElements();
      hide_marginal_sub_menu_act->trigger();
      hide_algo_menu_act->trigger();

      if (grm_args_contains(args_, "error"))
        {
          error = true;
          fprintf(stderr, "Plot types are not compatible with error-bars. The menu got disabled\n");
        }

      for (const auto &name : valid_series_names)
        {
          std::vector<std::shared_ptr<GRM::Element>> series_elements;
          if (name == "marginal_heatmap_plot")
            series_elements = plot_parent->querySelectorsAll(name);
          else
            series_elements = central_region->querySelectorsAll(name);
          for (const auto &series : series_elements)
            {
              int z, z_length;
              auto kind = static_cast<std::string>(series->getAttribute("kind"));

              if (kind == "contour" || kind == "heatmap" || kind == "imshow" || kind == "marginal_heatmap" ||
                  kind == "surface" || kind == "wireframe" || kind == "contourf")
                {
                  if (kind == "marginal_heatmap") show_algo_menu_act->trigger();

                  heatmapAct->setVisible(true);
                  surfaceAct->setVisible(true);
                  wireframeAct->setVisible(true);
                  contourAct->setVisible(true);
                  contourfAct->setVisible(true);
                  imshowAct->setVisible(true);
                  show_marginal_sub_menu_act->trigger();
                }
              else if (kind == "line" || (kind == "scatter" && !grm_args_values(args_, "z", "D", &z, &z_length)))
                {
                  lineAct->setVisible(true);
                  scatterAct->setVisible(true);
                }
              else if (kind == "volume" || kind == "isosurface")
                {
                  volumeAct->setVisible(true);
                  isosurfaceAct->setVisible(true);
                }
              else if (kind == "line3" || kind == "trisurface" || kind == "tricontour" || kind == "scatter3" ||
                       kind == "scatter")
                {
                  line3Act->setVisible(true);
                  trisurfAct->setVisible(true);
                  tricontAct->setVisible(true);
                  scatter3Act->setVisible(true);
                  scatterAct->setVisible(true);
                }
              else if ((kind == "histogram" || kind == "barplot" || kind == "stairs" || kind == "stem") && !error)
                {
                  histogramAct->setVisible(true);
                  barplotAct->setVisible(true);
                  stairsAct->setVisible(true);
                  stemAct->setVisible(true);
                }
              else if (kind == "shade" || kind == "hexbin")
                {
                  shadeAct->setVisible(true);
                  hexbinAct->setVisible(true);
                }
              else if (kind == "polar_line" || kind == "polar_scatter")
                {
                  polarLineAct->setVisible(true);
                  polarScatterAct->setVisible(true);
                }
              add_seperator_act->trigger();
            }
        }
    }
}

void GRPlotWidget::hidePlotTypeMenuElements()
{
  heatmapAct->setVisible(false);
  surfaceAct->setVisible(false);
  wireframeAct->setVisible(false);
  contourAct->setVisible(false);
  contourfAct->setVisible(false);
  imshowAct->setVisible(false);
  lineAct->setVisible(false);
  scatterAct->setVisible(false);
  volumeAct->setVisible(false);
  isosurfaceAct->setVisible(false);
  line3Act->setVisible(false);
  trisurfAct->setVisible(false);
  tricontAct->setVisible(false);
  scatter3Act->setVisible(false);
  histogramAct->setVisible(false);
  barplotAct->setVisible(false);
  stairsAct->setVisible(false);
  stemAct->setVisible(false);
  shadeAct->setVisible(false);
  hexbinAct->setVisible(false);
  polarLineAct->setVisible(false);
  polarScatterAct->setVisible(false);
}

QAction *GRPlotWidget::getPdfAct()
{
  return PdfAct;
}

QAction *GRPlotWidget::getPngAct()
{
  return PngAct;
}

QAction *GRPlotWidget::getJpegAct()
{
  return JpegAct;
}

QAction *GRPlotWidget::getSvgAct()
{
  return SvgAct;
}

QAction *GRPlotWidget::getLine3Act()
{
  return line3Act;
}

QAction *GRPlotWidget::getTrisurfAct()
{
  return trisurfAct;
}

QAction *GRPlotWidget::getTricontAct()
{
  return tricontAct;
}

QAction *GRPlotWidget::getScatter3Act()
{
  return scatter3Act;
}

QAction *GRPlotWidget::getHistogramAct()
{
  return histogramAct;
}

QAction *GRPlotWidget::getBarplotAct()
{
  return barplotAct;
}

QAction *GRPlotWidget::getStairsAct()
{
  return stairsAct;
}

QAction *GRPlotWidget::getStemAct()
{
  return stemAct;
}

QAction *GRPlotWidget::getShadeAct()
{
  return shadeAct;
}

QAction *GRPlotWidget::getHexbinAct()
{
  return hexbinAct;
}

QAction *GRPlotWidget::getPolarLineAct()
{
  return polarLineAct;
}

QAction *GRPlotWidget::getPolarScatterAct()
{
  return polarScatterAct;
}

QAction *GRPlotWidget::getLineAct()
{
  return lineAct;
}

QAction *GRPlotWidget::getScatterAct()
{
  return scatterAct;
}

QAction *GRPlotWidget::getVolumeAct()
{
  return volumeAct;
}

QAction *GRPlotWidget::getIsosurfaceAct()
{
  return isosurfaceAct;
}

QAction *GRPlotWidget::getHeatmapAct()
{
  return heatmapAct;
}

QAction *GRPlotWidget::getSurfaceAct()
{
  return surfaceAct;
}

QAction *GRPlotWidget::getWireframeAct()
{
  return wireframeAct;
}

QAction *GRPlotWidget::getContourAct()
{
  return contourAct;
}

QAction *GRPlotWidget::getImshowAct()
{
  return imshowAct;
}

QAction *GRPlotWidget::getContourfAct()
{
  return contourfAct;
}

QAction *GRPlotWidget::getSumAct()
{
  return sumAct;
}

QAction *GRPlotWidget::getMaxAct()
{
  return maxAct;
}

QAction *GRPlotWidget::getMarginalHeatmapAllAct()
{
  return marginalheatmapAllAct;
}

QAction *GRPlotWidget::getMarginalHeatmapLineAct()
{
  return marginalheatmapLineAct;
}

QAction *GRPlotWidget::getMovableModeAct()
{
  return moveableModeAct;
}

QAction *GRPlotWidget::getEditorAct()
{
  return editor_action;
}

QAction *GRPlotWidget::getSaveFileAct()
{
  return save_file_action;
}

QAction *GRPlotWidget::getLoadFileAct()
{
  return load_file_action;
}

QAction *GRPlotWidget::getShowContainerAct()
{
  return show_container_action;
}

QAction *GRPlotWidget::getShowBoundingBoxesAct()
{
  return show_bounding_boxes_action;
}

QAction *GRPlotWidget::getAddElementAct()
{
  return add_element_action;
}

QAction *GRPlotWidget::getShowContextAct()
{
  return show_context_action;
}

QAction *GRPlotWidget::getAddContextAct()
{
  return add_context_action;
}

QAction *GRPlotWidget::getAddGRplotDataContextAct()
{
  return add_grplot_data_context;
}

QAction *GRPlotWidget::getGenerateLinearContextAct()
{
  return generate_linear_context_action;
}

QTextStream *GRPlotWidget::getTestCommandsStream()
{
  return test_commands_stream;
}

QAction *GRPlotWidget::getHideAlgoMenuAct()
{
  return hide_algo_menu_act;
}

QAction *GRPlotWidget::getShowAlgoMenuAct()
{
  return show_algo_menu_act;
}

QAction *GRPlotWidget::getHideMarginalSubMenuAct()
{
  return hide_marginal_sub_menu_act;
}

QAction *GRPlotWidget::getShowMarginalSubMenuAct()
{
  return show_marginal_sub_menu_act;
}

QAction *GRPlotWidget::getHideConfigurationMenuAct()
{
  return hide_configuration_menu_act;
}

QAction *GRPlotWidget::getShowConfigurationMenuAct()
{
  return show_configuration_menu_act;
}

QAction *GRPlotWidget::getAddSeperatorAct()
{
  return add_seperator_act;
}

void GRPlotWidget::cursorHandler(int x, int y)
{
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
