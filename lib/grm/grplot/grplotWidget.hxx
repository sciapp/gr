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
#include <QRadioButton>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>

#include "gredit/BoundingObject.hxx"
#include "gredit/BoundingLogic.hxx"
#include "gredit/PreviewTextWidget.hxx"
class GRPlotWidget;
#include "gredit/TreeWidget.hxx"
#include "gredit/AddElementWidget.hxx"
#include "gredit/EditElementWidget.hxx"
#include "gredit/TableWidget.hxx"
#include "gredit/ColorPickerRGB.hxx"
#include "gredit/SelectionListWidget.hxx"
#include "gredit/IconBarWidget.hxx"
#include "qtterm/Receiver.hxx"
#include "qtterm/ArgsWrapper.hxx"
#include "util.hxx"

#include <grm.h>

#define DEFAULT_HOVER_MODE 0
#define MOVABLE_HOVER_MODE 1
#define INTEGRAL_SIDE_HOVER_MODE 2
#define LEGEND_ELEMENT_HOVER_MODE 3
#define VERTICAL_SCALE_HOVER_MODE 4
#define HORIZONTAL_SCALE_HOVER_MODE 5
#define B_DIAGONAL_SCALE_HOVER_MODE 6
#define F_DIAGONAL_SCALE_HOVER_MODE 7
#define MOVE_HOVER_MODE 8

class GRPlotWidget : public QWidget
{
  Q_OBJECT

public:
  explicit GRPlotWidget(QMainWindow *parent, int argc, char **argv, bool listen_mode = false, int listen_port = 8002,
                        bool test_mode = false, const QString &test_commands = "");
  explicit GRPlotWidget(QMainWindow *parent, grm_args_t *args);
  virtual ~GRPlotWidget() override;
  void redraw(bool full_redraw = false, bool tree_update = true);
  void addCurrentSelection(std::unique_ptr<BoundingObject> current_selection);
  std::list<std::unique_ptr<BoundingObject>>::iterator
  eraseCurrentSelection(std::list<std::unique_ptr<BoundingObject>>::const_iterator current_selection);
  void attributeEditEvent(bool highlight_location = false);
  void attributeComboBoxHandler(const std::string &cur_attr_name, std::string cur_elem_name, QWidget **line_edit);
  void attributeSetForComboBox(const std::string &attr_type, const std::shared_ptr<GRM::Element> &element,
                               const std::string &value, const std::string &label);
  void advancedAttributeComboBoxHandler(const std::string &cur_attr_name, std::string cur_elem_name,
                                        QWidget **line_edit);
  void editElementAccepted(bool highlight_location);
  void processTestCommandsFile();

  void setSelectedParent(BoundingObject *parent);
  void setCurrentSelection(BoundingObject *current_selection);
  void setTreeUpdate(bool status);
  void setReferencedElements(const std::vector<BoundingObject> &referenced_elements);
  void colorIndexPopUp(const std::string &attribute_name, int current_index,
                       const std::shared_ptr<GRM::Element> &element);
  void colorRGBPopUp(const std::string &attribute_name, const std::shared_ptr<GRM::Element> &element);
  void createHistoryElement(std::string flag = "");
  static void removeHistoryElement();
  void highlightTableWidgetAt(std::string column_name);
  void setUpPreviewTextWidget(const std::string &text, int scientific_format, int text_color, int font_precision,
                              int width, int height);

  inline bool isDarkMode()
  {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const auto scheme = QGuiApplication::styleHints()->colorScheme();
    return scheme == Qt::ColorScheme::Dark;
#else
    const QPalette default_palette;
    const auto text = default_palette.color(QPalette::WindowText);
    const auto window = default_palette.color(QPalette::Window);
    return text.lightness() > window.lightness();
#endif // QT_VERSION
  }

  const std::list<std::unique_ptr<BoundingObject>> &getCurrentSelections() const;
  std::shared_ptr<GRM::Document> getSchemaTree();
  QStringList getCheckBoxAttributes();
  QStringList getComboBoxAttributes();
  QStringList getColorIndAttributes();
  QStringList getColorRGBAttributes();
  QStringList getSliderAttributes();
  BoundingObject *getSelectedParent();
  BoundingObject **getCurrentSelection();
  bool getEnableAdvancedEditor();
  QAction *getPdfAct();
  QAction *getPngAct();
  QAction *getJpegAct();
  QAction *getSvgAct();
  QAction *getLine3Act();
  QAction *getTrisurfAct();
  QAction *getTricontAct();
  QAction *getScatter3Act();
  QAction *getBarplotAct();
  QAction *getStairsAct();
  QAction *getStemAct();
  QAction *getShadeAct();
  QAction *getHexbinAct();
  QAction *getPolarLineAct();
  QAction *getPolarScatterAct();
  QAction *getLineAct();
  QAction *getScatterAct();
  QAction *getVolumeAct();
  QAction *getIsosurfaceAct();
  QAction *getHeatmapAct();
  QAction *getSurfaceAct();
  QAction *getWireframeAct();
  QAction *getContourAct();
  QAction *getImshowAct();
  QAction *getContourfAct();
  QAction *getSumAct();
  QAction *getMaxAct();
  QAction *getMarginalHeatmapAllAct();
  QAction *getMarginalHeatmapLineAct();
  QAction *getMovableModeAct();
  QAction *getEditorAct();
  QAction *getSaveFileAct();
  QAction *getLoadFileAct();
  QAction *getShowContainerAct();
  QAction *getAddElementAct();
  QAction *getShowContextAct();
  QAction *getAddContextAct();
  QAction *getAddGRplotDataContextAct();
  QAction *getGenerateLinearContextAct();
  QTextStream *getTestCommandsStream();
  QAction *getHideAlgoMenuAct();
  QAction *getShowAlgoMenuAct();
  QAction *getHideMarginalSubMenuAct();
  QAction *getShowMarginalSubMenuAct();
  QAction *getHideOrientationSubMenuAct();
  QAction *getShowOrientationSubMenuAct();
  QAction *getHideAspectRatioSubMenuAct();
  QAction *getShowAspectRatioSubMenuAct();
  QAction *getHideLocationSubMenuAct();
  QAction *getShowLocationSubMenuAct();
  QAction *getHideLimSubMenuAct();
  QAction *getShowLimSubMenuAct();
  QAction *getHideLogSubMenuAct();
  QAction *getShowLogSubMenuAct();
  QAction *getHideFlipSubMenuAct();
  QAction *getShowFlipSubMenuAct();
  QAction *getHidePlotTypeSubMenuAct();
  QAction *getShowPlotTypeSubMenuAct();
  QAction *getAddSeperatorAct();
  QAction *getXLogAct();
  QAction *getYLogAct();
  QAction *getZLogAct();
  QAction *getRLogAct();
  QAction *getXFlipAct();
  QAction *getYFlipAct();
  QAction *getZFlipAct();
  QAction *getThetaFlipAct();
  QAction *getUseGR3Act();
  QAction *getPolarWithPanAct();
  QAction *getKeepWindowAct();
  QAction *getKeepAspectRatioAct();
  QAction *getOnlySquareAspectRatioAct();
  QAction *getVerticalOrientationAct();
  QAction *getHorizontalOrientationAct();
  QAction *getLegendAct();
  QAction *getColorbarAct();
  QAction *getLeftAxisAct();
  QAction *getRightAxisAct();
  QAction *getBottomAxisAct();
  QAction *getTopAxisAct();
  QAction *getTwinXAxisAct();
  QAction *getTwinYAxisAct();
  QAction *getColormapAct();
  QAction *getUndoAct();
  QAction *getRedoAct();
  QAction *getSelectableGridAct();
  QAction *getAdvancedEditorAct();
  QAction *getShowEditElementAct();
  QAction *getShowTreeWidgetAct();
  QAction *getShowTableWidgetAct();
  QAction *getShowTextPreviewAct();
  QAction *getShowSelectionListWidgetAct();
  QAction *getShowIconBarAct();
  QAction *getShowAddElementAct();
  QAction *getHideEditElementAct();
  QAction *getHideTreeWidgetAct();
  QAction *getHideTableWidgetAct();
  QAction *getHideTextPreviewAct();
  QAction *getHideSelectionListWidgetAct();
  QAction *getHideIconBarAct();
  QAction *getHideAddElementAct();
  QAction *getXLimAct();
  QAction *getYLimAct();
  QAction *getZLimAct();
  QAction *getIconBarAct();
  QAction *getTextColorIndAct();
  QWidget *getEditElementWidget();
  QWidget *getTreeWidget();
  QWidget *getTableWidget();
  QWidget *getTextPreviewWidget();
  QWidget *getSelectionListWidget();
  QWidget *getIconBarWidget();
  QWidget *getAddElementWidget();

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
  static Qt::KeyboardModifiers queryKeyboardModifiers();

signals:
  void pixmapRedrawn();
  void signalMoved(QRect const &r);

private slots:
  void heatmap();
  void marginalHeatmapAll();
  void marginalHeatmapLine();
  void line();
  void sumAlgorithm();
  void maxAlgorithm();
  void volume();
  void isosurface();
  void surface();
  void wireframe();
  void contour();
  void imshow();
  void line3();
  void contourf();
  void trisurf();
  void tricont();
  void scatter3();
  void scatter();
  void barplot();
  void stairs();
  void stem();
  void shade();
  void hexbin();
  void polarLine();
  void polarScatter();
  void pdf();
  void png();
  void jpeg();
  void svg();
  void moveableMode();
  void showContainerSlot();
  void saveFileSlot();
  void loadFileSlot();
  void enableEditorFunctions();
  void addElementSlot();
  void received(ArgsWrapper args);
  void screenChanged();
  void showContextSlot();
  void addContextSlot();
  void addGRPlotDataContextSlot();
  void generateLinearContextSlot();
  void xLogSlot();
  void yLogSlot();
  void zLogSlot();
  void rLogSlot();
  void xFlipSlot();
  void yFlipSlot();
  void zFlipSlot();
  void thetaFlipSlot();
  void useGR3Slot();
  void polarWithPanSlot();
  void keepWindowSlot();
  void onlySquareAspectRatioSlot();
  void keepAspectRatioSlot();
  void verticalOrientationSlot();
  void horizontalOrientationSlot();
  void legendSlot();
  void colorbarSlot();
  void leftAxisSlot();
  void rightAxisSlot();
  void bottomAxisSlot();
  void topAxisSlot();
  void twinXAxisSlot();
  void twinYAxisSlot();
  void colormapSlot();
  void undoSlot();
  void redoSlot();
  void selectableGridSlot();
  void advancedEditorSlot();
  void addTextSlot();
  void xLimSlot();
  void yLimSlot();
  void zLimSlot();
  void addImageSlot();
  void listItemCheckStatusChanged(QListWidgetItem *item);
  void listItemPressed(QListWidgetItem *item);
  void showIconBarSlot();
  void multipleRadioButtonGroupsListener();
  void colorIndexSlot();

private:
  struct MouseState
  {
    enum class Mode
    {
      NORMAL,
      PAN,
      BOXZOOM,
      MOVABLE_XFORM,
      MOVE_SELECTED,
      LEGEND_SELECTED,
      HORIZONTAL_SCALE_SELECTED,
      VERTICAL_SCALE_SELECTED,
      B_DIAGONAL_SCALE_SELECTED,
      F_DIAGONAL_SCALE_SELECTED,
    };
    Mode mode;
    QPoint pressed;
    QPoint anchor;
  };

  enum class RedrawType
  {
    NONE,
    PARTIAL,
    FULL,
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
      if (holdsAlternative<grm_accumulated_tooltip_info_t>())
        {
          auto accumulated_tooltip = get<grm_accumulated_tooltip_info_t>();
          std::free(accumulated_tooltip->y);
          std::free(accumulated_tooltip->y_labels);
        }
      std::visit([](auto *x) { std::free(x); }, tooltip_);
    }

    template <typename T> T *get() { return std::get<T *>(tooltip_); };
    template <typename T> const T *get() const { return std::get<T *>(tooltip_); };

    template <typename T> bool holdsAlternative() const { return std::holds_alternative<T *>(tooltip_); };

    int n() const
    {
      return std::visit(util::Overloaded{[](const grm_tooltip_info_t *) { return 1; },
                                         [](const grm_accumulated_tooltip_info_t *tooltip) { return tooltip->n; }},
                        tooltip_);
    };

    double x() const
    {
      return std::visit([](const auto *tooltip) { return tooltip->x; }, tooltip_);
    };

    int xPx() const
    {
      return std::visit([](const auto *tooltip) { return tooltip->x_px; }, tooltip_);
    };

    int yPx() const
    {
      return std::visit([](const auto *tooltip) { return tooltip->y_px; }, tooltip_);
    };

    const char *xLabel() const
    {
      return std::visit([](const auto *tooltip) { return tooltip->x_label; }, tooltip_);
    };

  private:
    std::variant<grm_tooltip_info_t *, grm_accumulated_tooltip_info_t *> tooltip_;
  };

  bool in_listen_mode = false;
  QPixmap pixmap;
  RedrawType redraw_pixmap;
  grm_args_t *args_;
  MouseState mouse_state;
  QRubberBand *rubber_band;
  std::vector<TooltipWrapper> tooltips;
  QTextDocument label;
  BoundingLogic *bounding_logic;
  std::vector<BoundingObject> clicked, referenced_elements;
  BoundingObject *current_selection, *mouse_move_selection, *selected_parent;
  std::list<std::unique_ptr<BoundingObject>> current_selections;
  TreeWidget *tree_widget;
  AddElementWidget *add_element_widget;
  int amount_scrolled;
  bool enable_editor;
  Receiver *receiver;
  std::shared_ptr<GRM::Document> schema_tree;
  bool tree_update = true;
  QSize size_hint;
  QStringList check_box_attr, combo_box_attr, color_ind_attr, color_rgb_attr, slider_attr;
  TableWidget *table_widget;
  EditElementWidget *edit_element_widget;
  PreviewTextWidget *preview_text_widget;
  ColorPickerRGB *color_picker_rgb;
  SelectionListWidget *selection_list_widget;
  IconBarWidget *icon_bar_widget;
  bool hide_grid_bbox = true;
  bool enable_advanced_editor = false;
  int add_pos_x, add_pos_y;

  QAction *marginal_heatmap_all_act, *marginal_heatmap_line_act;
  QAction *sum_act, *max_act;
  QAction *line_act, *scatter_act;
  QAction *volume_act, *isosurface_act;
  QAction *heatmap_act, *surface_act, *wireframe_act, *contour_act, *imshow_act, *contourf_act;
  QAction *line3_act, *trisurf_act, *tricont_act, *scatter3_act;
  QAction *barplot_act, *stairs_act, *stem_act;
  QAction *shade_act, *hexbin_act;
  QAction *polar_line_act, *polar_scatter_act;
  QAction *pdf_act, *png_act, *jpeg_act, *svg_act;
  QAction *show_container_action, *save_file_action, *load_file_action, *editor_action, *add_element_action;
  QAction *moveable_mode_act, *selectable_grid_act;
  QAction *show_context_action, *add_context_action, *generate_linear_context_action, *add_grplot_data_context;
  QAction *hide_algo_menu_act, *show_algo_menu_act, *hide_marginal_sub_menu_act, *show_marginal_sub_menu_act,
      *hide_orientation_sub_menu_act, *show_orientation_sub_menu_act, *hide_aspect_ratio_sub_menu_act,
      *show_aspect_ratio_sub_menu_act, *hide_location_sub_menu_act, *show_location_sub_menu_act, *add_seperator_act,
      *undo_action, *redo_action, *advanced_editor_act, *hide_lim_sub_menu_act, *show_lim_sub_menu_act,
      *hide_log_sub_menu_act, *show_log_sub_menu_act, *hide_flip_sub_menu_act, *show_flip_sub_menu_act,
      *hide_plot_type_sub_menu_act, *show_plot_type_sub_menu_act;
  QAction *x_flip_act, *y_flip_act, *z_flip_act, *theta_flip_act;
  QAction *x_log_act, *y_log_act, *z_log_act, *r_log_act;
  QAction *use_gr3_act, *polar_with_pan_act, *keep_window_act, *colormap_act, *text_color_ind_act;
  QAction *keep_aspect_ratio_act, *only_square_aspect_ratio_act;
  QAction *vertical_orientation_act, *horizontal_orientation_act;
  QAction *legend_act, *colorbar_act, *left_axis_act, *right_axis_act, *bottom_axis_act, *top_axis_act,
      *twin_x_axis_act, *twin_y_axis_act;
  QCursor *csr;
  QMenu *add_overlay_menu;
  QAction *add_text_act, *add_image_act;
  QAction *show_edit_element_act, *show_tree_widget_act, *show_table_widget_act, *show_preview_text_act,
      *show_selection_list_widget_act, *show_icon_bar_widget_act, *show_add_element_act;
  QAction *hide_edit_element_act, *hide_tree_widget_act, *hide_table_widget_act, *hide_preview_text_act,
      *hide_selection_list_widget_act, *hide_icon_bar_widget_act, *hide_add_element_act;
  QAction *x_lim_act, *y_lim_act, *z_lim_act;
  QAction *icon_bar_act;
  bool overlay_element_edit = false, called_by_location_change = false;
  QRadioButton *last_checked_radio_button = nullptr;

  void resetPixmap();
  void highlightCurrentSelection(QPainter &painter);
  void extractBoundingBoxesFromGRM(QPainter &painter);
  void showEvent(QShowEvent *) override;
  void closeEvent(QCloseEvent *event) override;
  QSize sizeHint() const override;
  void sizeCallback(const grm_event_t *);
  void cmdCallback(const grm_request_event_t *);
  void adjustPlotTypeMenu(const std::shared_ptr<GRM::Element> &plot_parent);
  void hidePlotTypeMenuElements();
  void cursorHandler(int x, int y);
  void overlayElementEdit();
  void colorIndexHelper(const std::shared_ptr<GRM::Element> &plot_elem, int current_index, QGridLayout *grid_layout,
                        QList<QRadioButton *> *radio_buttons, int max_index, int index_name_start);
};

#endif /* ifndef GRPLOT_WIDGET_H_INCLUDED */
