#include <sstream>
#include <iostream>
#include <QTimer>
#include "grplotMainwindow.hxx"

const unsigned int MAXPATHLEN = 1024;

GRPlotMainWindow::GRPlotMainWindow(int argc, char **argv, int width, int height, bool listen_mode, bool test_mode,
                                   QString test_commands_file_path, bool help_mode)
    : QMainWindow(), grplot_widget_(nullptr)
{
  bool hide_colormap = false;
  // there is no colormap or each colormap gets created -> check for 1 colormap is enough
  if (QPixmap(":preview_images/colormaps/afmhot").isNull()) hide_colormap = true;

  if (help_mode)
    {
      auto *w = new QWidget(this);
      std::string kind;
      static char path[MAXPATHLEN];
      std::snprintf(path, MAXPATHLEN, "%s/lib", GRDIR);

      auto *message = new QTextBrowser(w);
      message->setSearchPaths(QStringList(path));
      message->setSource(QUrl("../share/doc/grplot/grplot.man.md"));
      message->setReadOnly(true);
      message->setOpenExternalLinks(true);
      message->setAlignment(Qt::AlignTop);
      if (argc >= 3)
        {
          kind = argv[2];
          std::transform(kind.begin(), kind.end(), kind.begin(), ::toupper);
          if (!message->find(QString(kind.c_str()), QTextDocument::FindCaseSensitively))
            fprintf(stderr, "No plot type with the name %s was found.\n", kind.c_str());
        }
      setCentralWidget(message);
      resize(width, height);
    }
  else
    {
      grplot_widget_ = new GRPlotWidget(this, argc, argv, listen_mode, test_mode, test_commands_file_path);
      setCentralWidget(grplot_widget_);
      grplot_widget_->resize(width, height);
      grplot_widget_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

  setWindowTitle("GR Plot");
  if (!listen_mode)
    {
      if (grplot_widget_)
        {
          grplot_widget_->setMinimumSize(width, height);
          adjustSize();
          grplot_widget_->setMinimumSize(0, 0);
        }
      else
        {
          resize(width, height);
        }
    }
  if (test_mode && grplot_widget_->getTestCommandsStream())
    {
      QTimer::singleShot(100, grplot_widget_, &GRPlotWidget::processTestCommandsFile);
    }

  if (grplot_widget_ && !listen_mode && !test_mode && !grplot_widget_->getTestCommandsStream() && !help_mode)
    {
      menu = menuBar();
      file_menu = new QMenu("&File");
      export_menu = file_menu->addMenu("&Export");
      modi_menu = new QMenu("&Modi");
      options_menu = new QMenu("&Options");
      type_sub_menu = options_menu->addMenu("&Plot type");
      marginal_sub_menu = type_sub_menu->addMenu("&Marginal-heatmap");
      algo_sub_menu = options_menu->addMenu("&Algorithm");
      log_sub_menu = options_menu->addMenu("&Logarithm");
      flip_sub_menu = options_menu->addMenu("&Flip");
      orientation_sub_menu = options_menu->addMenu("&Orientation");
      aspect_ratio_sub_menu = options_menu->addMenu("&Aspectratio");

      export_menu->addAction(grplot_widget_->getPdfAct());
      export_menu->addAction(grplot_widget_->getPngAct());
      export_menu->addAction(grplot_widget_->getJpegAct());
      export_menu->addAction(grplot_widget_->getSvgAct());
      type_sub_menu->addAction(grplot_widget_->getLine3Act());
      type_sub_menu->addAction(grplot_widget_->getTrisurfAct());
      type_sub_menu->addAction(grplot_widget_->getTricontAct());
      type_sub_menu->addAction(grplot_widget_->getScatter3Act());
      type_sub_menu->addAction(grplot_widget_->getHistogramAct());
      type_sub_menu->addAction(grplot_widget_->getBarplotAct());
      type_sub_menu->addAction(grplot_widget_->getStairsAct());
      type_sub_menu->addAction(grplot_widget_->getStemAct());
      type_sub_menu->addAction(grplot_widget_->getShadeAct());
      type_sub_menu->addAction(grplot_widget_->getHexbinAct());
      type_sub_menu->addAction(grplot_widget_->getPolarLineAct());
      type_sub_menu->addAction(grplot_widget_->getPolarScatterAct());
      type_sub_menu->addAction(grplot_widget_->getLineAct());
      type_sub_menu->addAction(grplot_widget_->getScatterAct());
      type_sub_menu->addAction(grplot_widget_->getVolumeAct());
      type_sub_menu->addAction(grplot_widget_->getIsosurfaceAct());
      type_sub_menu->addAction(grplot_widget_->getHeatmapAct());
      type_sub_menu->addAction(grplot_widget_->getSurfaceAct());
      type_sub_menu->addAction(grplot_widget_->getWireframeAct());
      type_sub_menu->addAction(grplot_widget_->getContourAct());
      type_sub_menu->addAction(grplot_widget_->getImshowAct());
      type_sub_menu->addAction(grplot_widget_->getContourfAct());
      algo_sub_menu->addAction(grplot_widget_->getSumAct());
      algo_sub_menu->addAction(grplot_widget_->getMaxAct());
      marginal_sub_menu->addAction(grplot_widget_->getMarginalHeatmapAllAct());
      marginal_sub_menu->addAction(grplot_widget_->getMarginalHeatmapLineAct());
      modi_menu->addAction(grplot_widget_->getMovableModeAct());
      log_sub_menu->addAction(grplot_widget_->getXLogAct());
      log_sub_menu->addAction(grplot_widget_->getYLogAct());
      log_sub_menu->addAction(grplot_widget_->getZLogAct());
      flip_sub_menu->addAction(grplot_widget_->getXFlipAct());
      flip_sub_menu->addAction(grplot_widget_->getYFlipAct());
      flip_sub_menu->addAction(grplot_widget_->getZFlipAct());
      flip_sub_menu->addAction(grplot_widget_->getPhiFlipAct());
      options_menu->addAction(grplot_widget_->getAccelerateAct());
      options_menu->addAction(grplot_widget_->getPolarWithPanAct());
      options_menu->addAction(grplot_widget_->getKeepWindowAct());
      orientation_sub_menu->addAction(grplot_widget_->getVerticalOrientationAct());
      orientation_sub_menu->addAction(grplot_widget_->getHorizontalOrientationAct());
      aspect_ratio_sub_menu->addAction(grplot_widget_->getKeepAspectRatioAct());
      aspect_ratio_sub_menu->addAction(grplot_widget_->getOnlyQuadraticAspectRatioAct());
      options_menu->addAction(grplot_widget_->getColormapAct());

      marginal_sub_menu->menuAction()->setVisible(false);
      algo_sub_menu->menuAction()->setVisible(false);
      orientation_sub_menu->setVisible(false);
      if (hide_colormap) grplot_widget_->getColormapAct()->setVisible(false);

      connect(grplot_widget_->getHideAlgoMenuAct(), &QAction::triggered, this, &GRPlotMainWindow::hideAlgoMenu);
      connect(grplot_widget_->getShowAlgoMenuAct(), &QAction::triggered, this, &GRPlotMainWindow::showAlgoMenu);
      connect(grplot_widget_->getHideMarginalSubMenuAct(), &QAction::triggered, this,
              &GRPlotMainWindow::hideMarginalSubMenu);
      connect(grplot_widget_->getShowMarginalSubMenuAct(), &QAction::triggered, this,
              &GRPlotMainWindow::showMarginalSubMenu);
      connect(grplot_widget_->getHideConfigurationMenuAct(), &QAction::triggered, this,
              &GRPlotMainWindow::hideConfigurationMenu);
      connect(grplot_widget_->getShowConfigurationMenuAct(), &QAction::triggered, this,
              &GRPlotMainWindow::showConfigurationMenu);
      connect(grplot_widget_->getAddSeperatorAct(), &QAction::triggered, this, &GRPlotMainWindow::addSeperator);
      connect(grplot_widget_->getHideOrientationSubMenuAct(), &QAction::triggered, this,
              &GRPlotMainWindow::hideOrientationSubMenu);
      connect(grplot_widget_->getShowOrientationSubMenuAct(), &QAction::triggered, this,
              &GRPlotMainWindow::showOrientationSubMenu);
      connect(grplot_widget_->getHideAspectRatioSubMenuAct(), &QAction::triggered, this,
              &GRPlotMainWindow::hideAspectRatioSubMenu);
      connect(grplot_widget_->getShowAspectRatioSubMenuAct(), &QAction::triggered, this,
              &GRPlotMainWindow::showAspectRatioSubMenu);

      menu->addMenu(file_menu);
      menu->addMenu(options_menu);
      menu->addMenu(modi_menu);

      if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
        {
          editor_menu = new QMenu(tr("&Editor"));
          configuration_menu = editor_menu->addMenu(tr("&Show"));
          context_menu = new QMenu("&Data");
          add_context_data = new QMenu("&Add Data-Context");
          location_sub_menu = options_menu->addMenu("&Location");

          editor_menu->addAction(grplot_widget_->getEditorAct());
          file_menu->addAction(grplot_widget_->getSaveFileAct());
          file_menu->addAction(grplot_widget_->getLoadFileAct());
          configuration_menu->addAction(grplot_widget_->getShowContainerAct());
          configuration_menu->addAction(grplot_widget_->getShowBoundingBoxesAct());
          editor_menu->addAction(grplot_widget_->getAddElementAct());
          context_menu->addAction(grplot_widget_->getShowContextAct());
          add_context_data->addAction(grplot_widget_->getAddContextAct());
          add_context_data->addAction(grplot_widget_->getAddGRplotDataContextAct());
          add_context_data->addAction(grplot_widget_->getGenerateLinearContextAct());

          location_sub_menu->addAction(grplot_widget_->getLegendAct());
          location_sub_menu->addAction(grplot_widget_->getColorbarAct());
          location_sub_menu->addAction(grplot_widget_->getLeftAxisAct());
          location_sub_menu->addAction(grplot_widget_->getRightAxisAct());
          location_sub_menu->addAction(grplot_widget_->getBottomAxisAct());
          location_sub_menu->addAction(grplot_widget_->getTopAxisAct());
          location_sub_menu->addAction(grplot_widget_->getTwinXAxisAct());
          location_sub_menu->addAction(grplot_widget_->getTwinYAxisAct());

          location_sub_menu->menuAction()->setVisible(false);
          configuration_menu->menuAction()->setVisible(false);
          menu->addMenu(editor_menu);
          menu->addMenu(context_menu);
          context_menu->addMenu(add_context_data);

          connect(grplot_widget_->getHideLocationSubMenuAct(), &QAction::triggered, this,
                  &GRPlotMainWindow::hideLocationSubMenu);
          connect(grplot_widget_->getShowLocationSubMenuAct(), &QAction::triggered, this,
                  &GRPlotMainWindow::showLocationSubMenu);
        }
    }
}

GRPlotMainWindow::~GRPlotMainWindow() = default;

void GRPlotMainWindow::hideAlgoMenu()
{
  algo_sub_menu->menuAction()->setVisible(false);
}

void GRPlotMainWindow::showAlgoMenu()
{
  algo_sub_menu->menuAction()->setVisible(true);
}

void GRPlotMainWindow::hideMarginalSubMenu()
{
  marginal_sub_menu->menuAction()->setVisible(false);
}

void GRPlotMainWindow::showMarginalSubMenu()
{
  marginal_sub_menu->menuAction()->setVisible(true);
}

void GRPlotMainWindow::hideConfigurationMenu()
{
  configuration_menu->menuAction()->setVisible(false);
}

void GRPlotMainWindow::showConfigurationMenu()
{
  configuration_menu->menuAction()->setVisible(true);
}

void GRPlotMainWindow::hideOrientationSubMenu()
{
  orientation_sub_menu->menuAction()->setVisible(false);
}

void GRPlotMainWindow::showOrientationSubMenu()
{
  orientation_sub_menu->menuAction()->setVisible(true);
}

void GRPlotMainWindow::hideAspectRatioSubMenu()
{
  aspect_ratio_sub_menu->menuAction()->setVisible(false);
}

void GRPlotMainWindow::showAspectRatioSubMenu()
{
  aspect_ratio_sub_menu->menuAction()->setVisible(true);
}

void GRPlotMainWindow::hideLocationSubMenu()
{
  location_sub_menu->menuAction()->setVisible(false);
}

void GRPlotMainWindow::showLocationSubMenu()
{
  location_sub_menu->menuAction()->setVisible(true);
}

void GRPlotMainWindow::addSeperator()
{
  type_sub_menu->addSeparator();
}
