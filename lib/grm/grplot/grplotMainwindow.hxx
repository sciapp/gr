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
                            bool test_mode = false, QString test_commands_file_path = "", bool help_mode = false);
  ~GRPlotMainWindow() override;

private:
  GRPlotWidget *grplot_widget_;
  QMenuBar *menu;
  QMenu *type, *algo, *export_menu, *editor_menu, *modi_menu;
  QMenu *file_menu, *configuration_menu, *context_menu, *add_context_data;
  QMenu *marginal_sub_menu;

private slots:
  void hideAlgoMenu();
  void showAlgoMenu();
  void hideMarginalSubMenu();
  void showMarginalSubMenu();
  void hideConfigurationMenu();
  void showConfigurationMenu();
  void addSeperator();
};

#endif /* ifndef GRPLOT_MAIN_WINDOW_H_INCLUDED */
