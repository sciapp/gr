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
#include <QCursor>

#include "gredit/Bounding_object.h"
#include "gredit/Bounding_logic.h"
class GRPlotWidget;
#include "gredit/TreeWidget.h"
#include "gredit/AddElementWidget.h"
#include "gredit/EditElementWidget.h"
#include "gredit/TableWidget.h"
#include "qtterm/receiver_thread.h"
#include "qtterm/grm_args_t_wrapper.h"
#include "util.hxx"

#include <grm.h>

#define DEFAULT_HOVER_MODE 0
#define MOVABLE_HOVER_MODE 1
#define INTEGRAL_SIDE_HOVER_MODE 2

class GRPlotWidget : public QWidget
{
  Q_OBJECT

public:
  explicit GRPlotWidget(QMainWindow *parent, int argc, char **argv);
  virtual ~GRPlotWidget() override;
  void redraw(bool tree_update = true);
  std::shared_ptr<GRM::Document> get_schema_tree();
  void set_selected_parent(Bounding_object *parent);
  Bounding_object *get_selected_parent();
  void set_current_selection(Bounding_object *current_selection);
  Bounding_object **get_current_selection();
  void add_current_selection(std::unique_ptr<Bounding_object> current_selection);
  std::list<std::unique_ptr<Bounding_object>>::iterator
  erase_current_selection(std::list<std::unique_ptr<Bounding_object>>::const_iterator current_selection);
  const std::list<std::unique_ptr<Bounding_object>> &get_current_selections() const;
  void AttributeEditEvent();
  void attributeComboBoxHandler(const std::string &cur_attr_name, std::string cur_elem_name, QWidget **lineEdit);
  QStringList getCheckBoxAttributes();
  QStringList getComboBoxAttributes();
  void attributeSetForComboBox(const std::string &attr_type, std::shared_ptr<GRM::Element> element,
                               const std::string &value, const std::string &label);
  void advancedAttributeComboBoxHandler(const std::string &cur_attr_name, std::string cur_elem_name,
                                        QWidget **lineEdit);
  void setTreeUpdate(bool status);
  void editElementAccepted();

protected:
  virtual void draw();
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
  void leaveEvent(QEvent *event) override;
  void paint(QPaintDevice *paint_device);
  void processTestCommandsFile();
  static Qt::KeyboardModifiers queryKeyboardModifiers();

signals:
  void pixmapRedrawn();

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
  void polar_line();
  void polar_scatter();
  void pdf();
  void png();
  void jpeg();
  void svg();
  void moveableMode();
  void show_container_slot();
  void show_bounding_boxes_slot();
  void save_file_slot();
  void load_file_slot();
  void enable_editor_functions();
  void add_element_slot();
  void received(grm_args_t_wrapper args);
  void screenChanged();
  void showContextSlot();
  void addContextSlot();
  void addGRPlotDataContextSlot();
  void generateLinearContextSlot();

private:
  struct MouseState
  {
    enum class Mode
    {
      normal,
      pan,
      boxzoom,
      movable_xform,
      move_selected,
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

  bool in_listen_mode = false;
  QPixmap pixmap;
  bool redraw_pixmap;
  grm_args_t *args_;
  MouseState mouseState;
  QRubberBand *rubberBand;
  std::vector<TooltipWrapper> tooltips;
  QTextDocument label;
  Bounding_logic *bounding_logic;
  std::vector<Bounding_object> clicked;
  Bounding_object *current_selection, *mouse_move_selection, *selected_parent;
  std::list<std::unique_ptr<Bounding_object>> current_selections;
  bool highlightBoundingObjects;
  TreeWidget *treewidget;
  AddElementWidget *add_element_widget;
  int amount_scrolled;
  bool enable_editor;
  Receiver *receiver;
  std::shared_ptr<GRM::Document> schema_tree;
  bool tree_update = true;
  QSize size_hint;
  QStringList check_box_attr, combo_box_attr;
  TableWidget *table_widget;
  EditElementWidget *edit_element_widget;

  QMenuBar *menu;
  QMenu *type, *algo, *export_menu, *editor_menu, *modi_menu;
  QMenu *file_menu, *configuration_menu, *context_menu, *add_context_data;
  QAction *marginalheatmapAllAct, *marginalheatmapLineAct;
  QAction *sumAct, *maxAct;
  QAction *lineAct, *scatterAct;
  QAction *volumeAct, *isosurfaceAct;
  QAction *heatmapAct, *surfaceAct, *wireframeAct, *contourAct, *imshowAct, *contourfAct;
  QAction *plot3Act, *trisurfAct, *tricontAct, *scatter3Act;
  QAction *histAct, *barplotAct, *stairsAct, *stemAct;
  QAction *shadeAct, *hexbinAct;
  QAction *polarLineAct, *polarScatterAct;
  QAction *PdfAct, *PngAct, *JpegAct, *SvgAct;
  QAction *show_container_action, *show_bounding_boxes_action, *save_file_action, *load_file_action, *editor_action,
      *add_element_action;
  QAction *moveableModeAct;
  QAction *show_context_action, *add_context_action, *generate_linear_context_action, *add_grplot_data_context;
  QCursor *csr;

  void reset_pixmap();
  void moveEvent(QMoveEvent *event) override;
  void highlight_current_selection(QPainter &painter);
  void extract_bounding_boxes_from_grm(QPainter &painter);
  void showEvent(QShowEvent *) override;
  void closeEvent(QCloseEvent *event) override;
  QSize sizeHint() const override;
  void size_callback(const grm_event_t *);
  void cmd_callback(const grm_request_event_t *);
};

#endif /* ifndef GRPLOT_WIDGET_H_INCLUDED */
