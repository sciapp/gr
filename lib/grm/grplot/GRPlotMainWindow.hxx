#ifndef GRPLOT_MAIN_WINDOW_H_INCLUDED
#define GRPLOT_MAIN_WINDOW_H_INCLUDED
#include "GRPlotWidget.hxx"

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
  void closeEvent(QCloseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private:
  GRPlotWidget *grplot_widget_;
  QMenuBar *menu;
  QMenu *file_menu, *export_menu, *editor_menu, *modi_menu, *help_menu;
  QMenu *context_menu, *add_context_data;
  QDockWidget *edit_element_dock_widget, *tree_dock_widget, *table_dock_widget, *text_preview_dock_widget,
      *selection_list_dock_widget, *icon_bar_dock_widget, *add_element_dock_widget, *progress_bar_dock_widget;

  void center();
  void keepInDisplay();
private slots:
  void showEditElementDockSlot();
  void showTreeWidgetDockSlot();
  void showTableWidgetDockSlot();
  void showTextPreviewDockSlot();
  void showSelectionListDockSlot();
  void showIconBarDockSlot();
  void showAddElementDockSlot();
  void showProgressBarDockSlot();
  void hideEditElementDockSlot();
  void hideTreeWidgetDockSlot();
  void hideTableWidgetDockSlot();
  void hideTextPreviewDockSlot();
  void hideSelectionListDockSlot();
  void hideIconBarDockSlot();
  void hideAddElementDockSlot();
  void hideProgressBarDockSlot();
  void closeEditElementDockSlot();
  void closeTreeWidgetDockSlot();
  void closeTableWidgetDockSlot();
  void closeTextPreviewDockSlot();
  void closeSelectionListDockSlot();
  void closeIconBarDockSlot();
  void closeAddElementDockSlot();
  void closeProgressBarDockSlot();
  void updateEditElementDockTitleSlot();
  void showHelpDialogSlot(std::optional<std::string> kind = std::nullopt);
};

#endif /* ifndef GRPLOT_MAIN_WINDOW_H_INCLUDED */
