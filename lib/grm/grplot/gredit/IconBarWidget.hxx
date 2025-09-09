#ifndef ICONBARWIDGET_HXX
#define ICONBARWIDGET_HXX


#include <QWidget>
#include <QToolButton>
class IconBarWidget;
#include "../grplotWidget.hxx"

class IconBarWidget : public QWidget
{
  Q_OBJECT
public:
  explicit IconBarWidget(GRPlotWidget *widget, QWidget *parent = nullptr);

private:
  GRPlotWidget *grplot_widget;
  QToolButton *type_tool_button, *algo_tool_button, *log_tool_button, *flip_tool_button, *orientation_tool_button,
      *aspect_ratio_tool_button, *location_tool_button, *lim_tool_button, *accelerate_tool_button,
      *polar_with_pan_tool_button, *colormap_tool_button;
  QMenu *type_sub_menu, *marginal_sub_menu, *algo_sub_menu, *log_sub_menu, *flip_sub_menu, *orientation_sub_menu,
      *aspect_ratio_sub_menu, *location_sub_menu, *lim_sub_menu;
  QHBoxLayout *h_box_layout;

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
};


#endif // ICONBARWIDGET_HXX
