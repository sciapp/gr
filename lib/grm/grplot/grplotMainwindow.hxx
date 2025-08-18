#ifndef GRPLOT_MAIN_WINDOW_H_INCLUDED
#define GRPLOT_MAIN_WINDOW_H_INCLUDED
#include "grplotWidget.hxx"

#include <QMainWindow>
#include <QTextBrowser>
#include <fstream>

class GRPlotMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit GRPlotMainWindow(int argc, char **argv, int width, int height, bool listen_mode = false,
                            int listen_port = 8002, bool test_mode = false, QString test_commands_file_path = "",
                            bool help_mode = false);
  ~GRPlotMainWindow() override;

  void resizeGRPlotWidget(int width, int height);

protected:
  void keyPressEvent(QKeyEvent *event) override;

private:
  GRPlotWidget *grplot_widget_;
  QMenuBar *menu;
  QMenu *file_menu, *export_menu, *editor_menu, *modi_menu, *options_menu;
  QMenu *context_menu, *add_context_data;
  QMenu *type_sub_menu, *marginal_sub_menu, *algo_sub_menu, *log_sub_menu, *flip_sub_menu, *orientation_sub_menu,
      *aspect_ratio_sub_menu, *location_sub_menu, *lim_sub_menu;
  QLineEdit *find_line_edit;
  QTextBrowser *message;
  QDockWidget *edit_element_dock_widget, *tree_dock_widget, *table_dock_widget, *text_preview_dock_widget,
      *selection_list_dock_widget;
  bool help_mode = false;

  void center();
private slots:
  void hideAlgoMenu();
  void showAlgoMenu();
  void hideMarginalSubMenu();
  void showMarginalSubMenu();
  void hideOrientationSubMenu();
  void showOrientationSubMenu();
  void hideAspectRatioSubMenu();
  void showAspectRatioSubMenu();
  void hideLocationSubMenu();
  void showLocationSubMenu();
  void addSeperator();
  void findButtonClickedSlot();
  void showEditElementDockSlot();
  void showTreeWidgetDockSlot();
  void showTableWidgetDockSlot();
  void showTextPreviewDockSlot();
  void showSelectionListDockSlot();
  void hideEditElementDockSlot();
  void hideTreeWidgetDockSlot();
  void hideTableWidgetDockSlot();
  void hideTextPreviewDockSlot();
  void hideSelectionListDockSlot();
  void closeEditElementDockSlot();
  void closeTreeWidgetDockSlot();
  void closeTableWidgetDockSlot();
  void closeTextPreviewDockSlot();
  void closeSelectionListDockSlot();
};

#endif /* ifndef GRPLOT_MAIN_WINDOW_H_INCLUDED */
