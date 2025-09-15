#include "IconBarWidget.hxx"

const int ICON_SIZE = 20;
const int BUTTON_WIDTH = 30;
const int BUTTON_HEIGHT = 23;

IconBarWidget::IconBarWidget(GRPlotWidget *widget, QWidget *parent) : QWidget(parent)
{
  grplot_widget = widget;

  h_box_layout = new QHBoxLayout(this);

  type_tool_button = new QToolButton(this);
  std::string name = (grplot_widget->isDarkMode() ? "kind_dark" : "kind");
  auto kind = QPixmap((":/icons/" + name + ".png").c_str());
  type_tool_button->setIcon(kind);
  type_tool_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  type_tool_button->setContentsMargins(0, 0, 0, 0);
  type_tool_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  type_tool_button->setPopupMode(QToolButton::InstantPopup);
  type_tool_button->setStyleSheet(" QToolButton {border:none;} :hover {background: lightgray;} "
                                  "QToolButton::menu-indicator { width:5px; height:3px; padding:2px;}");
  type_tool_button->setToolTip("Series Kind");
  type_tool_button->setMinimumWidth(0);

  type_sub_menu = new QMenu(this);
  type_sub_menu->addAction(grplot_widget->getLine3Act());
  type_sub_menu->addAction(grplot_widget->getTrisurfAct());
  type_sub_menu->addAction(grplot_widget->getTricontAct());
  type_sub_menu->addAction(grplot_widget->getScatter3Act());
  type_sub_menu->addAction(grplot_widget->getBarplotAct());
  type_sub_menu->addAction(grplot_widget->getStairsAct());
  type_sub_menu->addAction(grplot_widget->getStemAct());
  type_sub_menu->addAction(grplot_widget->getShadeAct());
  type_sub_menu->addAction(grplot_widget->getHexbinAct());
  type_sub_menu->addAction(grplot_widget->getPolarLineAct());
  type_sub_menu->addAction(grplot_widget->getPolarScatterAct());
  type_sub_menu->addAction(grplot_widget->getLineAct());
  type_sub_menu->addAction(grplot_widget->getScatterAct());
  type_sub_menu->addAction(grplot_widget->getVolumeAct());
  type_sub_menu->addAction(grplot_widget->getIsosurfaceAct());
  type_sub_menu->addAction(grplot_widget->getHeatmapAct());
  type_sub_menu->addAction(grplot_widget->getSurfaceAct());
  type_sub_menu->addAction(grplot_widget->getWireframeAct());
  type_sub_menu->addAction(grplot_widget->getContourAct());
  type_sub_menu->addAction(grplot_widget->getImshowAct());
  type_sub_menu->addAction(grplot_widget->getContourfAct());
  marginal_sub_menu = type_sub_menu->addMenu("&Marginal Heatmap");
  marginal_sub_menu->addAction(grplot_widget->getMarginalHeatmapAllAct());
  marginal_sub_menu->addAction(grplot_widget->getMarginalHeatmapLineAct());
  type_tool_button->setMenu(type_sub_menu);

  algo_tool_button = new QToolButton(this);
  name = (grplot_widget->isDarkMode() ? "algorithm_dark" : "algorithm");
  auto algo = QPixmap((":/icons/" + name + ".png").c_str());
  algo_tool_button->setIcon(algo);
  algo_tool_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  algo_tool_button->setContentsMargins(0, 0, 0, 0);
  algo_tool_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  algo_tool_button->setPopupMode(QToolButton::InstantPopup);
  algo_tool_button->setStyleSheet(" QToolButton {border:none;} :hover {background: lightgray;} "
                                  "QToolButton::menu-indicator { width:5px; height:3px; padding:2px;}");
  algo_tool_button->setToolTip("Marginal Heatmap Algorithm");
  algo_tool_button->setMinimumWidth(0);

  algo_sub_menu = new QMenu(this);
  algo_sub_menu->addAction(grplot_widget->getSumAct());
  algo_sub_menu->addAction(grplot_widget->getMaxAct());
  algo_tool_button->setMenu(algo_sub_menu);

  log_tool_button = new QToolButton(this);
  name = (grplot_widget->isDarkMode() ? "log_dark" : "log");
  auto log = QPixmap((":/icons/" + name + ".png").c_str());
  log_tool_button->setIcon(log);
  log_tool_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  log_tool_button->setContentsMargins(0, 0, 0, 0);
  log_tool_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  log_tool_button->setPopupMode(QToolButton::InstantPopup);
  log_tool_button->setStyleSheet(" QToolButton {border:none;} :hover {background: lightgray;} "
                                 "QToolButton::menu-indicator { width:5px; height:3px; padding:2px;}");
  log_tool_button->setToolTip("Logarithm");
  log_tool_button->setMinimumWidth(0);

  log_sub_menu = new QMenu(this);
  log_sub_menu->addAction(grplot_widget->getXLogAct());
  log_sub_menu->addAction(grplot_widget->getYLogAct());
  log_sub_menu->addAction(grplot_widget->getZLogAct());
  log_sub_menu->addAction(grplot_widget->getRLogAct());
  log_tool_button->setMenu(log_sub_menu);

  flip_tool_button = new QToolButton(this);
  name = (grplot_widget->isDarkMode() ? "flip_dark" : "flip");
  auto flip = QPixmap((":/icons/" + name + ".png").c_str());
  flip_tool_button->setIcon(flip);
  flip_tool_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  flip_tool_button->setContentsMargins(0, 0, 0, 0);
  flip_tool_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  flip_tool_button->setPopupMode(QToolButton::InstantPopup);
  flip_tool_button->setStyleSheet(" QToolButton {border:none;} :hover {background: lightgray;} "
                                  "QToolButton::menu-indicator { width:5px; height:3px; padding:2px;}");
  flip_tool_button->setToolTip("Flip");
  flip_tool_button->setMinimumWidth(0);

  flip_sub_menu = new QMenu(this);
  flip_sub_menu->addAction(grplot_widget->getXFlipAct());
  flip_sub_menu->addAction(grplot_widget->getYFlipAct());
  flip_sub_menu->addAction(grplot_widget->getZFlipAct());
  flip_sub_menu->addAction(grplot_widget->getThetaFlipAct());
  flip_tool_button->setMenu(flip_sub_menu);

  lim_tool_button = new QToolButton(this);
  name = (grplot_widget->isDarkMode() ? "lim_dark" : "lim");
  auto lim = QPixmap((":/icons/" + name + ".png").c_str());
  lim_tool_button->setIcon(lim);
  lim_tool_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  lim_tool_button->setContentsMargins(0, 0, 0, 0);
  lim_tool_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  lim_tool_button->setPopupMode(QToolButton::InstantPopup);
  lim_tool_button->setStyleSheet(" QToolButton {border:none;} :hover {background: lightgray;} "
                                 "QToolButton::menu-indicator { width:5px; height:3px; padding:2px;}");
  lim_tool_button->setToolTip("Plot Limits");
  lim_tool_button->setMinimumWidth(0);

  lim_sub_menu = new QMenu(this);
  lim_sub_menu->addAction(grplot_widget->getXLimAct());
  lim_sub_menu->addAction(grplot_widget->getYLimAct());
  lim_sub_menu->addAction(grplot_widget->getZLimAct());
  lim_tool_button->setMenu(lim_sub_menu);

  orientation_tool_button = new QToolButton(this);
  name = (grplot_widget->isDarkMode() ? "orientation_dark" : "orientation");
  auto orientation = QPixmap((":/icons/" + name + ".png").c_str());
  orientation_tool_button->setIcon(orientation);
  orientation_tool_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  orientation_tool_button->setContentsMargins(0, 0, 0, 0);
  orientation_tool_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  orientation_tool_button->setPopupMode(QToolButton::InstantPopup);
  orientation_tool_button->setStyleSheet(" QToolButton {border:none;} :hover {background: lightgray;} "
                                         "QToolButton::menu-indicator { width:5px; height:3px; padding:2px;}");
  orientation_tool_button->setToolTip("Orientation");
  orientation_tool_button->setMinimumWidth(0);

  orientation_sub_menu = new QMenu(this);
  orientation_sub_menu->addAction(grplot_widget->getVerticalOrientationAct());
  orientation_sub_menu->addAction(grplot_widget->getHorizontalOrientationAct());
  orientation_sub_menu->addAction(grplot_widget->getKeepWindowAct());
  orientation_tool_button->setMenu(orientation_sub_menu);

  aspect_ratio_tool_button = new QToolButton(this);
  name = (grplot_widget->isDarkMode() ? "aspect_ratio_dark" : "aspect_ratio");
  auto aspect_ratio = QPixmap((":/icons/" + name + ".png").c_str());
  aspect_ratio_tool_button->setIcon(aspect_ratio);
  aspect_ratio_tool_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  aspect_ratio_tool_button->setContentsMargins(0, 0, 0, 0);
  aspect_ratio_tool_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  aspect_ratio_tool_button->setPopupMode(QToolButton::InstantPopup);
  aspect_ratio_tool_button->setStyleSheet(" QToolButton {border:none;} :hover {background: lightgray;} "
                                          "QToolButton::menu-indicator { width:5px; height:3px; padding:2px;}");
  aspect_ratio_tool_button->setToolTip("Aspect Ratio");
  aspect_ratio_tool_button->setMinimumWidth(0);

  aspect_ratio_sub_menu = new QMenu(this);
  aspect_ratio_sub_menu->addAction(grplot_widget->getKeepAspectRatioAct());
  aspect_ratio_sub_menu->addAction(grplot_widget->getOnlySquareAspectRatioAct());
  aspect_ratio_tool_button->setMenu(aspect_ratio_sub_menu);

  if (!getenv("GRDISPLAY") || (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "view") != 0))
    {
      location_tool_button = new QToolButton(this);
      name = (grplot_widget->isDarkMode() ? "location_dark" : "location");
      auto location = QPixmap((":/icons/" + name + ".png").c_str());
      location_tool_button->setIcon(location);
      location_tool_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
      location_tool_button->setContentsMargins(0, 0, 0, 0);
      location_tool_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
      location_tool_button->setPopupMode(QToolButton::InstantPopup);
      location_tool_button->setStyleSheet(" QToolButton {border:none;} :hover {background: lightgray;} "
                                          "QToolButton::menu-indicator { width:5px; height:3px; padding:2px;}");
      location_tool_button->setToolTip("Location");
      location_tool_button->setMinimumWidth(0);

      location_sub_menu = new QMenu(this);
      location_sub_menu->addAction(grplot_widget->getLegendAct());
      location_sub_menu->addAction(grplot_widget->getColorbarAct());
      location_sub_menu->addAction(grplot_widget->getLeftAxisAct());
      location_sub_menu->addAction(grplot_widget->getRightAxisAct());
      location_sub_menu->addAction(grplot_widget->getBottomAxisAct());
      location_sub_menu->addAction(grplot_widget->getTopAxisAct());
      location_sub_menu->addAction(grplot_widget->getTwinXAxisAct());
      location_sub_menu->addAction(grplot_widget->getTwinYAxisAct());
      location_tool_button->setMenu(location_sub_menu);
      location_sub_menu->menuAction()->setVisible(false);
      location_tool_button->setEnabled(false);

      connect(grplot_widget->getHideLocationSubMenuAct(), &QAction::triggered, this,
              &IconBarWidget::hideLocationSubMenu);
      connect(grplot_widget->getShowLocationSubMenuAct(), &QAction::triggered, this,
              &IconBarWidget::showLocationSubMenu);
    }

  use_gr3_tool_button = new QToolButton(this);
  use_gr3_tool_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  use_gr3_tool_button->setContentsMargins(0, 0, 0, 0);
  use_gr3_tool_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  use_gr3_tool_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
  use_gr3_tool_button->setStyleSheet("QToolButton {border:none;} :hover {background: lightgray;}");
  use_gr3_tool_button->setToolTip("GR3 or GR variant");
  use_gr3_tool_button->setMinimumWidth(0);

  auto use_gr3_act = grplot_widget->getUseGR3Act();
  use_gr3_tool_button->setDefaultAction(use_gr3_act);
  use_gr3_act->setText("");
  name = (grplot_widget->isDarkMode() ? "use_gr3_dark" : "use_gr3");
  auto use_gr3 = QPixmap((":/icons/" + name + ".png").c_str());
  use_gr3_act->setIcon(use_gr3);

  polar_with_pan_tool_button = new QToolButton(this);
  polar_with_pan_tool_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  polar_with_pan_tool_button->setContentsMargins(0, 0, 0, 0);
  polar_with_pan_tool_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  polar_with_pan_tool_button->setStyleSheet("QToolButton {border:none;} :hover {background: lightgray;}");
  polar_with_pan_tool_button->setToolTip("Polar With Pan");
  polar_with_pan_tool_button->setMinimumWidth(0);

  auto polar_with_pan_act = grplot_widget->getPolarWithPanAct();
  polar_with_pan_tool_button->setDefaultAction(polar_with_pan_act);
  polar_with_pan_act->setText("");
  name = (grplot_widget->isDarkMode() ? "polar_with_pan_dark" : "polar_with_pan");
  auto polar_with_pan = QPixmap((":/icons/" + name + ".png").c_str());
  polar_with_pan_act->setIcon(polar_with_pan);

  colormap_tool_button = new QToolButton(this);
  colormap_tool_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  colormap_tool_button->setContentsMargins(0, 0, 0, 0);
  colormap_tool_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  colormap_tool_button->setStyleSheet("QToolButton {border:none;} :hover {background: lightgray;}");
  colormap_tool_button->setToolTip("Colormap");
  colormap_tool_button->setMinimumWidth(0);

  auto colormap_act = grplot_widget->getColormapAct();
  colormap_tool_button->setDefaultAction(colormap_act);

  text_color_ind_button = new QToolButton(this);
  text_color_ind_button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  text_color_ind_button->setContentsMargins(0, 0, 0, 0);
  text_color_ind_button->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  text_color_ind_button->setStyleSheet("QToolButton {border:none;} :hover {background: lightgray;}");
  text_color_ind_button->setToolTip("Text Color Index");
  text_color_ind_button->setMinimumWidth(0);

  auto text_color_ind_act = grplot_widget->getTextColorIndAct();
  text_color_ind_button->setDefaultAction(text_color_ind_act);
  name = (grplot_widget->isDarkMode() ? "text_color_ind_dark" : "text_color_ind");
  auto text_color_ind = QPixmap((":/icons/" + name + ".png").c_str());
  text_color_ind_act->setIcon(text_color_ind);

  marginal_sub_menu->menuAction()->setVisible(false);
  algo_sub_menu->menuAction()->setVisible(false);
  algo_tool_button->setEnabled(false);
  orientation_sub_menu->menuAction()->setVisible(false);
  orientation_tool_button->setEnabled(false);
  if (!grplot_widget->getColormapAct()->isVisible()) colormap_tool_button->setVisible(false);

  connect(grplot_widget->getHideAlgoMenuAct(), &QAction::triggered, this, &IconBarWidget::hideAlgoMenu);
  connect(grplot_widget->getShowAlgoMenuAct(), &QAction::triggered, this, &IconBarWidget::showAlgoMenu);
  connect(grplot_widget->getHideMarginalSubMenuAct(), &QAction::triggered, this, &IconBarWidget::hideMarginalSubMenu);
  connect(grplot_widget->getShowMarginalSubMenuAct(), &QAction::triggered, this, &IconBarWidget::showMarginalSubMenu);
  connect(grplot_widget->getAddSeperatorAct(), &QAction::triggered, this, &IconBarWidget::addSeperator);
  connect(grplot_widget->getHideOrientationSubMenuAct(), &QAction::triggered, this,
          &IconBarWidget::hideOrientationSubMenu);
  connect(grplot_widget->getShowOrientationSubMenuAct(), &QAction::triggered, this,
          &IconBarWidget::showOrientationSubMenu);
  connect(grplot_widget->getHideAspectRatioSubMenuAct(), &QAction::triggered, this,
          &IconBarWidget::hideAspectRatioSubMenu);
  connect(grplot_widget->getShowAspectRatioSubMenuAct(), &QAction::triggered, this,
          &IconBarWidget::showAspectRatioSubMenu);
  connect(grplot_widget->getHideLimSubMenuAct(), &QAction::triggered, this, &IconBarWidget::hideLimSubMenu);
  connect(grplot_widget->getShowLimSubMenuAct(), &QAction::triggered, this, &IconBarWidget::showLimSubMenu);

  h_box_layout->addWidget(type_tool_button);
  h_box_layout->addWidget(algo_tool_button);
  h_box_layout->addWidget(log_tool_button);
  h_box_layout->addWidget(flip_tool_button);
  h_box_layout->addWidget(lim_tool_button);
  h_box_layout->addWidget(orientation_tool_button);
  h_box_layout->addWidget(aspect_ratio_tool_button);
  if (!getenv("GRDISPLAY") || (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "view") != 0))
    h_box_layout->addWidget(location_tool_button);
  h_box_layout->addWidget(use_gr3_tool_button);
  h_box_layout->addWidget(polar_with_pan_tool_button);
  h_box_layout->addWidget(colormap_tool_button);
  h_box_layout->addWidget(text_color_ind_button);

  h_box_layout->setContentsMargins(0, 0, 0, 0);
  h_box_layout->setAlignment(Qt::AlignLeft);
  h_box_layout->setSpacing(1);

  this->setLayout(h_box_layout);
  this->setContentsMargins(0, 0, 0, 0);
}

void IconBarWidget::hideAlgoMenu()
{
  algo_sub_menu->menuAction()->setVisible(false);
  algo_tool_button->setEnabled(false);
}

void IconBarWidget::showAlgoMenu()
{
  algo_sub_menu->menuAction()->setVisible(true);
  algo_tool_button->setEnabled(true);
}

void IconBarWidget::hideMarginalSubMenu()
{
  marginal_sub_menu->menuAction()->setVisible(false);
}

void IconBarWidget::showMarginalSubMenu()
{
  marginal_sub_menu->menuAction()->setVisible(true);
}

void IconBarWidget::hideOrientationSubMenu()
{
  orientation_sub_menu->menuAction()->setVisible(false);
  orientation_tool_button->setEnabled(false);
}

void IconBarWidget::showOrientationSubMenu()
{
  orientation_sub_menu->menuAction()->setVisible(true);
  orientation_tool_button->setEnabled(true);
}

void IconBarWidget::hideAspectRatioSubMenu()
{
  aspect_ratio_sub_menu->menuAction()->setVisible(false);
  aspect_ratio_tool_button->setEnabled(false);
}

void IconBarWidget::showAspectRatioSubMenu()
{
  aspect_ratio_sub_menu->menuAction()->setVisible(true);
  aspect_ratio_tool_button->setEnabled(true);
}

void IconBarWidget::hideLocationSubMenu()
{
  location_sub_menu->menuAction()->setVisible(false);
  location_tool_button->setEnabled(false);
}

void IconBarWidget::showLocationSubMenu()
{
  location_sub_menu->menuAction()->setVisible(true);
  location_tool_button->setEnabled(true);
}

void IconBarWidget::hideLimSubMenu()
{
  lim_sub_menu->menuAction()->setVisible(false);
  lim_tool_button->setEnabled(false);
}

void IconBarWidget::showLimSubMenu()
{
  lim_sub_menu->menuAction()->setVisible(true);
  lim_tool_button->setEnabled(true);
}

void IconBarWidget::addSeperator()
{
  type_sub_menu->addSeparator();
}
