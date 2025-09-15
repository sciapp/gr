#include <sstream>
#include <iostream>
#include <QTimer>
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
#include <QIcon>
#else
#include <QScreen>
#include <QGuiApplication>
#endif
#include "grplotMainwindow.hxx"
#include "grplotDockWidget.hxx"

const unsigned int MAXPATHLEN = 1024;
const int LEFT_AREA_WIDTH = 300;
const int RIGHT_AREA_WIDTH = 250;

GRPlotMainWindow::GRPlotMainWindow(int argc, char **argv, int width, int height, bool listen_mode, int listen_port,
                                   bool test_mode, QString test_commands_file_path, bool help_mode)
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

      this->help_mode = help_mode;
      find_line_edit = new QLineEdit(w);
      find_line_edit->setPlaceholderText("Search in document");
      auto button = new QPushButton("Find", w);
      QObject::connect(button, SIGNAL(clicked()), this, SLOT(findButtonClickedSlot()));
      auto form = new QGridLayout;
      form->addWidget(find_line_edit, 0, 0);
      form->addWidget(button, 0, 1);

      message = new QTextBrowser(w);
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
      form->addWidget(message, 1, 0, 1, 2);
      w->setLayout(form);
      setCentralWidget(w);
      resize(width, height);
    }
  else
    {
      grplot_widget_ = new GRPlotWidget(this, argc, argv, listen_mode, listen_port, test_mode, test_commands_file_path);
      setCentralWidget(grplot_widget_);
      grplot_widget_->resize(width, height);
      grplot_widget_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

  setWindowTitle("GR Plot");
  if (!listen_mode)
    {
      if (grplot_widget_)
        {
          resizeGRPlotWidget(width, height);
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

  if (grplot_widget_ && !test_mode && !grplot_widget_->getTestCommandsStream() && !help_mode)
    {
      menu = menuBar();
      file_menu = new QMenu("&File");
      export_menu = file_menu->addMenu("&Export");
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
      export_menu->menuAction()->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::CameraPhoto));
      export_menu->menuAction()->setIconVisibleInMenu(true);
#endif
      modi_menu = new QMenu("&Modi");

      export_menu->addAction(grplot_widget_->getPdfAct());
      export_menu->addAction(grplot_widget_->getPngAct());
      export_menu->addAction(grplot_widget_->getJpegAct());
      export_menu->addAction(grplot_widget_->getSvgAct());
      modi_menu->addAction(grplot_widget_->getMovableModeAct());
      if (hide_colormap) grplot_widget_->getColormapAct()->setVisible(false);

      menu->addMenu(file_menu);
      menu->addMenu(modi_menu);

      if (!getenv("GRDISPLAY") || (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "view") != 0))
        {
          editor_menu = new QMenu(tr("&Editor"));
          context_menu = new QMenu("&Data");
          add_context_data = new QMenu("&Add Data-Context");
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
          add_context_data->menuAction()->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ListAdd));
          add_context_data->menuAction()->setIconVisibleInMenu(true);
#endif

          editor_menu->addAction(grplot_widget_->getEditorAct());
          file_menu->addAction(grplot_widget_->getSaveFileAct());
          file_menu->addAction(grplot_widget_->getLoadFileAct());
          editor_menu->addAction(grplot_widget_->getShowContainerAct());
          editor_menu->addAction(grplot_widget_->getAdvancedEditorAct());
          editor_menu->addAction(grplot_widget_->getAddElementAct());
          editor_menu->addAction(grplot_widget_->getUndoAct());
          editor_menu->addAction(grplot_widget_->getRedoAct());
          context_menu->addAction(grplot_widget_->getShowContextAct());
          add_context_data->addAction(grplot_widget_->getAddContextAct());
          add_context_data->addAction(grplot_widget_->getAddGRplotDataContextAct());
          add_context_data->addAction(grplot_widget_->getGenerateLinearContextAct());
          modi_menu->addAction(grplot_widget_->getSelectableGridAct());
          modi_menu->addAction(grplot_widget_->getIconBarAct());

          menu->addMenu(editor_menu);
          menu->addMenu(context_menu);
          context_menu->addMenu(add_context_data);

          edit_element_dock_widget = new GRPlotDockWidget("Edit Element:", RIGHT_AREA_WIDTH, height, this);
          edit_element_dock_widget->setWidget(grplot_widget_->getEditElementWidget());
          edit_element_dock_widget->setAllowedAreas(Qt::RightDockWidgetArea);
          edit_element_dock_widget->hide();
          addDockWidget(Qt::RightDockWidgetArea, edit_element_dock_widget);
          QObject::connect(grplot_widget_->getHideEditElementAct(), SIGNAL(triggered()), this,
                           SLOT(hideEditElementDockSlot()));
          QObject::connect(grplot_widget_->getShowEditElementAct(), SIGNAL(triggered()), this,
                           SLOT(showEditElementDockSlot()));
          QObject::connect(edit_element_dock_widget, SIGNAL(resizeMainWindow()), this,
                           SLOT(closeEditElementDockSlot()));

          tree_dock_widget = new GRPlotDockWidget("Element Tree:", LEFT_AREA_WIDTH, height, this);
          tree_dock_widget->setWidget(grplot_widget_->getTreeWidget());
          tree_dock_widget->setAllowedAreas(Qt::LeftDockWidgetArea);
          tree_dock_widget->hide();
          addDockWidget(Qt::LeftDockWidgetArea, tree_dock_widget);
          QObject::connect(grplot_widget_->getShowTreeWidgetAct(), SIGNAL(triggered()), this,
                           SLOT(showTreeWidgetDockSlot()));
          QObject::connect(grplot_widget_->getHideTreeWidgetAct(), SIGNAL(triggered()), this,
                           SLOT(hideTreeWidgetDockSlot()));
          QObject::connect(tree_dock_widget, SIGNAL(resizeMainWindow()), this, SLOT(closeTreeWidgetDockSlot()));

          table_dock_widget = new GRPlotDockWidget("Data-Context Viewer:", LEFT_AREA_WIDTH, height, this);
          table_dock_widget->setWidget(grplot_widget_->getTableWidget());
          table_dock_widget->setAllowedAreas(Qt::LeftDockWidgetArea);
          table_dock_widget->hide();
          addDockWidget(Qt::LeftDockWidgetArea, table_dock_widget);
          QObject::connect(grplot_widget_->getHideTableWidgetAct(), SIGNAL(triggered()), this,
                           SLOT(hideTableWidgetDockSlot()));
          QObject::connect(grplot_widget_->getShowTableWidgetAct(), SIGNAL(triggered()), this,
                           SLOT(showTableWidgetDockSlot()));
          QObject::connect(table_dock_widget, SIGNAL(resizeMainWindow()), this, SLOT(closeTableWidgetDockSlot()));

          text_preview_dock_widget = new GRPlotDockWidget("Text Preview:", RIGHT_AREA_WIDTH, 30, this);
          text_preview_dock_widget->setWidget(grplot_widget_->getTextPreviewWidget());
          text_preview_dock_widget->setAllowedAreas(Qt::RightDockWidgetArea);
          text_preview_dock_widget->hide();
          addDockWidget(Qt::RightDockWidgetArea, text_preview_dock_widget);
          QObject::connect(grplot_widget_->getHideTextPreviewAct(), SIGNAL(triggered()), this,
                           SLOT(hideTextPreviewDockSlot()));
          QObject::connect(grplot_widget_->getShowTextPreviewAct(), SIGNAL(triggered()), this,
                           SLOT(showTextPreviewDockSlot()));
          QObject::connect(text_preview_dock_widget, SIGNAL(resizeMainWindow()), this,
                           SLOT(closeTextPreviewDockSlot()));

          selection_list_dock_widget = new GRPlotDockWidget("Selection List:", LEFT_AREA_WIDTH, 100, this);
          selection_list_dock_widget->setWidget(grplot_widget_->getSelectionListWidget());
          selection_list_dock_widget->setAllowedAreas(Qt::LeftDockWidgetArea);
          selection_list_dock_widget->hide();
          addDockWidget(Qt::LeftDockWidgetArea, selection_list_dock_widget);
          QObject::connect(grplot_widget_->getShowSelectionListWidgetAct(), SIGNAL(triggered()), this,
                           SLOT(showSelectionListDockSlot()));
          QObject::connect(grplot_widget_->getHideSelectionListWidgetAct(), SIGNAL(triggered()), this,
                           SLOT(hideSelectionListDockSlot()));
          QObject::connect(selection_list_dock_widget, SIGNAL(resizeMainWindow()), this,
                           SLOT(closeSelectionListDockSlot()));

          icon_bar_dock_widget = new GRPlotDockWidget("", 300, 28, this);
          icon_bar_dock_widget->setWidget(grplot_widget_->getIconBarWidget());
          icon_bar_dock_widget->setTitleBarWidget(new QWidget());
          icon_bar_dock_widget->setAllowedAreas(Qt::TopDockWidgetArea);
          icon_bar_dock_widget->setFixedHeight(28);
          icon_bar_dock_widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
          icon_bar_dock_widget->setMinimumWidth(0);
          icon_bar_dock_widget->setFeatures(QDockWidget::DockWidgetFloatable);
          addDockWidget(Qt::TopDockWidgetArea, icon_bar_dock_widget);
          QObject::connect(grplot_widget_->getHideIconBarAct(), SIGNAL(triggered()), this, SLOT(hideIconBarDockSlot()));
          QObject::connect(grplot_widget_->getShowIconBarAct(), SIGNAL(triggered()), this, SLOT(showIconBarDockSlot()));
          QObject::connect(icon_bar_dock_widget, SIGNAL(resizeMainWindow()), this, SLOT(closeIconBarDockSlot()));

          this->tabifyDockWidget(tree_dock_widget, table_dock_widget);

          // this way the existing tabs gets shown on the top side of the DockWidgetArea instead of the bottom side
          this->setTabPosition(Qt::RightDockWidgetArea, QTabWidget::North);
          this->setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North);
          this->setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::North);
          this->setTabPosition(Qt::TopDockWidgetArea, QTabWidget::North);

          resizeGRPlotWidget(width, height);
        }
    }
  center();
}

GRPlotMainWindow::~GRPlotMainWindow() = default;

void GRPlotMainWindow::findButtonClickedSlot()
{
  QString search_string = find_line_edit->text();
  message->find(search_string, QTextDocument::FindWholeWords);
}

void GRPlotMainWindow::keyPressEvent(QKeyEvent *event)
{
  if (this->help_mode)
    {
      if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) findButtonClickedSlot();
    }
}

void GRPlotMainWindow::resizeGRPlotWidget(int w, int h)
{
  // the addition of a DockWidget will lead to a smaller grplotWidget
  // -> resize the grplotWidget to its desired size
  // -> GrPlotMainWindow will grow
  if (grplot_widget_ != nullptr)
    {
      grplot_widget_->setMinimumSize(w, h);
      adjustSize();
      grplot_widget_->setMinimumSize(0, 0);
    }
}

void GRPlotMainWindow::showEditElementDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  edit_element_dock_widget->show();
  edit_element_dock_widget->setWindowTitle(grplot_widget_->getEditElementWidget()->windowTitle());
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::showTreeWidgetDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  tree_dock_widget->show();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::showTableWidgetDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  table_dock_widget->show();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::showTextPreviewDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  text_preview_dock_widget->show();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::showSelectionListDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  selection_list_dock_widget->show();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::showIconBarDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  icon_bar_dock_widget->show();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::hideEditElementDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  edit_element_dock_widget->hide();
  edit_element_dock_widget->setWindowTitle(grplot_widget_->getEditElementWidget()->windowTitle());
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::hideTreeWidgetDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  tree_dock_widget->hide();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::hideTableWidgetDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  table_dock_widget->hide();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::hideTextPreviewDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  text_preview_dock_widget->hide();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::hideSelectionListDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  selection_list_dock_widget->hide();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::hideIconBarDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();
  icon_bar_dock_widget->hide();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::closeEditElementDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();

  edit_element_dock_widget->hide();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::closeTreeWidgetDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();

  tree_dock_widget->hide();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::closeTableWidgetDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();

  table_dock_widget->hide();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::closeTextPreviewDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();

  text_preview_dock_widget->hide();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::closeSelectionListDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();

  selection_list_dock_widget->hide();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::closeIconBarDockSlot()
{
  auto w = grplot_widget_->width();
  auto h = grplot_widget_->height();

  icon_bar_dock_widget->hide();
  resizeGRPlotWidget(w, h);
  center();
}

void GRPlotMainWindow::center()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
  auto new_dimensions = this->screen()->availableSize();
  this->move((new_dimensions.width() - this->width()) / 2, (new_dimensions.height() - this->height()) / 2);
#else
  QScreen *screen = QGuiApplication::primaryScreen();
  QRect screen_geometry = screen->geometry();
  this->move((screen_geometry.width() - this->width()) / 2, (screen_geometry.height() - this->height()) / 2);
#endif
}
