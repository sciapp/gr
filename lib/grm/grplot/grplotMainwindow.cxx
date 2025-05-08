#include <sstream>
#include <iostream>
#include <QTimer>
#include "grplotMainwindow.hxx"

const unsigned int MAXPATHLEN = 1024;

GRPlotMainWindow::GRPlotMainWindow(int argc, char **argv, int width, int height, bool listen_mode, bool test_mode,
                                   QString test_commands_file_path, bool help_mode)
    : QMainWindow(), grplot_widget_(nullptr)
{
  if (QPixmap(":/colorbars/uniform.png").isNull())
    {
      std::cerr << "Could not load example colorbar." << std::endl;
      exit(1);
    }
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
      type = new QMenu("&Plot type");
      algo = new QMenu("&Algorithm");
      modi_menu = new QMenu("&Modi");
      marginal_sub_menu = type->addMenu("&Marginal-heatmap");

      export_menu->addAction(grplot_widget_->getPdfAct());
      export_menu->addAction(grplot_widget_->getPngAct());
      export_menu->addAction(grplot_widget_->getJpegAct());
      export_menu->addAction(grplot_widget_->getSvgAct());
      type->addAction(grplot_widget_->getLine3Act());
      type->addAction(grplot_widget_->getTrisurfAct());
      type->addAction(grplot_widget_->getTricontAct());
      type->addAction(grplot_widget_->getScatter3Act());
      type->addAction(grplot_widget_->getHistogramAct());
      type->addAction(grplot_widget_->getBarplotAct());
      type->addAction(grplot_widget_->getStairsAct());
      type->addAction(grplot_widget_->getStemAct());
      type->addAction(grplot_widget_->getShadeAct());
      type->addAction(grplot_widget_->getHexbinAct());
      type->addAction(grplot_widget_->getPolarLineAct());
      type->addAction(grplot_widget_->getPolarScatterAct());
      type->addAction(grplot_widget_->getLineAct());
      type->addAction(grplot_widget_->getScatterAct());
      type->addAction(grplot_widget_->getVolumeAct());
      type->addAction(grplot_widget_->getIsosurfaceAct());
      type->addAction(grplot_widget_->getHeatmapAct());
      type->addAction(grplot_widget_->getSurfaceAct());
      type->addAction(grplot_widget_->getWireframeAct());
      type->addAction(grplot_widget_->getContourAct());
      type->addAction(grplot_widget_->getImshowAct());
      type->addAction(grplot_widget_->getContourfAct());
      algo->addAction(grplot_widget_->getSumAct());
      algo->addAction(grplot_widget_->getMaxAct());
      marginal_sub_menu->addAction(grplot_widget_->getMarginalHeatmapAllAct());
      marginal_sub_menu->addAction(grplot_widget_->getMarginalHeatmapLineAct());
      modi_menu->addAction(grplot_widget_->getMovableModeAct());

      marginal_sub_menu->menuAction()->setVisible(false);
      algo->menuAction()->setVisible(false);

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

      menu->addMenu(file_menu);
      menu->addMenu(type);
      menu->addMenu(algo);
      menu->addMenu(modi_menu);

      if (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "edit") == 0)
        {
          editor_menu = new QMenu(tr("&Editor"));
          configuration_menu = editor_menu->addMenu(tr("&Show"));
          context_menu = new QMenu("&Data");
          add_context_data = new QMenu("&Add Data-Context");

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

          configuration_menu->menuAction()->setVisible(false);
          menu->addMenu(editor_menu);
          menu->addMenu(context_menu);
          context_menu->addMenu(add_context_data);
        }
    }
}

GRPlotMainWindow::~GRPlotMainWindow() = default;

void GRPlotMainWindow::hideAlgoMenu()
{
  algo->menuAction()->setVisible(false);
}

void GRPlotMainWindow::showAlgoMenu()
{
  algo->menuAction()->setVisible(true);
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

void GRPlotMainWindow::addSeperator()
{
  type->addSeparator();
}
