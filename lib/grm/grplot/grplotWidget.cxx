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
#include <QStyle>
#include <QIcon>

#include "grplotWidget.hxx"

#include <gks.h>

#ifndef GR_UNUSED
#define GR_UNUSED(x) (void)(x)
#endif

QStringList axis_type_list{
    "x",
    "y",
};
QStringList clip_region_list{
    "rectangular",
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
QStringList step_where_list{
    "pre",
    "post",
    "mid",
};

static std::string file_export;
static QFile *test_commands_file = nullptr;
static QTextStream *test_commands_stream = nullptr;
static Qt::KeyboardModifiers modifiers = Qt::NoModifier;
static std::vector<BoundingObject> cur_moved;
static bool disable_movable_xform = false;
static bool ctrl_key_mode = false;
static bool mouse_move_triggert = false;
static std::weak_ptr<GRM::Element> previous_active_plot;
static bool active_plot_changed = false;
static bool draw_called_at_least_once = false;
static std::weak_ptr<GRM::Element> prev_selection;
static const char *grm_tmp_dir = nullptr;
static int history_count = 0;
static int history_forward_count = 0;

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
extern "C" void sizeCallbackWrapper(const grm_event_t *cb)
{
  size_callback(cb);
}

std::function<void(const grm_request_event_t *)> cmd_callback;
extern "C" void cmdCallbackWrapper(const grm_event_t *event)
{
  cmd_callback(reinterpret_cast<const grm_request_event_t *>(event));
}


GRPlotWidget::GRPlotWidget(QMainWindow *parent, int argc, char **argv, bool listen_mode, int listen_port,
                           bool test_mode, QString test_commands)
    : QWidget(parent), pixmap(), redraw_pixmap(RedrawType::NONE), args_(nullptr), rubber_band(nullptr)
{
  args_ = grm_args_new();

  enable_editor = false;
  highlight_bounding_objects = false;
  bounding_logic = new BoundingLogic();
  current_selection = nullptr;
  amount_scrolled = 0;
  tree_widget = new TreeWidget(this);
  tree_widget->hide();
  table_widget = new TableWidget(this);
  table_widget->hide();
  color_picker_rgb = new ColorPickerRGB(this);
  color_picker_rgb->hide();
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
      "fill_int_style",
      "fill_style",
      "font",
      "font_precision",
      "kind",
      "line_type",
      "location",
      "marginal_heatmap_kind",
      "marker_type",
      "model",
      "norm",
      "orientation",
      "ref_x_axis_location",
      "ref_y_axis_location",
      "resample_method",
      "scientific_format",
      "size_x_type",
      "size_y_type",
      "size_x_unit",
      "size_y_unit",
      "step_where",
      "style",
      "text_encoding",
      "text_align_horizontal",
      "text_align_vertical",
      "tick_orientation",
      "transformation",
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
      "colored",
      "colormap_inverted",
      "disable_x_trans",
      "disable_y_trans",
      "draw_grid",
      "flip_col_and_row",
      "fit_parents_height",
      "fit_parents_width",
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
      "only_square_aspect_ratio",
      "polar_with_pan",
      "r_log",
      "set_text_color_for_background",
      "space",
      "stairs",
      "text_is_title",
      "theta_flip",
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
  color_ind_attr = QStringList{
      "border_color_ind", "fill_color_ind", "line_color_ind", "marker_color_ind", "text_color_ind",
  };
  color_rgb_attr = QStringList{"line_color_rgb", "fill_color_rgb"};

  // add context attributes to combobox list
  auto context_attributes = GRM::getContextAttributes();
  for (const auto &attr : context_attributes)
    {
      if (attr == "line_color_rgb" || attr == "fill_color_rgb") continue;
      combo_box_attr.push_back(attr.c_str());
    }

#ifdef _WIN32
  putenv("GKS_WSTYPE=381");
  putenv("GKS_DOUBLE_BUF=True");
#else
  setenv("GKS_WSTYPE", "381", 1);
  setenv("GKS_DOUBLE_BUF", "True", 1);
#endif

  rubber_band = new QRubberBand(QRubberBand::Rectangle, this);
  setFocusPolicy(Qt::FocusPolicy::StrongFocus); // needed to receive key press events
  setMouseTracking(true);
  mouse_state.mode = MouseState::Mode::NORMAL;
  mouse_state.pressed = {0, 0};
  mouse_state.anchor = {0, 0};

  pdf_act = new QAction(tr("&PDF"), this);
  connect(pdf_act, &QAction::triggered, this, &GRPlotWidget::pdf);
  png_act = new QAction(tr("&PNG"), this);
  connect(png_act, &QAction::triggered, this, &GRPlotWidget::png);
  jpeg_act = new QAction(tr("&JPEG"), this);
  connect(jpeg_act, &QAction::triggered, this, &GRPlotWidget::jpeg);
  svg_act = new QAction(tr("&SVG"), this);
  connect(svg_act, &QAction::triggered, this, &GRPlotWidget::svg);

  if (listen_mode)
    {
      in_listen_mode = true;
      qRegisterMetaType<ArgsWrapper>("ArgsWrapper");
      receiver = new Receiver(listen_port);
      QObject::connect(receiver, SIGNAL(resultReady(ArgsWrapper)), this, SLOT(received(ArgsWrapper)),
                       Qt::QueuedConnection);
      QObject::connect(this, SIGNAL(pixmapRedrawn()), receiver, SLOT(dataProcessed()), Qt::QueuedConnection);
      receiver->start();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      ::size_callback = [this](auto &&tmp) { sizeCallback(std::forward<decltype(tmp)>(tmp)); };
      ::cmd_callback = [this](auto &&tmp) { cmdCallback(std::forward<decltype(tmp)>(tmp)); };
#else
      ::size_callback = std::bind(&GRPlotWidget::sizeCallback, this, std::placeholders::_1);
      ::cmd_callback = std::bind(&GRPlotWidget::cmdCallback, this, std::placeholders::_1);
#endif

      grm_register(GRM_EVENT_SIZE, sizeCallbackWrapper);
      grm_register(GRM_EVENT_REQUEST, cmdCallbackWrapper);
      ArgsWrapper configuration;
      configuration.setWrapper(grm_args_new());
      grm_args_push(configuration.getWrapper(), "hold_plots", "i", 0);
      grm_merge(configuration.getWrapper());
      grm_args_delete(configuration.getWrapper());
    }
  else
    {
      grm_args_push(args_, "keep_aspect_ratio", "i", 1);
      if (!grm_interactive_plot_from_file(args_, argc, argv)) exit(0);
      if (test_mode)
        {
          test_commands_file = new QFile(test_commands);
          if (test_commands_file->open(QIODevice::ReadOnly))
            {
              test_commands_stream = new QTextStream(test_commands_file);
            }
          else
            {
              std::cerr << "Unable to open test commands file" << std::endl;
              QApplication::quit();
            }
        }

      heatmap_act = new QAction(tr("&Heatmap"), this);
      connect(heatmap_act, &QAction::triggered, this, &GRPlotWidget::heatmap);
      marginal_heatmap_all_act = new QAction(tr("&Type 1 all"), this);
      connect(marginal_heatmap_all_act, &QAction::triggered, this, &GRPlotWidget::marginalHeatmapAll);
      marginal_heatmap_line_act = new QAction(tr("&Type 2 line"), this);
      connect(marginal_heatmap_line_act, &QAction::triggered, this, &GRPlotWidget::marginalHeatmapLine);
      surface_act = new QAction(tr("&Surface"), this);
      connect(surface_act, &QAction::triggered, this, &GRPlotWidget::surface);
      wireframe_act = new QAction(tr("&Wireframe"), this);
      connect(wireframe_act, &QAction::triggered, this, &GRPlotWidget::wireframe);
      contour_act = new QAction(tr("&Contour"), this);
      connect(contour_act, &QAction::triggered, this, &GRPlotWidget::contour);
      imshow_act = new QAction(tr("&Imshow"), this);
      connect(imshow_act, &QAction::triggered, this, &GRPlotWidget::imshow);
      sum_act = new QAction(tr("&Sum"), this);
      connect(sum_act, &QAction::triggered, this, &GRPlotWidget::sumAlgorithm);
      max_act = new QAction(tr("&Maximum"), this);
      connect(max_act, &QAction::triggered, this, &GRPlotWidget::maxAlgorithm);
      contourf_act = new QAction(tr("&Contourf"), this);
      connect(contourf_act, &QAction::triggered, this, &GRPlotWidget::contourf);
      line_act = new QAction(tr("&Line"), this);
      connect(line_act, &QAction::triggered, this, &GRPlotWidget::line);
      scatter_act = new QAction(tr("&Scatter"), this);
      connect(scatter_act, &QAction::triggered, this, &GRPlotWidget::scatter);
      volume_act = new QAction(tr("&Volume"), this);
      connect(volume_act, &QAction::triggered, this, &GRPlotWidget::volume);
      isosurface_act = new QAction(tr("&Isosurface"), this);
      connect(isosurface_act, &QAction::triggered, this, &GRPlotWidget::isosurface);
      line3_act = new QAction(tr("&Line3"), this);
      connect(line3_act, &QAction::triggered, this, &GRPlotWidget::line3);
      trisurf_act = new QAction(tr("&Trisurface"), this);
      connect(trisurf_act, &QAction::triggered, this, &GRPlotWidget::trisurf);
      tricont_act = new QAction(tr("&Tricontour"), this);
      connect(tricont_act, &QAction::triggered, this, &GRPlotWidget::tricont);
      scatter3_act = new QAction(tr("&Scatter3"), this);
      connect(scatter3_act, &QAction::triggered, this, &GRPlotWidget::scatter3);
      barplot_act = new QAction(tr("&Barplot"), this);
      connect(barplot_act, &QAction::triggered, this, &GRPlotWidget::barplot);
      stairs_act = new QAction(tr("&Stairs"), this);
      connect(stairs_act, &QAction::triggered, this, &GRPlotWidget::stairs);
      stem_act = new QAction(tr("&Stem"), this);
      connect(stem_act, &QAction::triggered, this, &GRPlotWidget::stem);
      shade_act = new QAction(tr("&Shade"), this);
      connect(shade_act, &QAction::triggered, this, &GRPlotWidget::shade);
      hexbin_act = new QAction(tr("&Hexbin"), this);
      connect(hexbin_act, &QAction::triggered, this, &GRPlotWidget::hexbin);
      polar_line_act = new QAction(tr("&Polar Line"), this);
      connect(polar_line_act, &QAction::triggered, this, &GRPlotWidget::polarLine);
      polar_scatter_act = new QAction(tr("&Polar Scatter"), this);
      connect(polar_scatter_act, &QAction::triggered, this, &GRPlotWidget::polarScatter);

      hidePlotTypeMenuElements();
      moveable_mode_act = new QAction(tr("&Disable movable transformation"), this);
      connect(moveable_mode_act, &QAction::triggered, this, &GRPlotWidget::moveableMode);

      x_log_act = new QAction(tr("&X Log"), this);
      connect(x_log_act, &QAction::triggered, this, &GRPlotWidget::xLogSlot);
      y_log_act = new QAction(tr("&Y Log"), this);
      connect(y_log_act, &QAction::triggered, this, &GRPlotWidget::yLogSlot);
      z_log_act = new QAction(tr("&Z Log"), this);
      connect(z_log_act, &QAction::triggered, this, &GRPlotWidget::zLogSlot);
      r_log_act = new QAction(tr("&R Log"), this);
      connect(r_log_act, &QAction::triggered, this, &GRPlotWidget::rLogSlot);
      x_flip_act = new QAction(tr("&X Flip"), this);
      connect(x_flip_act, &QAction::triggered, this, &GRPlotWidget::xFlipSlot);
      y_flip_act = new QAction(tr("&Y Flip"), this);
      connect(y_flip_act, &QAction::triggered, this, &GRPlotWidget::yFlipSlot);
      z_flip_act = new QAction(tr("&Z Flip"), this);
      connect(z_flip_act, &QAction::triggered, this, &GRPlotWidget::zFlipSlot);
      theta_flip_act = new QAction(tr("&Theta Flip"), this);
      connect(theta_flip_act, &QAction::triggered, this, &GRPlotWidget::thetaFlipSlot);

      accelerate_act = new QAction(tr("&Accelerate"), this);
      connect(accelerate_act, &QAction::triggered, this, &GRPlotWidget::accelerateSlot);
      polar_with_pan_act = new QAction(tr("&Polar Pan"), this);
      connect(polar_with_pan_act, &QAction::triggered, this, &GRPlotWidget::polarWithPanSlot);
      keep_window_act = new QAction(tr("&Keep Window"), this);
      connect(keep_window_act, &QAction::triggered, this, &GRPlotWidget::keepWindowSlot);
      colormap_act = new QAction(tr("&Colormap"), this);
      connect(colormap_act, &QAction::triggered, this, &GRPlotWidget::colormapSlot);

      vertical_orientation_act = new QAction(tr("&Vertical"), this);
      connect(vertical_orientation_act, &QAction::triggered, this, &GRPlotWidget::verticalOrientationSlot);
      horizontal_orientation_act = new QAction(tr("&Horizontal"), this);
      connect(horizontal_orientation_act, &QAction::triggered, this, &GRPlotWidget::horizontalOrientationSlot);
      keep_aspect_ratio_act = new QAction(tr("&Keep Aspectratio"), this);
      connect(keep_aspect_ratio_act, &QAction::triggered, this, &GRPlotWidget::keepAspectRatioSlot);
      only_square_aspect_ratio_act = new QAction(tr("&Square Aspect Ratio"), this);
      connect(only_square_aspect_ratio_act, &QAction::triggered, this, &GRPlotWidget::onlySquareAspectRatioSlot);

      legend_act = new QAction(tr("&Legend"), this);
      connect(legend_act, &QAction::triggered, this, &GRPlotWidget::legendSlot);
      colorbar_act = new QAction(tr("&Colorbar"), this);
      connect(colorbar_act, &QAction::triggered, this, &GRPlotWidget::colorbarSlot);
      left_axis_act = new QAction(tr("&Left Axis"), this);
      connect(left_axis_act, &QAction::triggered, this, &GRPlotWidget::leftAxisSlot);
      right_axis_act = new QAction(tr("&Right Axis"), this);
      connect(right_axis_act, &QAction::triggered, this, &GRPlotWidget::rightAxisSlot);
      bottom_axis_act = new QAction(tr("&Bottom Axis"), this);
      connect(bottom_axis_act, &QAction::triggered, this, &GRPlotWidget::bottomAxisSlot);
      top_axis_act = new QAction(tr("&Top Axis"), this);
      connect(top_axis_act, &QAction::triggered, this, &GRPlotWidget::topAxisSlot);
      twin_x_axis_act = new QAction(tr("&Twin-X Axis"), this);
      connect(twin_x_axis_act, &QAction::triggered, this, &GRPlotWidget::twinXAxisSlot);
      twin_y_axis_act = new QAction(tr("&Twin-Y Axis"), this);
      connect(twin_y_axis_act, &QAction::triggered, this, &GRPlotWidget::twinYAxisSlot);

      hide_algo_menu_act = new QAction(this);
      show_algo_menu_act = new QAction(this);
      hide_marginal_sub_menu_act = new QAction(this);
      show_marginal_sub_menu_act = new QAction(this);
      hide_configuration_menu_act = new QAction(this);
      show_configuration_menu_act = new QAction(this);
      hide_orientation_sub_menu_act = new QAction(this);
      show_orientation_sub_menu_act = new QAction(this);
      hide_aspect_ratio_sub_menu_act = new QAction(this);
      show_aspect_ratio_sub_menu_act = new QAction(this);
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
      QObject::connect(editor_action, SIGNAL(triggered()), this, SLOT(enableEditorFunctions()));

      save_file_action = new QAction("&Save");
      save_file_action->setShortcut(Qt::CTRL | Qt::Key_S);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
      save_file_action->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave));
      save_file_action->setIconVisibleInMenu(true);
#endif
      QObject::connect(save_file_action, SIGNAL(triggered()), this, SLOT(saveFileSlot()));

      load_file_action = new QAction("&Load");
      load_file_action->setShortcut(Qt::CTRL | Qt::Key_O);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
      load_file_action->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen));
      load_file_action->setIconVisibleInMenu(true);
#endif
      QObject::connect(load_file_action, SIGNAL(triggered()), this, SLOT(loadFileSlot()));

      show_container_action = new QAction(tr("&GRM Container"));
      show_container_action->setCheckable(true);
      show_container_action->setShortcut(Qt::CTRL | Qt::Key_C);
      show_container_action->setVisible(false);
      QObject::connect(show_container_action, SIGNAL(triggered()), this, SLOT(showContainerSlot()));

      add_element_action = new QAction("&Add Element");
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
      add_element_action->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ListAdd));
      add_element_action->setIconVisibleInMenu(true);
#endif
      add_element_action->setCheckable(true);
      add_element_action->setShortcut(Qt::CTRL | Qt::Key_Plus);
      QObject::connect(add_element_action, SIGNAL(triggered()), this, SLOT(addElementSlot()));
      add_element_action->setVisible(false);

      undo_action = new QAction("&Undo");
      undo_action->setShortcut(Qt::CTRL | Qt::Key_Z);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
      undo_action->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditUndo));
      undo_action->setIconVisibleInMenu(true);
#endif
      QObject::connect(undo_action, SIGNAL(triggered()), this, SLOT(undoSlot()));
      undo_action->setVisible(false);

      redo_action = new QAction("&Redo");
      redo_action->setShortcut(Qt::CTRL | Qt::Key_Y);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
      redo_action->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditRedo));
      redo_action->setIconVisibleInMenu(true);
#endif
      QObject::connect(redo_action, SIGNAL(triggered()), this, SLOT(redoSlot()));
      redo_action->setVisible(false);

      show_context_action = new QAction(tr("&Display Data-Context"));
      show_context_action->setCheckable(true);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
      show_context_action->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DialogInformation));
      show_context_action->setIconVisibleInMenu(true);
#endif
      QObject::connect(show_context_action, SIGNAL(triggered()), this, SLOT(showContextSlot()));
      add_context_action = new QAction(tr("&Column files"));
      QObject::connect(add_context_action, SIGNAL(triggered()), this, SLOT(addContextSlot()));
      add_grplot_data_context = new QAction(tr("&Interpret matrix as 1 column data"));
      QObject::connect(add_grplot_data_context, SIGNAL(triggered()), this, SLOT(addGRPlotDataContextSlot()));
      generate_linear_context_action = new QAction(tr("&Generate linear Data-Context"));
      QObject::connect(generate_linear_context_action, SIGNAL(triggered()), this, SLOT(generateLinearContextSlot()));

      selectable_grid_act = new QAction(tr("&Selectable Grid"));
      QObject::connect(selectable_grid_act, SIGNAL(triggered()), this, SLOT(selectableGridSlot()));
      selectable_grid_act->setCheckable(true);
      selectable_grid_act->setChecked(false);

      hide_location_sub_menu_act = new QAction(this);
      show_location_sub_menu_act = new QAction(this);
    }
}

GRPlotWidget::GRPlotWidget(QMainWindow *parent, grm_args_t *args)
    : QWidget(parent), pixmap(), redraw_pixmap(RedrawType::NONE), args_(nullptr), rubber_band(nullptr)
{
  args_ = args;

  enable_editor = false;
  highlight_bounding_objects = false;
  bounding_logic = new BoundingLogic();
  current_selection = nullptr;
  amount_scrolled = 0;
  selected_parent = nullptr;
  tree_widget = nullptr;
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

  rubber_band = new QRubberBand(QRubberBand::Rectangle, this);
  setFocusPolicy(Qt::FocusPolicy::StrongFocus); // needed to receive key press events
  setMouseTracking(true);
  mouse_state.mode = MouseState::Mode::NORMAL;
  mouse_state.pressed = {0, 0};
  mouse_state.anchor = {0, 0};

  pdf_act = new QAction(tr("&PDF"), this);
  connect(pdf_act, &QAction::triggered, this, &GRPlotWidget::pdf);
  png_act = new QAction(tr("&PNG"), this);
  connect(png_act, &QAction::triggered, this, &GRPlotWidget::png);
  jpeg_act = new QAction(tr("&JPEG"), this);
  connect(jpeg_act, &QAction::triggered, this, &GRPlotWidget::jpeg);
  svg_act = new QAction(tr("&SVG"), this);
  connect(svg_act, &QAction::triggered, this, &GRPlotWidget::svg);
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
                                            QWidget **line_edit)
{
  QStringList size_unit_list, colormap_list, font_list, font_precision_list, line_type_list, location_list,
      x_axis_location_list, y_axis_location_list, marker_type_list, text_align_horizontal_list,
      text_align_vertical_list, algorithm_volume_list, model_list, context_attr_list, fill_style_list,
      fill_int_style_list, transformation_list;
  auto size_unit_vec = GRM::getSizeUnits();
  size_unit_list.reserve((int)size_unit_vec.size());
  for (auto &i : size_unit_vec)
    {
      size_unit_list.push_back(i.c_str());
    }
  auto colormap_vec = GRM::getColormaps();
  colormap_list.reserve((int)colormap_vec.size());
  for (auto &i : colormap_vec)
    {
      colormap_list.push_back(i.c_str());
    }
  auto font_vec = GRM::getFonts();
  font_list.reserve((int)font_vec.size());
  for (auto &i : font_vec)
    {
      font_list.push_back(i.c_str());
    }
  auto font_precision_vec = GRM::getFontPrecisions();
  font_precision_list.reserve((int)font_precision_vec.size());
  for (auto &i : font_precision_vec)
    {
      font_precision_list.push_back(i.c_str());
    }
  auto line_type_vec = GRM::getLineTypes();
  line_type_list.reserve((int)line_type_vec.size());
  for (auto &i : line_type_vec)
    {
      line_type_list.push_back(i.c_str());
    }
  auto location_vec = GRM::getLocations();
  location_list.reserve((int)location_vec.size());
  for (auto &i : location_vec)
    {
      location_list.push_back(i.c_str());
    }
  auto x_axis_location_vec = GRM::getXAxisLocations();
  x_axis_location_list.reserve((int)x_axis_location_vec.size());
  for (auto &i : x_axis_location_vec)
    {
      x_axis_location_list.push_back(i.c_str());
    }
  auto y_axis_location_vec = GRM::getYAxisLocations();
  y_axis_location_list.reserve((int)y_axis_location_vec.size());
  for (auto &i : y_axis_location_vec)
    {
      y_axis_location_list.push_back(i.c_str());
    }
  auto marker_type_vec = GRM::getMarkerTypes();
  marker_type_list.reserve((int)marker_type_vec.size());
  for (auto &i : marker_type_vec)
    {
      marker_type_list.push_back(i.c_str());
    }
  auto text_align_horizontal_vec = GRM::getTextAlignHorizontal();
  text_align_horizontal_list.reserve((int)text_align_horizontal_vec.size());
  for (auto &i : text_align_horizontal_vec)
    {
      text_align_horizontal_list.push_back(i.c_str());
    }
  auto text_align_vertical_vec = GRM::getTextAlignVertical();
  text_align_vertical_list.reserve((int)text_align_vertical_vec.size());
  for (auto &i : text_align_vertical_vec)
    {
      text_align_vertical_list.push_back(i.c_str());
    }
  auto algorithm_volume_vec = GRM::getAlgorithm();
  algorithm_volume_list.reserve((int)algorithm_volume_vec.size());
  for (auto &i : algorithm_volume_vec)
    {
      algorithm_volume_list.push_back(i.c_str());
    }
  auto model_vec = GRM::getModel();
  model_list.reserve((int)model_vec.size());
  for (auto &i : model_vec)
    {
      model_list.push_back(i.c_str());
    }
  auto fill_style_vec = GRM::getFillStyles();
  fill_style_list.reserve((int)fill_style_vec.size());
  for (auto &i : fill_style_vec)
    {
      fill_style_list.push_back(i.c_str());
    }
  auto fill_int_style_vec = GRM::getFillIntStyles();
  fill_int_style_list.reserve((int)fill_int_style_vec.size());
  for (auto &i : fill_int_style_vec)
    {
      fill_int_style_list.push_back(i.c_str());
    }
  auto transformation_vec = GRM::getTransformation();
  transformation_list.reserve((int)transformation_vec.size());
  for (auto &i : transformation_vec)
    {
      transformation_list.push_back(i.c_str());
    }
  table_widget->extractContextNames(grm_get_render()->getContext());
  auto context_attr_vec = table_widget->getContextNames();
  context_attr_list.reserve((int)context_attr_vec.size());
  for (auto &i : context_attr_vec)
    {
      context_attr_list.push_back(i.c_str());
    }

  static std::map<std::string, QStringList> attribute_to_list{
      {"axis_type", axis_type_list},
      {"error_bar_style", error_bar_style_list},
      {"clip_region", clip_region_list},
      {"size_x_unit", size_unit_list},
      {"size_y_unit", size_unit_list},
      {"colormap", colormap_list},
      {"fill_style", fill_style_list},
      {"fill_int_style", fill_int_style_list},
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
      {"ref_x_axis_location", x_axis_location_list},
      {"ref_y_axis_location", y_axis_location_list},
      {"resample_method", resample_method_list},
      {"scientific_format", scientific_format_list},
      {"size_x_type", size_type_list},
      {"size_y_type", size_type_list},
      {"step_where", step_where_list},
      {"style", style_list},
      {"text_encoding", text_encoding_list},
      {"x_org_pos", org_pos_list},
      {"y_org_pos", org_pos_list},
      {"z_org_pos", org_pos_list},
      {"tick_orientation", tick_orientation_list},
      {"transformation", transformation_list},
  };
  // add for all context attributes all possible values
  for (const auto &attr : GRM::getContextAttributes())
    {
      if (attribute_to_list.count(attr) >= 1)
        attribute_to_list[attr] = context_attr_list;
      else
        attribute_to_list.emplace(attr, context_attr_list);
    }

  ((QComboBox *)*line_edit)->setEditable(true);
  if (attribute_to_list.count(cur_attr_name))
    {
      int cnt = 0;
      QStringList list = attribute_to_list[cur_attr_name];
      for (const auto &elem : list)
        {
          ((QComboBox *)*line_edit)->addItem(elem.toStdString().c_str());
          if (cur_attr_name == "colormap")
            {
              const auto global_root = grm_get_document_root();
              const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
              const auto figure_elem =
                  (layout_grid != nullptr && layout_grid->querySelectors("[_selected_for_menu]") != nullptr)
                      ? layout_grid->querySelectors("[_selected_for_menu]")
                      : global_root->querySelectors("figure[active=1]");
              const auto plot_elem = figure_elem->querySelectors("plot");

              auto colormap = QPixmap((":/preview_images/colormaps/" + elem.toStdString() + ".png").c_str());
              auto colormap_inverted = plot_elem->hasAttribute("colormap_inverted") &&
                                       static_cast<int>(plot_elem->getAttribute("colormap_inverted"));
              if (!colormap.isNull())
                {
                  colormap.setDevicePixelRatio(15);
                  if (colormap_inverted) colormap = colormap.transformed(QTransform().rotate(180));
                  ((QComboBox *)*line_edit)->setItemData(cnt++, colormap, Qt::DecorationRole);
                }
            }
          else if (cur_attr_name == "marker_type")
            {
              auto marker_type = QPixmap((":/preview_images/marker_types/" + elem.toStdString() + ".png").c_str());
              if (!marker_type.isNull())
                {
                  marker_type.setDevicePixelRatio(15);
                  ((QComboBox *)*line_edit)->setItemData(cnt++, marker_type, Qt::DecorationRole);
                }
            }
          else if (cur_attr_name == "line_type")
            {
              auto line_type = QPixmap((":/preview_images/line_types/" + elem.toStdString() + ".png").c_str());
              if (!line_type.isNull())
                {
                  line_type.setDevicePixelRatio(15);
                  ((QComboBox *)*line_edit)->setItemData(cnt++, line_type, Qt::DecorationRole);
                }
            }
          else if (cur_attr_name == "fill_style")
            {
              auto fill_style = QPixmap((":/preview_images/fill_styles/" + elem.toStdString() + ".png").c_str());
              if (!fill_style.isNull())
                {
                  fill_style = fill_style.scaled(30, 30, Qt::KeepAspectRatio);
                  ((QComboBox *)*line_edit)->setItemData(cnt++, fill_style, Qt::DecorationRole);
                }
            }
          else if (cur_attr_name == "font_precision")
            {
              auto font_precision =
                  QPixmap((":/preview_images/font_precisions/" + elem.toStdString() + ".png").c_str());
              if (!font_precision.isNull())
                {
                  font_precision.setDevicePixelRatio(15);
                  ((QComboBox *)*line_edit)->setItemData(cnt++, font_precision, Qt::DecorationRole);
                }
            }
          else if (cur_attr_name == "font")
            {
              auto font = QPixmap((":/preview_images/fonts/" + elem.toStdString() + ".png").c_str());
              if (!font.isNull())
                {
                  font.setDevicePixelRatio(15);
                  ((QComboBox *)*line_edit)->setItemData(cnt++, font, Qt::DecorationRole);
                }
            }
        }
      auto *completer = new QCompleter(list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*line_edit)->setCompleter(completer);
    }
  else if (cur_attr_name == "algorithm" && cur_elem_name == "series_volume")
    {
      for (const auto &elem : algorithm_volume_list)
        {
          ((QComboBox *)*line_edit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(algorithm_volume_list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*line_edit)->setCompleter(completer);
    }
  else if (cur_attr_name == "algorithm" && cur_elem_name == "marginal_heatmap_plot")
    {
      for (const auto &elem : algorithm_marginal_heatmap_list)
        {
          ((QComboBox *)*line_edit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(algorithm_marginal_heatmap_list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*line_edit)->setCompleter(completer);
    }
  else if (cur_attr_name == "kind")
    {
      QStringList list;
      QStringList line_group = {"line", "scatter"};
      QStringList heatmap_group = {"contour",          "contourf", "heatmap",  "imshow",
                                   "marginal_heatmap", "surface",  "wireframe"};
      QStringList isosurface_group = {"isosurface", "volume"};
      QStringList line3_group = {"line3", "scatter", "scatter3", "tricontour", "trisurface"};
      QStringList barplot_group = {"barplot", "stem", "stairs"};
      QStringList hexbin_group = {"hexbin", "shade"};
      QStringList polar_line_group = {"polar_line", "polar_scatter"};
      QStringList other_kinds = {"histogram", "pie", "polar_heatmap", "polar_histogram", "quiver"};
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
          ((QComboBox *)*line_edit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*line_edit)->setCompleter(completer);
    }
  else if (cur_attr_name == "location" &&
           (cur_elem_name == "side_region" || cur_elem_name == "side_plot_region" || cur_elem_name == "text_region") &&
           cur_elem_name != "axis")
    {
      for (const auto &elem : side_region_location_list)
        {
          ((QComboBox *)*line_edit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(side_region_location_list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*line_edit)->setCompleter(completer);
    }
  else if (cur_attr_name == "location" &&
           !(cur_elem_name == "side_region" || cur_elem_name == "side_plot_region" || cur_elem_name == "text_region") &&
           cur_elem_name != "axis")
    {
      for (const auto &elem : location_list)
        {
          ((QComboBox *)*line_edit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter(location_list, this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*line_edit)->setCompleter(completer);
    }
  else if (cur_attr_name == "location" &&
           !(cur_elem_name == "side_region" || cur_elem_name == "side_plot_region" || cur_elem_name == "text_region") &&
           cur_elem_name == "axis")
    {
      auto axis_type = static_cast<std::string>(current_selection->getRef()->getAttribute("axis_type"));
      for (const auto &elem : (axis_type == "x" ? x_axis_location_list : y_axis_location_list))
        {
          ((QComboBox *)*line_edit)->addItem(elem.toStdString().c_str());
        }
      auto *completer = new QCompleter((axis_type == "x" ? x_axis_location_list : y_axis_location_list), this);
      completer->setCaseSensitivity(Qt::CaseInsensitive);
      ((QComboBox *)*line_edit)->setCompleter(completer);
    }
}

void GRPlotWidget::advancedAttributeComboBoxHandler(const std::string &cur_attr_name, std::string cur_elem_name,
                                                    QWidget **line_edit)
{
  if (cur_attr_name == "kind")
    cur_elem_name = "series_" + static_cast<std::string>(current_selection->getRef()->getAttribute(cur_attr_name));
  attributeComboBoxHandler(cur_attr_name, cur_elem_name, line_edit);
  ((QComboBox *)*line_edit)->addItem(""); // entry to remove the attribute

  auto current_text = static_cast<std::string>(current_selection->getRef()->getAttribute(cur_attr_name));
  if (cur_attr_name == "text_align_vertical" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::textAlignVerticalIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "text_align_horizontal" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text = GRM::textAlignHorizontalIntToString(
          static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "algorithm" && current_selection->getRef()->localName() == "series_volume" &&
           current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::algorithmIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "model" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text = GRM::modelIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "location" &&
           !(current_selection->getRef()->localName() == "side_region" ||
             current_selection->getRef()->localName() == "side_plot_region" ||
             current_selection->getRef()->localName() == "text_region") &&
           current_selection->getRef()->localName() != "axis" &&
           current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::locationIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "location" &&
           !(current_selection->getRef()->localName() == "side_region" ||
             current_selection->getRef()->localName() == "side_plot_region" ||
             current_selection->getRef()->localName() == "text_region") &&
           current_selection->getRef()->localName() == "axis" &&
           current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      auto type = static_cast<std::string>(current_selection->getRef()->getAttribute("axis_type"));
      if (type == "x")
        current_text =
            GRM::xAxisLocationIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
      else
        current_text =
            GRM::yAxisLocationIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "clip_region" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::clipRegionIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "colormap" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::colormapIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "text_encoding" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::textEncodingIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "marker_type" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::markerTypeIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "font" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text = GRM::fontIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "font_precision" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::fontPrecisionIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "line_type" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::lineTypeIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "resample_method" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::resampleMethodIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "scientific_format" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::scientificFormatIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "tick_orientation" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::tickOrientationIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "error_bar_style" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::errorBarStyleIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "fill_style" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::fillStyleIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "fill_int_style" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::fillIntStyleIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }
  else if (cur_attr_name == "transformation" && current_selection->getRef()->getAttribute(cur_attr_name).isInt())
    {
      current_text =
          GRM::transformationIntToString(static_cast<int>(current_selection->getRef()->getAttribute(cur_attr_name)));
    }

  int index = ((QComboBox *)*line_edit)->findText(current_text.c_str());
  if (index == -1) index += ((QComboBox *)*line_edit)->count();
  ((QComboBox *)*line_edit)->setCurrentIndex(index);
  if (cur_attr_name == "colormap") ((QComboBox *)*line_edit)->setIconSize(QSize(90, 40));
  if (cur_attr_name == "line_type") ((QComboBox *)*line_edit)->setIconSize(QSize(60, 40));
  if (cur_attr_name == "font_precision") ((QComboBox *)*line_edit)->setIconSize(QSize(60, 40));
  if (cur_attr_name == "font") ((QComboBox *)*line_edit)->setIconSize(QSize(60, 40));
}

void GRPlotWidget::attributeSetForComboBox(const std::string &attr_type, std::shared_ptr<GRM::Element> element,
                                           const std::string &value, const std::string &label)
{
  if (attr_type == "xs:string" && !util::isDigits(value))
    {
      auto context_attributes = GRM::getContextAttributes();
      if (label == "algorithm" && element->localName() == "marginal_heatmap_plot")
        {
          if (algorithm_marginal_heatmap_list.contains(QString::fromStdString(value)))
            element->setAttribute(label, value);
          else
            fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
      else if (label == "axis_type")
        {
          if (axis_type_list.contains(QString::fromStdString(value)))
            element->setAttribute(label, value);
          else
            fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
      else if (label == "orientation")
        {
          if (orientation_list.contains(QString::fromStdString(value)))
            element->setAttribute(label, value);
          else
            fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
      else if (label == "marginal_heatmap_kind")
        {
          if (marginal_heatmap_kind_list.contains(QString::fromStdString(value)))
            element->setAttribute(label, value);
          else
            fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
      else if (label == "norm")
        {
          if (norm_list.contains(QString::fromStdString(value)))
            element->setAttribute(label, value);
          else
            fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
      else if (label == "plot_type")
        {
          if (plot_type_list.contains(QString::fromStdString(value)))
            element->setAttribute(label, value);
          else
            fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
      else if ((label == "size_x_type" || label == "size_y_type"))
        {
          if (size_type_list.contains(QString::fromStdString(value)))
            element->setAttribute(label, value);
          else
            fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
      else if (label == "style")
        {
          if (style_list.contains(QString::fromStdString(value)))
            element->setAttribute(label, value);
          else
            fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
      else if ((label == "x_org_pos" || label == "y_org_pos" || label == "z_org_pos"))
        {
          if (org_pos_list.contains(QString::fromStdString(value)))
            element->setAttribute(label, value);
          else
            fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
      else if (std::find(context_attributes.begin(), context_attributes.end(), label) != context_attributes.end())
        {
          QStringList context_attr_list;
          table_widget->extractContextNames(grm_get_render()->getContext());
          auto context_attr_vec = table_widget->getContextNames();
          context_attr_list.reserve((int)context_attr_vec.size());
          for (auto &i : context_attr_vec)
            {
              context_attr_list.push_back(i.c_str());
            }

          if (context_attr_list.contains(QString::fromStdString(value)))
            element->setAttribute(label, value);
          else
            fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
      else if (label == "step_where")
        {
          if (step_where_list.contains(QString::fromStdString(value)))
            element->setAttribute(label, value);
          else
            fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
      else
        {
          element->setAttribute(label, value);
        }
    }
  else if ((attr_type == "xs:integer" || attr_type == "strint") && !util::isDigits(value))
    {
      try
        {
          if (label == "text_align_vertical")
            {
              element->setAttribute(label, GRM::textAlignVerticalStringToInt(value));
            }
          else if (label == "text_align_horizontal")
            {
              element->setAttribute(label, GRM::textAlignHorizontalStringToInt(value));
            }
          else if (label == "algorithm")
            {
              element->setAttribute(label, GRM::algorithmStringToInt(value));
            }
          else if (label == "model")
            {
              element->setAttribute(label, GRM::modelStringToInt(value));
            }
          else if (label == "location" && element->localName() != "legend" && element->localName() != "axis")
            {
              if (side_region_location_list.contains(QString::fromStdString(value)))
                element->setAttribute(label, value);
              else
                fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
            }
          else if (label == "location" && element->localName() == "legend")
            {
              element->setAttribute(label, GRM::locationStringToInt(value));
            }
          else if (label == "location" && element->localName() == "axis")
            {
              auto type = static_cast<std::string>(current_selection->getRef()->getAttribute("axis_type"));
              if (type == "x")
                element->setAttribute(label, GRM::xAxisLocationStringToInt(value));
              else
                element->setAttribute(label, GRM::yAxisLocationStringToInt(value));
            }
          else if (label == "clip_region")
            {
              element->setAttribute(label, GRM::clipRegionStringToInt(value));
            }
          else if (label == "colormap")
            {
              element->setAttribute(label, GRM::colormapStringToInt(value));
            }
          else if (label == "text_encoding")
            {
              element->setAttribute(label, GRM::textEncodingStringToInt(value));
            }
          else if (label == "marker_type")
            {
              element->setAttribute(label, GRM::markerTypeStringToInt(value));
            }
          else if (label == "font")
            {
              element->setAttribute(label, GRM::fontStringToInt(value));
            }
          else if (label == "font_precision")
            {
              element->setAttribute(label, GRM::fontPrecisionStringToInt(value));
            }
          else if (label == "line_type")
            {
              element->setAttribute(label, GRM::lineTypeStringToInt(value));
            }
          else if (label == "resample_method")
            {
              element->setAttribute(label, GRM::resampleMethodStringToInt(value));
            }
          else if (label == "tick_orientation")
            {
              element->setAttribute(label, GRM::tickOrientationStringToInt(value));
            }
          else if (label == "scientific_format")
            {
              element->setAttribute(label, GRM::scientificFormatStringToInt(value));
            }
          else if (label == "error_bar_style")
            {
              element->setAttribute(label, GRM::errorBarStyleStringToInt(value));
            }
          else if (label == "fill_style")
            {
              element->setAttribute(label, GRM::fillStyleStringToInt(value));
            }
          else if (label == "fill_int_style")
            {
              element->setAttribute(label, GRM::fillIntStyleStringToInt(value));
            }
          else if (label == "transformation")
            {
              element->setAttribute(label, GRM::transformationStringToInt(value));
            }
        }
      catch (std::logic_error &e)
        {
          fprintf(stderr, "Invalid value %s for combobox attribute %s\n", value.c_str(), label.c_str());
        }
    }
}

void GRPlotWidget::attributeEditEvent(bool highlight_location)
{
  edit_element_widget->show();
  edit_element_widget->attributeEditEvent(highlight_location);
}

void GRPlotWidget::draw()
{
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
  if (!draw_called_at_least_once || in_listen_mode)
    {
      /* Call `grm_plot` at least once to initialize the internal argument container structure,
       * but use `grm_render` afterwards, so the graphics tree is not deleted every time. */
      was_successful = grm_plot(nullptr);
    }
  else
    {
      was_successful = grm_render();
    }
  if (!was_successful)
    {
      // Todo: detailed error message according to the specific error
      fprintf(stderr,
              "An error occured, the application will be closed. Please verify ur input is correct and try it again\n");
      exit(1);
    }
  draw_called_at_least_once = true;
}

void GRPlotWidget::redraw(bool full_redraw, bool update_tree)
{
  redraw_pixmap = full_redraw ? RedrawType::FULL : RedrawType::PARTIAL;
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
      if (keyboard_modifiers != Qt::AltModifier) tooltips.clear();
      auto current_tooltip = grm_get_tooltip(mouse_pos.x(), mouse_pos.y());
      if (current_tooltip != nullptr)
        {
          bool found_current_tooltip = false;
          for (const auto &tooltip : tooltips)
            {
              if (tooltip.get<grm_tooltip_info_t>()->x == current_tooltip->x &&
                  tooltip.get<grm_tooltip_info_t>()->y == current_tooltip->y)
                {
                  found_current_tooltip = true;
                  break;
                }
            }
          if (!found_current_tooltip) tooltips.emplace_back(current_tooltip);
        }
    }
}

static const std::string TOOLTIP_STYLE{"\
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

static const std::string TOOLTIP_TEMPLATE{"\
    <span class=\"gr-label\">%s</span><br>\n\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span><br>\n\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span>"};

static const std::string ACCUMULATED_TOOLTIP_TEMPLATE{"\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span>"};

void GRPlotWidget::paintEvent(QPaintEvent *event)
{
  if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
    {
      if (table_widget != nullptr && !table_widget->isVisible() && show_context_action->isChecked())
        show_context_action->setChecked(false);
      if (tree_widget != nullptr && !tree_widget->isVisible() && show_container_action->isChecked())
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
      redraw_pixmap = RedrawType::FULL;
    }

  if (redraw_pixmap == RedrawType::PARTIAL || redraw_pixmap == RedrawType::FULL)
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
          if (redraw_pixmap == RedrawType::PARTIAL)
            {
              // A PARTIAL redraw was requested, but if no plot is marked as active, we need to redraw the whole figure
              if ((active_figure->querySelectors("layout_grid") == nullptr ||
                   active_figure->querySelectors("plot[_active=\"1\"]") == nullptr) &&
                  (active_figure->querySelectors("plot[_active_through_update=\"1\"]") == nullptr))
                redraw_pixmap = RedrawType::FULL;
            }
          if (redraw_pixmap == RedrawType::FULL)
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
      redraw_pixmap = RedrawType::NONE;

      if (tree_widget != nullptr && tree_update) tree_widget->updateData(grm_get_document_root());
      if (mouse_move_triggert) collectTooltips();
      emit pixmapRedrawn();
    }

  painter.begin(paint_device);
  painter.drawPixmap(0, 0, pixmap);
  bounding_logic->clear();
  extractBoundingBoxesFromGRM((QPainter &)painter);
  highlightCurrentSelection((QPainter &)painter);
  // Todo: only trigger this method in non multiplot case where 1 plot has different series when not all elements has to
  // be processed to figure out which kinds are all used
  auto global_root = grm_get_document_root();
  if (!in_listen_mode && global_root->querySelectors("layout_grid") == nullptr)
    adjustPlotTypeMenu(global_root->querySelectors("figure[active=1]")->querySelectors("plot"));
  if (!tooltips.empty() && !enable_editor)
    {
      for (const auto &tooltip : tooltips)
        {
          if (tooltip.xPx() > 0 && tooltip.yPx() > 0)
            {
              QColor background(224, 224, 224, 128);
              QPainterPath triangle;
              std::string info, x_label = tooltip.xLabel();

              if (util::startsWith(x_label, "$") && util::endsWith(x_label, "$")) x_label = "x";

              if (tooltip.holdsAlternative<grm_tooltip_info_t>())
                {
                  const auto *single_tooltip = tooltip.get<grm_tooltip_info_t>();
                  std::string y_label = single_tooltip->y_label;

                  if (util::startsWith(y_label, "$") && util::endsWith(y_label, "$")) y_label = "y";
                  info = util::stringFormat(TOOLTIP_TEMPLATE, single_tooltip->label, x_label.c_str(), single_tooltip->x,
                                            y_label.c_str(), single_tooltip->y);
                }
              else
                {
                  const auto *accumulated_tooltip = tooltip.get<grm_accumulated_tooltip_info_t>();
                  std::vector<std::string> y_labels(accumulated_tooltip->y_labels,
                                                    accumulated_tooltip->y_labels + accumulated_tooltip->n);

                  std::vector<std::string> info_parts;
                  info_parts.push_back(
                      util::stringFormat(ACCUMULATED_TOOLTIP_TEMPLATE, x_label.c_str(), accumulated_tooltip->x));
                  for (int i = 0; i < y_labels.size(); ++i)
                    {
                      auto &y = accumulated_tooltip->y[i];
                      auto &y_label = y_labels[i];
                      if (util::startsWith(y_label, "$") && util::endsWith(y_label, "$")) y_label = "y";
                      info_parts.emplace_back("<br>\n");
                      info_parts.push_back(util::stringFormat(ACCUMULATED_TOOLTIP_TEMPLATE, y_label.c_str(), y));
                    }
                  std::ostringstream info_stream;
                  std::copy(info_parts.begin(), info_parts.end(), std::ostream_iterator<std::string>(info_stream));
                  info = info_stream.str();
                }
              label.setDefaultStyleSheet(QString::fromStdString(TOOLTIP_STYLE));
              label.setHtml(QString::fromStdString(info));

              /* tooltips for filled plot kinds gets a higher alpha value to make the values more visible */
              int width = this->size().width(), height = this->size().height();
              GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
              auto max_width_height = std::max(width, height);
              auto x = (double)tooltip.xPx() / max_width_height;
              auto y = (double)(height - tooltip.yPx()) / max_width_height;
              auto plot_element = grm_get_subplot_from_ndc_points_using_dom(1, &x, &y);
              if (plot_element != nullptr && plot_element->hasAttribute("_kind"))
                kind = static_cast<std::string>(plot_element->getAttribute("_kind"));
              if (kind == "heatmap" || kind == "marginal_heatmap" || kind == "contourf" || kind == "imshow")
                background.setAlpha(224);

              painter.fillRect(tooltip.xPx() + 8, (int)(tooltip.yPx() - label.size().height() / 2),
                               (int)label.size().width(), (int)label.size().height(),
                               QBrush(background, Qt::SolidPattern));

              triangle.moveTo(tooltip.xPx(), tooltip.yPx());
              triangle.lineTo(tooltip.xPx() + 8, tooltip.yPx() + 6);
              triangle.lineTo(tooltip.xPx() + 8, tooltip.yPx() - 6);
              triangle.closeSubpath();
              background.setRgb(128, 128, 128, 128);
              painter.fillPath(triangle, QBrush(background, Qt::SolidPattern));

              painter.save();
              painter.translate(tooltip.xPx() + 8, tooltip.yPx() - label.size().height() / 2);
              label.drawContents(&painter);
              painter.restore();
            }
        }
    }
  painter.end();
}

void GRPlotWidget::keyPressEvent(QKeyEvent *event)
{
  if (enable_editor)
    {
      if (event->key() == Qt::Key_Escape)
        {
          if (!current_selections.empty())
            {
              for (const auto &selection : current_selections)
                {
                  selection->getRef()->setAttribute("_selected", 0);
                }
              current_selections.clear();
            }
          if (current_selection) current_selection->getRef()->removeAttribute("_highlighted");
          current_selection = nullptr;
          mouse_move_selection = nullptr;
          prev_selection.reset();
          tree_widget->updateData(grm_get_document_root());
          redraw();
        }
      else if (event->key() == Qt::Key_Return)
        {
          attributeEditEvent();
        }
      else if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
        {
          if (current_selection == nullptr) return;
          amount_scrolled = 0;
          // to remove y_line, title, xlabel and ylabel from axis Node
          auto elem_name = static_cast<std::string>(current_selection->getRef()->getAttribute("name"));
          if (current_selection->getRef()->parentElement()->hasAttribute(elem_name))
            {
              if (elem_name == "y_line")
                {
                  current_selection->getRef()->parentElement()->setAttribute(elem_name, false);
                }
              else
                {
                  current_selection->getRef()->parentElement()->removeAttribute(elem_name);
                }
            }
          auto parent = current_selection->getRef()->parentElement();
          while (parent != nullptr && parent->localName() != "root" && parent->childElementCount() <= 1)
            {
              auto tmp_parent = parent->parentElement();
              // to remove x_tick_labels, y_tick_labels from coordinate_system
              elem_name = static_cast<std::string>(parent->getAttribute("name"));
              if (tmp_parent->hasAttribute(elem_name)) tmp_parent->removeAttribute(elem_name);
              parent->remove();
              parent = tmp_parent;
            }
          current_selection->getRef()->remove();
          // to prevent recreation of the tree a new flag is introduced
          if (parent->localName() == "root" && !parent->hasChildNodes())
            parent->setAttribute("_removed_children", true);
          mouse_move_selection = nullptr;
          resetPixmap();
        }
      else if (event->key() == Qt::Key_Shift)
        {
          if (!clicked.empty() && current_selection != nullptr)
            {
              for (int i = 0; i < clicked.size(); i++)
                {
                  if (clicked[i].getId() == current_selection->getId())
                    {
                      if (i + 1 < clicked.size())
                        {
                          current_selection = &clicked[i + 1];
                          break;
                        }
                      current_selection = &clicked[i + 1 - clicked.size()];
                      break;
                    }
                }
            }
          tree_widget->updateData(grm_get_document_root());
          tree_widget->selectItem(current_selection->getRef());
        }
      else if (event->key() == Qt::Key_Control)
        {
          ctrl_key_mode = true;
        }
      else if (event->key() == Qt::Key_R)
        {
          // Todo: maybe reset ndc_shift and ndc_scale
          resetPixmap();
        }
    }
  else
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
      if (mouse_state.mode == MouseState::Mode::MOVE_SELECTED && !ctrl_key_mode)
        {
          grm_args_t *args = grm_args_new();

          grm_args_push(args, "x", "i", mouse_state.anchor.x());
          grm_args_push(args, "y", "i", mouse_state.anchor.y());
          grm_args_push(args, "x_shift", "i", x - mouse_state.anchor.x());
          grm_args_push(args, "y_shift", "i", y - mouse_state.anchor.y());
          grm_args_push(args, "move_selection", "i", 1);

          grm_input(args);
          grm_args_delete(args);

          mouse_state.anchor.setX(x);
          mouse_state.anchor.setY(y);
          redraw();
        }
      else
        {
          cur_moved = bounding_logic->getBoundingObjectsAtPoint(x, y, hide_grid_bbox);

          if (current_selection == nullptr)
            {
              mouse_move_selection = nullptr;
              if (!cur_moved.empty()) mouse_move_selection = &cur_moved[0];
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
      auto plot_element = grm_get_subplot_from_ndc_points_using_dom(1, &x, &y);
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
      if (mouse_state.mode == MouseState::Mode::BOXZOOM)
        {
          rubber_band->setGeometry(QRect(mouse_state.pressed, event->pos()).normalized());
        }
      else if (mouse_state.mode == MouseState::Mode::PAN)
        {
          grm_args_t *args = grm_args_new();

          grm_args_push(args, "x", "i", mouse_state.anchor.x());
          grm_args_push(args, "y", "i", mouse_state.anchor.y());
          grm_args_push(args, "x_shift", "i", mouse_x - mouse_state.anchor.x());
          grm_args_push(args, "y_shift", "i", mouse_y - mouse_state.anchor.y());

          grm_input(args);
          grm_args_delete(args);

          mouse_state.anchor.setX(mouse_x);
          mouse_state.anchor.setY(mouse_y);
          redraw();
        }
      else if (mouse_state.mode == MouseState::Mode::MOVABLE_XFORM)
        {
          grm_args_t *args = grm_args_new();

          grm_args_push(args, "x", "i", mouse_state.anchor.x());
          grm_args_push(args, "y", "i", mouse_state.anchor.y());
          grm_args_push(args, "x_shift", "i", mouse_x - mouse_state.anchor.x());
          grm_args_push(args, "y_shift", "i", mouse_y - mouse_state.anchor.y());
          if (disable_movable_xform) grm_args_push(args, "disable_movable_trans", "i", disable_movable_xform);

          /* get the correct cursor and sets it */
          int cursor_state = grm_get_hover_mode(mouse_x, mouse_y, disable_movable_xform);
          grm_args_push(args, "movable_state", "i", cursor_state);

          grm_input(args);
          grm_args_delete(args);

          mouse_state.anchor.setX(mouse_x);
          mouse_state.anchor.setY(mouse_y);
          redraw();
        }
      else
        {
          collectTooltips();
          if (plot_element)
            {
              auto kind = static_cast<std::string>(plot_element->getAttribute("_kind"));
              if (kind == "marginal_heatmap")
                {
                  grm_args_t *input_args = grm_args_new();

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
                  auto kind = static_cast<std::string>(plot_elem->getAttribute("_kind"));
                  if (kind == "marginal_heatmap")
                    {
                      grm_args_t *input_args = grm_args_new();

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
  mouse_state.pressed = event->pos();
  if (event->button() == Qt::MouseButton::RightButton)
    {
      mouse_state.mode = MouseState::Mode::BOXZOOM;
      rubber_band->setGeometry(QRect(mouse_state.pressed, QSize()));
      rubber_band->show();
    }
  else if (event->button() == Qt::MouseButton::LeftButton)
    {
      int x, y;
      getMousePos(event, &x, &y);
      if (current_selections.empty() || ctrl_key_mode)
        {
          mouse_state.mode = MouseState::Mode::PAN;
        }
      else
        {
          mouse_state.mode = MouseState::Mode::MOVE_SELECTED;
        }
      mouse_state.anchor.setX(x);
      mouse_state.anchor.setY(y);

      int cursor_state = grm_get_hover_mode(x, y, disable_movable_xform);
      if (cursor_state != DEFAULT_HOVER_MODE)
        {
          grm_args_t *args = grm_args_new();
          grm_args_push(args, "clear_locked_state", "i", 1);
          grm_input(args);
          grm_args_delete(args);

          mouse_state.mode = MouseState::Mode::MOVABLE_XFORM;
        }

      if (enable_editor)
        {
          amount_scrolled = 0;
          auto cur_clicked = bounding_logic->getBoundingObjectsAtPoint(x, y, hide_grid_bbox);
          if (cur_clicked.empty())
            {
              clicked.clear();
              current_selection = nullptr;
              tree_widget->updateData(grm_get_document_root());
              update();
              return;
            }
          clicked = cur_clicked;
          current_selection = &clicked[0];
          if (ctrl_key_mode)
            {
              bool removed_selection = false;
              std::unique_ptr<BoundingObject> tmp(new BoundingObject(clicked[0]));
              for (auto it = std::begin(current_selections); it != std::end(current_selections); ++it)
                {
                  if ((*it)->getRef() == tmp->getRef())
                    {
                      (*it)->getRef()->setAttribute("_selected", 0);
                      it = current_selections.erase(it);
                      removed_selection = true;
                      break;
                    }
                }
              prev_selection.reset();
              if (!removed_selection)
                {
                  tmp->getRef()->setAttribute("_selected", 1);
                  addCurrentSelection(std::move(tmp));
                }
              mouse_state.mode = MouseState::Mode::MOVE_SELECTED;
            }
          tree_widget->updateData(grm_get_document_root());
          tree_widget->selectItem(current_selection->getRef());
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
              auto plot_element = grm_get_subplot_from_ndc_points_using_dom(1, &ndc_x, &ndc_y);
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

  if (mouse_state.mode == MouseState::Mode::BOXZOOM)
    {
      rubber_band->hide();
      if (std::abs(x - mouse_state.pressed.x()) >= 5 && std::abs(y - mouse_state.pressed.y()) >= 5)
        {
          grm_args_push(args, "keep_aspect_ratio", "i", event->modifiers() & Qt::ShiftModifier);
          grm_args_push(args, "x1", "i", mouse_state.pressed.x());
          grm_args_push(args, "y1", "i", mouse_state.pressed.y());
          grm_args_push(args, "x2", "i", x);
          grm_args_push(args, "y2", "i", y);
        }
    }
  else if (mouse_state.mode == MouseState::Mode::PAN)
    {
      mouse_state.mode = MouseState::Mode::NORMAL;
    }
  else if (mouse_state.mode == MouseState::Mode::MOVABLE_XFORM)
    {
      mouse_state.mode = MouseState::Mode::NORMAL;

      cursorHandler(x, y);
    }
  else if (mouse_state.mode == MouseState::Mode::MOVE_SELECTED)
    {
      mouse_state.mode = MouseState::Mode::NORMAL;
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

  if (current_selection) current_selection->getRef()->removeAttribute("_highlighted");
  current_selection = nullptr;
  for (const auto &selection : current_selections)
    {
      selection->getRef()->setAttribute("_selected", 0);
    }
  prev_selection.reset();
  current_selections.clear();
  mouse_move_selection = nullptr;
  amount_scrolled = 0;
  clicked.clear();
  tooltips.clear();
  resetPixmap();
}

void GRPlotWidget::wheelEvent(QWheelEvent *event)
{
  if (event->angleDelta().y() != 0)
    {
      int x, y;
      getWheelPos(event, &x, &y);
      QPoint num_pixels = event->pixelDelta();
      QPoint num_degrees = event->angleDelta();

      if (enable_editor)
        {
          if (!num_pixels.isNull())
            {
              // Scrolling with pixels (For high-res scrolling like on macOS)
              // Prevent flickering when scrolling fast
              if (num_pixels.y() > 0)
                {
                  amount_scrolled += num_pixels.y() < 10 ? num_pixels.y() : 10;
                }
              else if (num_pixels.y() < 0)
                {
                  amount_scrolled += num_pixels.y() > -10 ? num_pixels.y() : -10;
                }
            }
          else if (!num_degrees.isNull())
            {
              QPoint num_steps = num_degrees / 16;
              // Scrolling with degrees
              if (num_steps.y() != 0) amount_scrolled += num_steps.y();
            }

          if (amount_scrolled > 50)
            {
              if (!clicked.empty() && current_selection != nullptr)
                {
                  for (int i = 0; i < clicked.size(); i++)
                    {
                      if (clicked[i].getId() == current_selection->getId() && i + 1 < clicked.size())
                        {
                          current_selection = &clicked[i + 1];
                          tree_widget->updateData(grm_get_document_root());
                          tree_widget->selectItem(current_selection->getRef());
                          break;
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
                      if (clicked[i].getId() == current_selection->getId() && i - 1 > 0)
                        {
                          current_selection = &clicked[i - 1];
                          tree_widget->updateData(grm_get_document_root());
                          tree_widget->selectItem(current_selection->getRef());
                          break;
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
      if (event->button() == Qt::LeftButton) attributeEditEvent();
    }
  else
    {
      GR_UNUSED(event);
      grm_args_t *args = grm_args_new();
      const QPoint pos = mapFromGlobal(QCursor::pos());
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
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

void GRPlotWidget::marginalHeatmapAll()
{
  show_algo_menu_act->trigger();
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"series_heatmap", "series_surface",  "series_wireframe",
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

void GRPlotWidget::marginalHeatmapLine()
{
  show_algo_menu_act->trigger();
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"series_heatmap", "series_surface",  "series_wireframe",
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");

  for (const auto &elem : plot_elem->querySelectorsAll("series_scatter"))
    {
      elem->setAttribute("kind", "line");
    }

  // to get the same lines then before all lines have to exist during render call so that the line_spec work properly
  for (const auto &elem : plot_elem->querySelectorsAll("series_line"))
    {
      elem->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::sumAlgorithm()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");

  for (const auto &elem : plot_elem->querySelectorsAll("marginal_heatmap_plot"))
    {
      elem->setAttribute("algorithm", "sum");
    }
  redraw();
}

void GRPlotWidget::maxAlgorithm()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");

  for (const auto &elem : plot_elem->querySelectorsAll("marginal_heatmap_plot"))
    {
      elem->setAttribute("algorithm", "max");
    }
  redraw();
}

void GRPlotWidget::volume()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");

  for (const auto &elem : plot_elem->querySelectorsAll("series_isosurface"))
    {
      elem->setAttribute("kind", "volume");
    }
  redraw();
}
void GRPlotWidget::isosurface()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_heatmap",  "series_wireframe",
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface",  "series_heatmap",
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface",  "series_wireframe",
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface",  "series_wireframe",
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"series_scatter3", "series_tricontour", "series_trisurface",
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"marginal_heatmap_plot", "series_surface", "series_wireframe",
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"series_scatter3", "series_tricontour", "series_line3",
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"series_scatter3", "series_line3", "series_trisurface",
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"series_line3", "series_tricontour", "series_trisurface",
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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");

  for (const auto &elem : plot_elem->querySelectorsAll("series_line"))
    {
      elem->setAttribute("kind", "scatter");
    }
  redraw();
}

void GRPlotWidget::barplot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"series_stem", "series_stairs"};

  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "barplot");
        }
    }

  // to get the same bars then before all bars have to exist during render call so that the line_spec work properly
  for (const auto &elem : plot_elem->querySelectorsAll("series_barplot"))
    {
      elem->removeAttribute("fill_color_ind");
      elem->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::stairs()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"series_barplot", "series_stem"};

  for (const auto &name : valid_series_names)
    {
      auto series_elements = plot_elem->querySelectorsAll(name);
      for (const auto &series_elem : series_elements)
        {
          series_elem->setAttribute("kind", "stairs");
        }
    }

  // to get the same lines then before all lines have to exist during render call so that the line_spec work properly
  for (const auto &elem : plot_elem->querySelectorsAll("series_stairs"))
    {
      elem->setAttribute("_update_required", true);
    }
  redraw();
}

void GRPlotWidget::stem()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");
  const std::vector<std::string> valid_series_names = {"series_barplot", "series_stairs"};

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
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");

  for (const auto &elem : plot_elem->querySelectorsAll("series_hexbin"))
    {
      elem->setAttribute("kind", "shade");
    }
  redraw();
}

void GRPlotWidget::hexbin()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");

  for (const auto &elem : plot_elem->querySelectorsAll("series_shade"))
    {
      elem->setAttribute("kind", "hexbin");
    }
  redraw();
}

void GRPlotWidget::polarLine()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                  : global_root->querySelectors("figure[active=1]");

  for (const auto &elem : plot_elem->querySelectorsAll("series_polar_scatter"))
    {
      elem->setAttribute("kind", "polar_line");
    }
  redraw();
}

void GRPlotWidget::polarScatter()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto plot_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
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

void GRPlotWidget::xLogSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  bool x_log = plot_elem->hasAttribute("x_log") && static_cast<int>(plot_elem->getAttribute("x_log"));
  plot_elem->setAttribute("x_log", !x_log);

  redraw();
}

void GRPlotWidget::yLogSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  bool y_log = plot_elem->hasAttribute("y_log") && static_cast<int>(plot_elem->getAttribute("y_log"));
  plot_elem->setAttribute("y_log", !y_log);

  redraw();
}

void GRPlotWidget::zLogSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  bool z_log = plot_elem->hasAttribute("z_log") && static_cast<int>(plot_elem->getAttribute("z_log"));
  plot_elem->setAttribute("z_log", !z_log);

  redraw();
}

void GRPlotWidget::rLogSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  bool r_log = plot_elem->hasAttribute("r_log") && static_cast<int>(plot_elem->getAttribute("r_log"));
  plot_elem->setAttribute("r_log", !r_log);

  redraw();
}

void GRPlotWidget::xFlipSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  bool x_flip = plot_elem->hasAttribute("x_flip") && static_cast<int>(plot_elem->getAttribute("x_flip"));
  plot_elem->setAttribute("x_flip", !x_flip);

  redraw();
}

void GRPlotWidget::yFlipSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  bool y_flip = plot_elem->hasAttribute("y_flip") && static_cast<int>(plot_elem->getAttribute("y_flip"));
  plot_elem->setAttribute("y_flip", !y_flip);

  redraw();
}

void GRPlotWidget::zFlipSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  bool z_flip = plot_elem->hasAttribute("z_flip") && static_cast<int>(plot_elem->getAttribute("z_flip"));
  plot_elem->setAttribute("z_flip", !z_flip);

  redraw();
}

void GRPlotWidget::thetaFlipSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  bool theta_flip = plot_elem->hasAttribute("theta_flip") && static_cast<int>(plot_elem->getAttribute("theta_flip"));
  plot_elem->setAttribute("theta_flip", !theta_flip);

  redraw();
}

void GRPlotWidget::accelerateSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");

  auto series_elements = figure_elem->querySelectorsAll("series_surface");
  for (const auto &series_elem : series_elements)
    {
      bool accelerate = static_cast<int>(series_elem->getAttribute("accelerate"));
      series_elem->setAttribute("accelerate", !accelerate);
    }

  redraw();
}

void GRPlotWidget::polarWithPanSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  bool polar_with_pan =
      plot_elem->hasAttribute("polar_with_pan") && static_cast<int>(plot_elem->getAttribute("polar_with_pan"));
  plot_elem->setAttribute("polar_with_pan", !polar_with_pan);

  redraw();
}

void GRPlotWidget::keepWindowSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto central_region = figure_elem->querySelectors("central_region");

  bool keep_window =
      central_region->hasAttribute("keep_window") && static_cast<int>(central_region->getAttribute("keep_window"));
  central_region->setAttribute("keep_window", !keep_window);

  redraw();
}

void GRPlotWidget::colormapSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  auto colormap_old = GRM::colormapIntToString(static_cast<int>(plot_elem->getAttribute("colormap")));
  auto colormap_inverted =
      plot_elem->hasAttribute("colormap_inverted") && static_cast<int>(plot_elem->getAttribute("colormap_inverted"));

  QList<QRadioButton *> radio_buttons;
  QDialog dialog(this);
  QString title("Colormaps");
  dialog.setWindowTitle(title);
  auto form = new QFormLayout;
  auto colormap_names = GRM::getColormaps();

  // needed information to generate linear data for the context
  for (const auto &name : colormap_names)
    {
      QString tooltip_string = name.c_str();
      auto colormap = QPixmap((":/preview_images/colormaps/" + name + ".png").c_str());
      colormap.setDevicePixelRatio(10);
      if (colormap_inverted) colormap = colormap.transformed(QTransform().rotate(180));
      auto label_pix = new QLabel();
      label_pix->setPixmap(colormap);
      label_pix->setToolTip(tooltip_string);
      auto button = new QRadioButton(this);
      if (name == colormap_old) button->setChecked(true);

      form->addRow(label_pix, button);
      radio_buttons << button;
    }

  auto checkbox = new QCheckBox(this);
  checkbox->setToolTip("If checked flip the selected colormap");
  if (colormap_inverted) checkbox->setChecked(true);
  auto text_label = QString("colormap_inverted");
  form->addRow(text_label, checkbox);

  QDialogButtonBox button_box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
  form->addRow(&button_box);
  QObject::connect(&button_box, SIGNAL(accepted()), &dialog, SLOT(accept()));
  QObject::connect(&button_box, SIGNAL(rejected()), &dialog, SLOT(reject()));

  auto scroll_area_content = new QWidget;
  scroll_area_content->setLayout(form);
  auto scroll_area = new QScrollArea;
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll_area->setWidgetResizable(true);
  scroll_area->setWidget(scroll_area_content);

  auto group_box_layout = new QVBoxLayout;
  group_box_layout->addWidget(scroll_area);
  dialog.setLayout(group_box_layout);

  if (dialog.exec() == QDialog::Accepted)
    {
      std::vector<std::string> values;
      std::vector<double> data_vec;
      std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();

      if (enable_editor) createHistoryElement();

      for (int i = 0; i < colormap_names.size(); i++)
        {
          if (radio_buttons[i]->isChecked())
            plot_elem->setAttribute("colormap", GRM::colormapStringToInt(colormap_names[i]));
        }

      plot_elem->setAttribute("colormap_inverted", checkbox->isChecked());
    }

  redraw();
}

void GRPlotWidget::selectableGridSlot()
{
  hide_grid_bbox = !hide_grid_bbox;
  selectable_grid_act->setChecked(!hide_grid_bbox);
}

void GRPlotWidget::colorRGBPopUp(std::string attribute_name, const std::shared_ptr<GRM::Element> element)
{
  color_picker_rgb->show();
  color_picker_rgb->start(attribute_name, element);
  if (color_picker_rgb->exec() == QDialog::Accepted) redraw();
}

void GRPlotWidget::colorIndexPopUp(std::string attribute_name, int current_index,
                                   const std::shared_ptr<GRM::Element> element)
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  QList<QRadioButton *> radio_buttons;
  QDialog dialog(this);
  QString title(attribute_name.c_str());
  dialog.setWindowTitle(title);
  auto grid_layout = new QGridLayout;

  int col = 0;
  for (int index = 0; index < 1255; index++)
    {
      int errind;
      double r, g, b;
      QImage image(1, 1, QImage::Format_RGB32);
      QRgb value;

      if (plot_elem->hasAttribute("colormap"))
        {
          auto colormap = static_cast<int>(plot_elem->getAttribute("colormap"));
          gr_setcolormap(colormap);
        }
      gks_inq_color_rep(-1, index, -1, &errind, &r, &g, &b);
      value = qRgb(255 * r, 255 * g, 255 * b);
      image.setPixel(0, 0, value);

      auto label_pix = new QLabel();
      auto color_pic = QPixmap::fromImage(image);
      color_pic = color_pic.scaled(20, 20);
      label_pix->setPixmap(color_pic);
      auto button = new QRadioButton(this);
      auto label = new QLabel(std::to_string(index).c_str());

      if (index == current_index) button->setChecked(true);

      grid_layout->addWidget(button, index / 6, col++ % 18);
      grid_layout->addWidget(label_pix, index / 6, col++ % 18);
      grid_layout->addWidget(label, index / 6, col++ % 18);

      radio_buttons << button;
    }

  QDialogButtonBox button_box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
  grid_layout->addWidget(&button_box, 210, 0, 1, 18);
  QObject::connect(&button_box, SIGNAL(accepted()), &dialog, SLOT(accept()));
  QObject::connect(&button_box, SIGNAL(rejected()), &dialog, SLOT(reject()));

  auto scroll_area_content = new QWidget;
  scroll_area_content->setLayout(grid_layout);
  auto scroll_area = new QScrollArea;
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll_area->setWidgetResizable(true);
  scroll_area->setWidget(scroll_area_content);

  auto group_box_layout = new QVBoxLayout;
  group_box_layout->addWidget(scroll_area);
  dialog.setLayout(group_box_layout);

  if (dialog.exec() == QDialog::Accepted)
    {
      std::vector<std::string> values;
      std::vector<double> data_vec;
      std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();

      createHistoryElement();

      for (int i = 0; i < 1255; i++)
        {
          if (radio_buttons[i]->isChecked()) element->setAttribute(attribute_name, i);
        }
    }

  redraw();
}

void GRPlotWidget::createHistoryElement(std::string flag)
{
  if (grm_get_render() == nullptr)
    {
      QApplication::beep();
      return;
    }

  // keep history element number at <= 10
  if (history_count > 10)
    {
      std::string file_name = std::string(grm_tmp_dir) + "_history" + std::to_string(history_count - 10);
      std::ofstream open_file_stream(file_name);
      if (open_file_stream) std::remove(file_name.c_str());
    }

  std::string save_file_name;
  if (flag == "_forward")
    save_file_name = std::string(grm_tmp_dir) + flag + "_history" + std::to_string(history_forward_count++);
  else
    save_file_name = std::string(grm_tmp_dir) + "_history" + std::to_string(history_count++);
  std::ofstream save_file_stream(save_file_name);
  if (!save_file_stream)
    {
      std::stringstream text_stream;
      text_stream << "Could not create history entry \"" << save_file_name << "\".";
      QMessageBox::critical(this, "History creation not possible", QString::fromStdString(text_stream.str()));
      return;
    }
  auto graphics_tree_str = std::unique_ptr<char, decltype(&std::free)>(grm_dump_graphics_tree_str(), std::free);
  save_file_stream << graphics_tree_str.get() << std::endl;
  save_file_stream.close();
  undo_action->setVisible(true);
  redo_action->setVisible(false);
}

void GRPlotWidget::removeHistoryElement()
{
  if (grm_get_render() == nullptr)
    {
      QApplication::beep();
      return;
    }

  std::string file_name = std::string(grm_tmp_dir) + "_history" + std::to_string(--history_count);
  std::ofstream open_file_stream(file_name);
  if (open_file_stream) std::remove(file_name.c_str());
}

void GRPlotWidget::keepAspectRatioSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  bool keep_aspect_ratio =
      plot_elem->hasAttribute("keep_aspect_ratio") && static_cast<int>(plot_elem->getAttribute("keep_aspect_ratio"));
  plot_elem->setAttribute("keep_aspect_ratio", !keep_aspect_ratio);

  redraw();
}

void GRPlotWidget::onlySquareAspectRatioSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  bool only_square_aspect_ratio = plot_elem->hasAttribute("only_square_aspect_ratio") &&
                                  static_cast<int>(plot_elem->getAttribute("only_square_aspect_ratio"));
  plot_elem->setAttribute("only_square_aspect_ratio", !only_square_aspect_ratio);

  redraw();
}

void GRPlotWidget::verticalOrientationSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto central_region = figure_elem->querySelectors("central_region");

  central_region->setAttribute("orientation", "vertical");

  redraw();
}

void GRPlotWidget::horizontalOrientationSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto central_region = figure_elem->querySelectors("central_region");

  central_region->setAttribute("orientation", "horizontal");

  redraw();
}

void GRPlotWidget::legendSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto legend_elem = figure_elem->querySelectors("legend");

  if (legend_elem)
    {
      auto bbox_id = static_cast<int>(legend_elem->getAttribute("_bbox_id"));
      auto bbox_x_min = static_cast<double>(legend_elem->getAttribute("_bbox_x_min"));
      auto bbox_x_max = static_cast<double>(legend_elem->getAttribute("_bbox_x_max"));
      auto bbox_y_min = static_cast<double>(legend_elem->getAttribute("_bbox_y_min"));
      auto bbox_y_max = static_cast<double>(legend_elem->getAttribute("_bbox_y_max"));
      auto *bbox = new BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, legend_elem);

      editor_action->trigger(); // open the editor view to update the location
      current_selection = bbox;
      attributeEditEvent(true);
      editor_action->trigger(); // clode the editor view again

      redraw();
    }
}

void GRPlotWidget::colorbarSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto colorbar = figure_elem->querySelectors("colorbar");

  if (colorbar)
    {
      auto bbox_id = static_cast<int>(colorbar->parentElement()->getAttribute("_bbox_id"));
      auto bbox_x_min = static_cast<double>(colorbar->parentElement()->getAttribute("_bbox_x_min"));
      auto bbox_x_max = static_cast<double>(colorbar->parentElement()->getAttribute("_bbox_x_max"));
      auto bbox_y_min = static_cast<double>(colorbar->parentElement()->getAttribute("_bbox_y_min"));
      auto bbox_y_max = static_cast<double>(colorbar->parentElement()->getAttribute("_bbox_y_max"));
      auto *bbox =
          new BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, colorbar->parentElement());

      editor_action->trigger(); // open the editor view to update the location
      current_selection = bbox;
      attributeEditEvent(true);
      editor_action->trigger(); // clode the editor view again

      redraw();
    }
}

void GRPlotWidget::leftAxisSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto side_region = figure_elem->querySelectors("side_region[location=\"left\"]");

  if (side_region)
    {
      const auto side_plot_region = side_region->querySelectors("side_plot_region");

      for (const auto &axis : side_plot_region->children())
        {
          if (axis->localName() == "axis")
            {
              auto bbox_id = static_cast<int>(axis->getAttribute("_bbox_id"));
              auto bbox_x_min = static_cast<double>(axis->getAttribute("_bbox_x_min"));
              auto bbox_x_max = static_cast<double>(axis->getAttribute("_bbox_x_max"));
              auto bbox_y_min = static_cast<double>(axis->getAttribute("_bbox_y_min"));
              auto bbox_y_max = static_cast<double>(axis->getAttribute("_bbox_y_max"));
              auto *bbox = new BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, axis);

              editor_action->trigger(); // open the editor view to update the location
              current_selection = bbox;
              attributeEditEvent();
              editor_action->trigger(); // clode the editor view again
            }
        }

      redraw();
    }
}

void GRPlotWidget::rightAxisSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto side_region = figure_elem->querySelectors("side_region[location=\"right\"]");

  if (side_region)
    {
      const auto side_plot_region = side_region->querySelectors("side_plot_region");

      for (const auto &axis : side_plot_region->children())
        {
          if (axis->localName() == "axis")
            {
              auto bbox_id = static_cast<int>(axis->getAttribute("_bbox_id"));
              auto bbox_x_min = static_cast<double>(axis->getAttribute("_bbox_x_min"));
              auto bbox_x_max = static_cast<double>(axis->getAttribute("_bbox_x_max"));
              auto bbox_y_min = static_cast<double>(axis->getAttribute("_bbox_y_min"));
              auto bbox_y_max = static_cast<double>(axis->getAttribute("_bbox_y_max"));
              auto *bbox = new BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, axis);

              editor_action->trigger(); // open the editor view to update the location
              current_selection = bbox;
              attributeEditEvent();
              editor_action->trigger(); // clode the editor view again
            }
        }

      redraw();
    }
}

void GRPlotWidget::bottomAxisSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto side_region = figure_elem->querySelectors("side_region[location=\"bottom\"]");

  if (side_region)
    {
      const auto side_plot_region = side_region->querySelectors("side_plot_region");

      for (const auto &axis : side_plot_region->children())
        {
          if (axis->localName() == "axis")
            {
              auto bbox_id = static_cast<int>(axis->getAttribute("_bbox_id"));
              auto bbox_x_min = static_cast<double>(axis->getAttribute("_bbox_x_min"));
              auto bbox_x_max = static_cast<double>(axis->getAttribute("_bbox_x_max"));
              auto bbox_y_min = static_cast<double>(axis->getAttribute("_bbox_y_min"));
              auto bbox_y_max = static_cast<double>(axis->getAttribute("_bbox_y_max"));
              auto *bbox = new BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, axis);

              editor_action->trigger(); // open the editor view to update the location
              current_selection = bbox;
              attributeEditEvent();
              editor_action->trigger(); // clode the editor view again
            }
        }

      redraw();
    }
}

void GRPlotWidget::topAxisSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto side_region = figure_elem->querySelectors("side_region[location=\"top\"]");

  if (side_region)
    {
      const auto side_plot_region = side_region->querySelectors("side_plot_region");

      for (const auto &axis : side_plot_region->children())
        {
          if (axis->localName() == "axis")
            {
              auto bbox_id = static_cast<int>(axis->getAttribute("_bbox_id"));
              auto bbox_x_min = static_cast<double>(axis->getAttribute("_bbox_x_min"));
              auto bbox_x_max = static_cast<double>(axis->getAttribute("_bbox_x_max"));
              auto bbox_y_min = static_cast<double>(axis->getAttribute("_bbox_y_min"));
              auto bbox_y_max = static_cast<double>(axis->getAttribute("_bbox_y_max"));
              auto *bbox = new BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, axis);

              editor_action->trigger(); // open the editor view to update the location
              current_selection = bbox;
              attributeEditEvent();
              editor_action->trigger(); // clode the editor view again
            }
        }

      redraw();
    }
}

void GRPlotWidget::twinXAxisSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto coordinate_system = figure_elem->querySelectors("coordinate_system");

  if (coordinate_system)
    {
      const auto axis = coordinate_system->querySelectors("axis[location=\"twin_x\"]");

      auto bbox_id = static_cast<int>(axis->getAttribute("_bbox_id"));
      auto bbox_x_min = static_cast<double>(axis->getAttribute("_bbox_x_min"));
      auto bbox_x_max = static_cast<double>(axis->getAttribute("_bbox_x_max"));
      auto bbox_y_min = static_cast<double>(axis->getAttribute("_bbox_y_min"));
      auto bbox_y_max = static_cast<double>(axis->getAttribute("_bbox_y_max"));
      auto *bbox = new BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, axis);

      editor_action->trigger(); // open the editor view to update the location
      current_selection = bbox;
      attributeEditEvent();
      editor_action->trigger(); // clode the editor view again

      redraw();
    }
}

void GRPlotWidget::twinYAxisSlot()
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto coordinate_system = figure_elem->querySelectors("coordinate_system");

  if (coordinate_system)
    {
      const auto axis = coordinate_system->querySelectors("axis[location=\"twin_x\"]");

      auto bbox_id = static_cast<int>(axis->getAttribute("_bbox_id"));
      auto bbox_x_min = static_cast<double>(axis->getAttribute("_bbox_x_min"));
      auto bbox_x_max = static_cast<double>(axis->getAttribute("_bbox_x_max"));
      auto bbox_y_min = static_cast<double>(axis->getAttribute("_bbox_y_min"));
      auto bbox_y_max = static_cast<double>(axis->getAttribute("_bbox_y_max"));
      auto *bbox = new BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, axis);

      editor_action->trigger(); // open the editor view to update the location
      current_selection = bbox;
      attributeEditEvent();
      editor_action->trigger(); // clode the editor view again

      redraw();
    }
}

void GRPlotWidget::moveEvent(QMoveEvent *event)
{
  if (enable_editor)
    {
      tree_widget->move(this->width() + event->pos().x(),
                        event->pos().y() - (tree_widget->geometry().y() - tree_widget->pos().y()));
    }
}

void GRPlotWidget::extractBoundingBoxesFromGRM(QPainter &painter)
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
              auto b = BoundingObject(id, xmin, xmax, ymin, ymax, cur_child);
              bounding_logic->addBoundingObject(b);
              auto bounding_rect = b.boundingRect();
              if (highlight_bounding_objects)
                {
                  painter.drawRect(bounding_rect);
                  painter.drawText(bounding_rect.topLeft() + QPointF(5, 10), cur_child->localName().c_str());
                }
            }
        }
    }
}

void GRPlotWidget::highlightCurrentSelection(QPainter &painter)
{
  for (const auto &elem : grm_get_document_root()->querySelectorsAll("[_highlighted=\"1\"]"))
    {
      elem->removeAttribute("_highlighted");
    }

  if (enable_editor)
    {
      if (!current_selections.empty())
        {
          for (const auto &selection : current_selections)
            {
              painter.fillRect(selection->boundingRect(), QBrush(QColor(190, 210, 232, 150), Qt::Dense7Pattern));
              if (selection->getRef() != nullptr)
                painter.drawText(selection->boundingRect().topLeft() + QPointF(5, 10),
                                 selection->getRef()->localName().c_str());
            }
        }

      // only highlight current_selection when ctrl isn't pressed, else an element doesn't get visually removed from the
      // list cause it still gets highlighted
      if (!ctrl_key_mode && mouse_state.mode != MouseState::Mode::MOVE_SELECTED)
        {
          if (current_selection != nullptr)
            {
              painter.drawRect(current_selection->boundingRect().toRect());
              if (current_selection->getRef() != nullptr)
                {
                  painter.drawText(current_selection->boundingRect().topLeft() + QPointF(5, 10),
                                   current_selection->getRef()->localName().c_str());
                  current_selection->getRef()->setAttribute("_highlighted", true);
                  if (prev_selection.lock() == nullptr || prev_selection.lock() != current_selection->getRef())
                    redraw();
                  prev_selection = current_selection->getRef();
                }
            }
          else if (mouse_move_selection != nullptr)
            {
              painter.fillRect(mouse_move_selection->boundingRect(),
                               QBrush(QColor(190, 210, 232, 150), Qt::SolidPattern));
              if (mouse_move_selection->getRef() != nullptr)
                painter.drawText(mouse_move_selection->boundingRect().topLeft() + QPointF(5, 10),
                                 mouse_move_selection->getRef()->localName().c_str());
            }
        }
      if (selected_parent != nullptr)
        {
          auto rect = selected_parent->boundingRect();
          if (selected_parent->getRef() != nullptr)
            {
              auto bbox_xmin = static_cast<double>(selected_parent->getRef()->getAttribute("_bbox_x_min"));
              auto bbox_xmax = static_cast<double>(selected_parent->getRef()->getAttribute("_bbox_x_max"));
              auto bbox_ymin = static_cast<double>(selected_parent->getRef()->getAttribute("_bbox_y_min"));
              auto bbox_ymax = static_cast<double>(selected_parent->getRef()->getAttribute("_bbox_y_max"));
              rect = QRectF(bbox_xmin, bbox_ymin, bbox_xmax - bbox_xmin, bbox_ymax - bbox_ymin);
              painter.drawText(rect.topLeft() + QPointF(5, 10), selected_parent->getRef()->localName().c_str());
            }
          painter.fillRect(rect, QBrush(QColor(255, 0, 0, 30), Qt::SolidPattern));
        }
      if (!referenced_elements.empty())
        {
          for (const auto &elem : referenced_elements)
            {
              auto rect = elem.boundingRect();
              if (elem.getRef() != nullptr)
                {
                  auto bbox_xmin = static_cast<double>(elem.getRef()->getAttribute("_bbox_x_min"));
                  auto bbox_xmax = static_cast<double>(elem.getRef()->getAttribute("_bbox_x_max"));
                  auto bbox_ymin = static_cast<double>(elem.getRef()->getAttribute("_bbox_y_min"));
                  auto bbox_ymax = static_cast<double>(elem.getRef()->getAttribute("_bbox_y_max"));
                  rect = QRectF(bbox_xmin, bbox_ymin, bbox_xmax - bbox_xmin, bbox_ymax - bbox_ymin);
                  painter.drawText(rect.topLeft() + QPointF(5, 10), elem.getRef()->localName().c_str());
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

void GRPlotWidget::resetPixmap()
{
  redraw_pixmap = RedrawType::FULL;
  if (current_selection) current_selection->getRef()->removeAttribute("_highlighted");
  current_selection = nullptr;
  for (const auto &selection : current_selections)
    {
      selection->getRef()->setAttribute("_selected", 0);
    }
  prev_selection.reset();
  current_selections.clear();
  update();
}

void GRPlotWidget::loadFileSlot()
{
  if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
    {
#ifndef NO_XERCES_C
      std::string path =
          QFileDialog::getOpenFileName(this, "Open XML", QDir::homePath(), "XML files (*.xml)").toStdString();
      if (path.empty()) return;

      auto file = fopen(path.c_str(), "r");
      if (!file)
        {
          std::stringstream text_stream;
          text_stream << "Could not open the XML file \"" << path << "\".";
          QMessageBox::critical(this, "File open not possible", QString::fromStdString(text_stream.str()));
          return;
        }
      if (enable_editor) createHistoryElement();
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

void GRPlotWidget::saveFileSlot()
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
      if (save_file_name.empty()) return;
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

void GRPlotWidget::showContainerSlot()
{
  if (enable_editor)
    {
      if (show_container_action->isChecked())
        {
          tree_widget->show();
          tree_widget->updateData(grm_get_document_root());
        }
      else
        {
          tree_widget->hide();
          tree_widget->clearContractElements();
        }
      tree_widget->resize(350, height());
      tree_widget->move((int)(this->pos().x() + 0.5 * this->width() - 61),
                        this->pos().y() - 28 + tree_widget->geometry().y());
    }
}

void GRPlotWidget::enableEditorFunctions()
{
  if (editor_action->isChecked())
    {
      enable_editor = true;
      grm_tmp_dir = grm_get_render()->initializeHistory();
      add_element_action->setVisible(true);
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
      show_container_action->setVisible(false);
      show_container_action->setChecked(false);
      hide_configuration_menu_act->trigger();
      tree_widget->hide();
      add_element_widget->hide();
      editor_action->setText(tr("&Enable Editorview"));

      // if the editor gets turned off everything needs to be reseted
      if (current_selection) current_selection->getRef()->removeAttribute("_highlighted");
      current_selection = nullptr;
      mouse_move_selection = nullptr;
      amount_scrolled = 0;
      clicked.clear();

      for (const auto &selection : current_selections)
        {
          selection->getRef()->setAttribute("_selected", 0);
        }
      prev_selection.reset();
      current_selections.clear();
      redraw();
    }
}

void GRPlotWidget::addElementSlot()
{
  if (enable_editor)
    {
      if (add_element_action->isChecked())
        add_element_widget->show();
      else
        add_element_widget->hide();
      add_element_widget->resize(400, height());
      add_element_widget->move(this->pos().x() + this->width() + add_element_widget->width(),
                               this->pos().y() + add_element_widget->geometry().y() - 28);
    }
}

void GRPlotWidget::received(ArgsWrapper args)
{
  if (!isVisible()) window()->show();
  if (args_) grm_args_delete(args_);
  grm_switch(1);
  args_ = args.getWrapper();
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
  if (size_hint.isValid()) return size_hint;
  return QWidget::sizeHint();
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
      auto line_edit = new QLineEdit(&dialog);
      ((QLineEdit *)line_edit)->setText("");
      auto text_label = QString(label[i].c_str());
      form->addRow(text_label, line_edit);
      fields << line_edit;
    }

  QDialogButtonBox button_box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
  form->addRow(&button_box);
  QObject::connect(&button_box, SIGNAL(accepted()), &dialog, SLOT(accept()));
  QObject::connect(&button_box, SIGNAL(rejected()), &dialog, SLOT(reject()));

  auto scroll_area_content = new QWidget;
  scroll_area_content->setLayout(form);
  auto scroll_area = new QScrollArea;
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll_area->setWidgetResizable(true);
  scroll_area->setWidget(scroll_area_content);

  auto group_box_layout = new QVBoxLayout;
  group_box_layout->addWidget(scroll_area);
  dialog.setLayout(group_box_layout);

  if (dialog.exec() == QDialog::Accepted)
    {
      int n;
      double start, end;
      std::vector<std::string> values;
      std::vector<double> data_vec;
      std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();

      for (int i = 0; i < 4; i++)
        {
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
        }
    }
}

void GRPlotWidget::undoSlot()
{
  if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
    {
#ifndef NO_XERCES_C
      std::string path = std::string(grm_tmp_dir) + "_history" + std::to_string(--history_count);

      auto file = fopen(path.c_str(), "r");
      if (!file)
        {
          std::stringstream text_stream;
          text_stream << "Could not go back in history";
          QMessageBox::critical(this, "Going back in history not possible", QString::fromStdString(text_stream.str()));
          undo_action->setVisible(false);
          return;
        }
      createHistoryElement("_forward");
      grm_load_graphics_tree(file);
      redraw();
      if (table_widget->isVisible()) table_widget->updateData(grm_get_render()->getContext());

      if (edit_element_widget->isVisible()) edit_element_widget->hide();
      current_selections.clear();
      current_selection = nullptr;
      if (add_element_widget->isVisible()) add_element_widget->hide();
      redo_action->setVisible(true);
      if (history_count == 0) undo_action->setVisible(false);
#else
      std::stringstream text_stream;
      text_stream << "XML support not compiled in. Please recompile GRPlot with libxml2 support.";
      QMessageBox::critical(this, "File open not possible", QString::fromStdString(text_stream.str()));

      if (edit_element_widget->isVisible()) edit_element_widget->hide();
      current_selections.clear();
      current_selection = nullptr;
      if (add_element_widget->isVisible()) add_element_widget->hide();
      redo_action->setVisible(true);
      if (history_count == 0) undo_action->setVisible(false);
      return;
#endif
    }
}

void GRPlotWidget::redoSlot()
{
  if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
    {
#ifndef NO_XERCES_C
      std::string path = std::string(grm_tmp_dir) + "_forward_history" + std::to_string(--history_forward_count);

      auto file = fopen(path.c_str(), "r");
      if (!file)
        {
          std::stringstream text_stream;
          text_stream << "Could not go forward in history";
          QMessageBox::critical(this, "Going forward in history not possible",
                                QString::fromStdString(text_stream.str()));
          redo_action->setVisible(false);
          return;
        }
      grm_load_graphics_tree(file);
      redraw();
      if (table_widget->isVisible()) table_widget->updateData(grm_get_render()->getContext());

      if (edit_element_widget->isVisible()) edit_element_widget->hide();
      current_selections.clear();
      current_selection = nullptr;
      if (add_element_widget->isVisible()) add_element_widget->hide();
      if (history_forward_count == 0) redo_action->setVisible(false);
      undo_action->setVisible(true);
      history_count += 1;
#else
      std::stringstream text_stream;
      text_stream << "XML support not compiled in. Please recompile GRPlot with libxml2 support.";
      QMessageBox::critical(this, "File open not possible", QString::fromStdString(text_stream.str()));

      if (edit_element_widget->isVisible()) edit_element_widget->hide();
      current_selections.clear();
      current_selection = nullptr;
      if (add_element_widget->isVisible()) add_element_widget->hide();
      if (history_forward_count == 0) redo_action->setVisible(false);
      undo_action->setVisible(true);
      history_count += 1;
      return;
#endif
    }
}

void GRPlotWidget::sizeCallback(const grm_event_t *new_size_object)
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

void GRPlotWidget::cmdCallback(const grm_request_event_t *event)
{
  if (strcmp(event->request_string, "close") == 0) QApplication::quit();
}

Qt::KeyboardModifiers GRPlotWidget::queryKeyboardModifiers()
{
  return modifiers | QApplication::queryKeyboardModifiers();
}

void GRPlotWidget::processTestCommandsFile()
{
  if (!draw_called_at_least_once)
    {
      QTimer::singleShot(100, this, &GRPlotWidget::processTestCommandsFile);
      return;
    }
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
          if (words[0] == "setArg" && words.size() >= 4)
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
              auto val_utf8 = words[n - 1].toUtf8();
              auto value = val_utf8.constData();
              if (strcmp(value, "hist") == 0) value = "histogram";
              if (strcmp(value, "plot3") == 0) value = "line3";

              if (strcmp(value, "line") == 0)
                {
                  // to get the same lines then before all lines have to exist during render call so that the line_spec
                  // work properly
                  for (const auto &elem : global_root->querySelectorsAll("series_line"))
                    {
                      elem->setAttribute("_update_required", true);
                    }
                }
              if (strcmp(value, "barplot") == 0 || strcmp(value, "stairs") == 0)
                {
                  // to get the same barplots then before all lines have to exist during render call so that the
                  // line_spec work properly
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
              std::cerr << "Failed to parse mouseMoveEvent: " << line.toStdString() << std::endl;
              break;
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
              std::cerr << "Failed to parse wheelEvent: " << line.toStdString() << std::endl;
              break;
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
              std::cerr << "Failed to parse sleep: " << line.toStdString() << std::endl;
              break;
            }
          else if (words[0] == "resize" && words.size() == 3)
            {
              bool width_flag, height_flag;
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
              std::cerr << "Failed to parse resize: " << line.toStdString() << std::endl;
              break;
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

std::shared_ptr<GRM::Document> GRPlotWidget::getSchemaTree()
{
  return this->schema_tree;
}

void GRPlotWidget::setSelectedParent(BoundingObject *parent)
{
  this->selected_parent = parent;
}

BoundingObject *GRPlotWidget::getSelectedParent()
{
  return this->selected_parent;
}

void GRPlotWidget::setCurrentSelection(BoundingObject *p_current_selection)
{
  this->current_selection = p_current_selection;
}

BoundingObject **GRPlotWidget::getCurrentSelection()
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

QStringList GRPlotWidget::getColorIndAttributes()
{
  return color_ind_attr;
}

QStringList GRPlotWidget::getColorRGBAttributes()
{
  return color_rgb_attr;
}

void GRPlotWidget::addCurrentSelection(std::unique_ptr<BoundingObject> curr_selection)
{
  current_selections.emplace_back(std::move(curr_selection));
}

std::list<std::unique_ptr<BoundingObject>>::iterator
GRPlotWidget::eraseCurrentSelection(std::list<std::unique_ptr<BoundingObject>>::const_iterator current_selection)
{
  return current_selections.erase(current_selection);
}

const std::list<std::unique_ptr<BoundingObject>> &GRPlotWidget::getCurrentSelections() const
{
  return current_selections;
}

void GRPlotWidget::setTreeUpdate(bool status)
{
  this->tree_update = status;
}

void GRPlotWidget::editElementAccepted()
{
  if (current_selection) current_selection->getRef()->removeAttribute("_highlighted");
  current_selection = nullptr;
  mouse_move_selection = nullptr;
  amount_scrolled = 0;
  clicked.clear();

  for (const auto &selection : current_selections)
    {
      selection->getRef()->setAttribute("_selected", 0);
    }
  prev_selection.reset();
  current_selections.clear();
  redraw();
}

void GRPlotWidget::setReferencedElements(std::vector<BoundingObject> referenced_elements)
{
  this->referenced_elements = referenced_elements;
}

void GRPlotWidget::adjustPlotTypeMenu(std::shared_ptr<GRM::Element> plot_parent)
{
  bool error = false;
  auto central_region = plot_parent->querySelectors("central_region");
  if (central_region == nullptr) return;
  bool edit_enabled = getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0;

  // hide all menu elements
  if (tree_widget != nullptr) // dummy elem which only exist in default grplot case
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
      accelerate_act->setVisible(false);
      polar_with_pan_act->setVisible(false);
      z_flip_act->setVisible(false);
      z_log_act->setVisible(false);
      r_log_act->setVisible(false);
      theta_flip_act->setVisible(false);
      legend_act->setVisible(false);
      colorbar_act->setVisible(false);
      left_axis_act->setVisible(false);
      right_axis_act->setVisible(false);
      bottom_axis_act->setVisible(false);
      top_axis_act->setVisible(false);
      twin_x_axis_act->setVisible(false);
      twin_y_axis_act->setVisible(false);
      hide_orientation_sub_menu_act->trigger();
      hide_aspect_ratio_sub_menu_act->trigger();
      if (edit_enabled) hide_location_sub_menu_act->trigger();

      if (grm_args_contains(args_, "error"))
        {
          error = true;
          fprintf(stderr, "Plot types are not compatible with error-bars. The menu got disabled\n");
        }

      auto coordinate_system = plot_parent->querySelectors("coordinate_system");
      if (coordinate_system != nullptr)
        {
          auto plot_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
          if (plot_type == "2d") show_aspect_ratio_sub_menu_act->trigger();
        }

      if (coordinate_system && coordinate_system->querySelectors("axis[location=\"twin_x\"]"))
        {
          if (edit_enabled) show_location_sub_menu_act->trigger();
          twin_x_axis_act->setVisible(true);
        }
      if (coordinate_system && coordinate_system->querySelectors("axis[location=\"twin_y\"]"))
        {
          if (edit_enabled) show_location_sub_menu_act->trigger();
          twin_y_axis_act->setVisible(true);
        }
      if (plot_parent->querySelectors("legend") && coordinate_system)
        {
          // check for coordinate_system cause pie legends location cant be changed
          if (edit_enabled) show_location_sub_menu_act->trigger();
          legend_act->setVisible(true);
        }
      if (plot_parent->querySelectors("colorbar"))
        {
          if (edit_enabled) show_location_sub_menu_act->trigger();
          colorbar_act->setVisible(true);
        }
      if (plot_parent->querySelectors("side_plot_region"))
        {
          if (edit_enabled) show_location_sub_menu_act->trigger();
          auto side_plot_region = plot_parent->querySelectors("side_plot_region");
          auto location = static_cast<std::string>(side_plot_region->getAttribute("location"));
          if (location.empty())
            location = static_cast<std::string>(side_plot_region->parentElement()->getAttribute("location"));

          if (!side_plot_region->querySelectors("colorbar"))
            {
              if (location == "left")
                left_axis_act->setVisible(true);
              else if (location == "right")
                right_axis_act->setVisible(true);
              else if (location == "top")
                top_axis_act->setVisible(true);
              else if (location == "bottom")
                bottom_axis_act->setVisible(true);
            }
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
                  if (kind == "surface") accelerate_act->setVisible(true);
                  if (kind == "surface" || kind == "wireframe")
                    {
                      z_flip_act->setVisible(true);
                      z_log_act->setVisible(true);
                    }

                  heatmap_act->setVisible(true);
                  surface_act->setVisible(true);
                  wireframe_act->setVisible(true);
                  contour_act->setVisible(true);
                  contourf_act->setVisible(true);
                  imshow_act->setVisible(true);
                  show_marginal_sub_menu_act->trigger();
                }
              else if (kind == "line" || (kind == "scatter" && !grm_args_values(args_, "z", "D", &z, &z_length)))
                {
                  line_act->setVisible(true);
                  scatter_act->setVisible(true);
                  show_orientation_sub_menu_act->trigger();
                }
              else if (kind == "volume" || kind == "isosurface")
                {
                  volume_act->setVisible(true);
                  isosurface_act->setVisible(true);
                }
              else if (kind == "line3" || kind == "trisurface" || kind == "tricontour" || kind == "scatter3" ||
                       kind == "scatter")
                {
                  if (kind != "tricontour")
                    {
                      z_flip_act->setVisible(true);
                      z_log_act->setVisible(true);
                    }
                  line3_act->setVisible(true);
                  trisurf_act->setVisible(true);
                  tricont_act->setVisible(true);
                  scatter3_act->setVisible(true);
                  scatter_act->setVisible(true);
                }
              else if ((kind == "barplot" || kind == "stairs" || kind == "stem") && !error)
                {
                  barplot_act->setVisible(true);
                  stairs_act->setVisible(true);
                  stem_act->setVisible(true);
                  show_orientation_sub_menu_act->trigger();
                }
              else if (kind == "shade" || kind == "hexbin")
                {
                  shade_act->setVisible(true);
                  hexbin_act->setVisible(true);
                }
              else if (kind == "polar_line" || kind == "polar_scatter")
                {
                  polar_line_act->setVisible(true);
                  polar_scatter_act->setVisible(true);
                  polar_with_pan_act->setVisible(true);
                  theta_flip_act->setVisible(true);
                  r_log_act->setVisible(true);
                }
              else if (kind == "polar_heatmap" || kind == "polar_histogram")
                {
                  polar_with_pan_act->setVisible(true);
                  theta_flip_act->setVisible(true);
                  r_log_act->setVisible(true);
                }
              else if (kind == "histogram")
                {
                  show_orientation_sub_menu_act->trigger();
                }
              add_seperator_act->trigger();
            }
        }
    }
}

void GRPlotWidget::hidePlotTypeMenuElements()
{
  heatmap_act->setVisible(false);
  surface_act->setVisible(false);
  wireframe_act->setVisible(false);
  contour_act->setVisible(false);
  contourf_act->setVisible(false);
  imshow_act->setVisible(false);
  line_act->setVisible(false);
  scatter_act->setVisible(false);
  volume_act->setVisible(false);
  isosurface_act->setVisible(false);
  line3_act->setVisible(false);
  trisurf_act->setVisible(false);
  tricont_act->setVisible(false);
  scatter3_act->setVisible(false);
  barplot_act->setVisible(false);
  stairs_act->setVisible(false);
  stem_act->setVisible(false);
  shade_act->setVisible(false);
  hexbin_act->setVisible(false);
  polar_line_act->setVisible(false);
  polar_scatter_act->setVisible(false);
}

QAction *GRPlotWidget::getPdfAct()
{
  return png_act;
}

QAction *GRPlotWidget::getPngAct()
{
  return png_act;
}

QAction *GRPlotWidget::getJpegAct()
{
  return jpeg_act;
}

QAction *GRPlotWidget::getSvgAct()
{
  return svg_act;
}

QAction *GRPlotWidget::getLine3Act()
{
  return line3_act;
}

QAction *GRPlotWidget::getTrisurfAct()
{
  return trisurf_act;
}

QAction *GRPlotWidget::getTricontAct()
{
  return tricont_act;
}

QAction *GRPlotWidget::getScatter3Act()
{
  return scatter3_act;
}

QAction *GRPlotWidget::getBarplotAct()
{
  return barplot_act;
}

QAction *GRPlotWidget::getStairsAct()
{
  return stairs_act;
}

QAction *GRPlotWidget::getStemAct()
{
  return stem_act;
}

QAction *GRPlotWidget::getShadeAct()
{
  return shade_act;
}

QAction *GRPlotWidget::getHexbinAct()
{
  return hexbin_act;
}

QAction *GRPlotWidget::getPolarLineAct()
{
  return polar_line_act;
}

QAction *GRPlotWidget::getPolarScatterAct()
{
  return polar_scatter_act;
}

QAction *GRPlotWidget::getLineAct()
{
  return line_act;
}

QAction *GRPlotWidget::getScatterAct()
{
  return scatter_act;
}

QAction *GRPlotWidget::getVolumeAct()
{
  return volume_act;
}

QAction *GRPlotWidget::getIsosurfaceAct()
{
  return isosurface_act;
}

QAction *GRPlotWidget::getHeatmapAct()
{
  return heatmap_act;
}

QAction *GRPlotWidget::getSurfaceAct()
{
  return surface_act;
}

QAction *GRPlotWidget::getWireframeAct()
{
  return wireframe_act;
}

QAction *GRPlotWidget::getContourAct()
{
  return contour_act;
}

QAction *GRPlotWidget::getImshowAct()
{
  return imshow_act;
}

QAction *GRPlotWidget::getContourfAct()
{
  return contourf_act;
}

QAction *GRPlotWidget::getSumAct()
{
  return sum_act;
}

QAction *GRPlotWidget::getMaxAct()
{
  return max_act;
}

QAction *GRPlotWidget::getMarginalHeatmapAllAct()
{
  return marginal_heatmap_all_act;
}

QAction *GRPlotWidget::getMarginalHeatmapLineAct()
{
  return marginal_heatmap_line_act;
}

QAction *GRPlotWidget::getMovableModeAct()
{
  return moveable_mode_act;
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

QAction *GRPlotWidget::getHideOrientationSubMenuAct()
{
  return hide_orientation_sub_menu_act;
}

QAction *GRPlotWidget::getShowOrientationSubMenuAct()
{
  return show_orientation_sub_menu_act;
}

QAction *GRPlotWidget::getHideAspectRatioSubMenuAct()
{
  return hide_aspect_ratio_sub_menu_act;
}

QAction *GRPlotWidget::getShowAspectRatioSubMenuAct()
{
  return show_aspect_ratio_sub_menu_act;
}

QAction *GRPlotWidget::getHideLocationSubMenuAct()
{
  return hide_location_sub_menu_act;
}

QAction *GRPlotWidget::getShowLocationSubMenuAct()
{
  return show_location_sub_menu_act;
}

QAction *GRPlotWidget::getAddSeperatorAct()
{
  return add_seperator_act;
}

QAction *GRPlotWidget::getXLogAct()
{
  return x_log_act;
}

QAction *GRPlotWidget::getYLogAct()
{
  return y_log_act;
}

QAction *GRPlotWidget::getZLogAct()
{
  return z_log_act;
}

QAction *GRPlotWidget::getRLogAct()
{
  return r_log_act;
}

QAction *GRPlotWidget::getXFlipAct()
{
  return x_flip_act;
}

QAction *GRPlotWidget::getYFlipAct()
{
  return y_flip_act;
}

QAction *GRPlotWidget::getZFlipAct()
{
  return z_flip_act;
}

QAction *GRPlotWidget::getThetaFlipAct()
{
  return theta_flip_act;
}

QAction *GRPlotWidget::getAccelerateAct()
{
  return accelerate_act;
}

QAction *GRPlotWidget::getPolarWithPanAct()
{
  return polar_with_pan_act;
}

QAction *GRPlotWidget::getKeepWindowAct()
{
  return keep_window_act;
}

QAction *GRPlotWidget::getKeepAspectRatioAct()
{
  return keep_aspect_ratio_act;
}

QAction *GRPlotWidget::getOnlySquareAspectRatioAct()
{
  return only_square_aspect_ratio_act;
}

QAction *GRPlotWidget::getVerticalOrientationAct()
{
  return vertical_orientation_act;
}

QAction *GRPlotWidget::getHorizontalOrientationAct()
{
  return horizontal_orientation_act;
}

QAction *GRPlotWidget::getLegendAct()
{
  return legend_act;
}

QAction *GRPlotWidget::getColorbarAct()
{
  return colorbar_act;
}
QAction *GRPlotWidget::getLeftAxisAct()
{
  return left_axis_act;
}

QAction *GRPlotWidget::getRightAxisAct()
{
  return right_axis_act;
}

QAction *GRPlotWidget::getBottomAxisAct()
{
  return bottom_axis_act;
}

QAction *GRPlotWidget::getTopAxisAct()
{
  return top_axis_act;
}

QAction *GRPlotWidget::getTwinXAxisAct()
{
  return twin_x_axis_act;
}

QAction *GRPlotWidget::getTwinYAxisAct()
{
  return twin_y_axis_act;
}

QAction *GRPlotWidget::getColormapAct()
{
  return colormap_act;
}

QAction *GRPlotWidget::getUndoAct()
{
  return undo_action;
}

QAction *GRPlotWidget::getRedoAct()
{
  return redo_action;
}

QAction *GRPlotWidget::getSelectableGridAct()
{
  return selectable_grid_act;
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
