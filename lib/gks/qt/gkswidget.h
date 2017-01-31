
#include <QWidget>
#include <QMainWindow>
#include <QMutex>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QKeySequence>
#include <QPrintDialog>
#include <QPageSetupDialog>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSignalMapper>
#include <QPainter>
#include <QIcon>
#include <QPrinter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QLabel>
#include <QRubberBand>
#include <QSpacerItem>

#include "gksserver.h"

class GKSWidget: public QWidget
{
  Q_OBJECT

 public:
  GKSWidget(QWidget *parent=0, Qt::WindowFlags flags = 0);
  ~GKSWidget();

  void setWidgetNumber(int number = 0);
  void setLastWidgetNumber(int number = 0);
  int getWidgetNumber();
  int getLastWidgetNumber();
  QPixmap* getPixmap();

 protected:
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

 private:
  int widgetNumber;
  static int lastWidgetNumber;
  int rotation;
  qreal rotateBy;
  char *dl;
  QPixmap *pm;
  QVBoxLayout *layout;
  QStatusBar *sb;
  QLabel *sb_label;
  QRubberBand *rubberBand;
  QSpacerItem *spacer;
  bool leftButton;
  bool first;

 public:
  void rotate();
  void setRotation(const int val);
  int getRotation();
  void setRotateBy(const qreal val);
  void setDisplayList(char *val);
};

class GKSQtWindow : public QMainWindow
{
  Q_OBJECT

 public:
  GKSQtWindow(QWidget *parent = 0);
  ~GKSQtWindow();

 private:
  void createMenubar();
  void createToolbar();
  char *dl;
  GKSServer *server;
  QMdiArea *mdiArea;
  QSignalMapper *windowMapper;
  GKSWidget *activeMdiChild();

 private:
  QAction  *actionQuitGKSQt;
  QAction  *actionClose;
  QAction  *actionCloseAll;
  QAction  *actionSave_As;
  QAction  *actionPage_Setup;
  QAction  *actionPrint;
  QAction  *actionCut;
  QAction  *actionCopy;
  QAction  *actionPaste;
  QAction  *actionKeep_on_Display;
  QAction  *actionRotate_by_90;
  QAction  *actionSpecial_Characters;
  QAction  *actionMinimize;
  QAction  *actionTabbedView;
  QMenu    *menuTabPosition;
  QActionGroup *groupTabPosition;
  QAction  *actionTabPositionNorth;
  QAction  *actionTabPositionWest;
  QAction  *actionTabPositionSouth;
  QAction  *actionTabPositionEast;
  QAction  *actionTile;
  QAction  *actionCascade;
  QAction  *actionNext;
  QAction  *actionPrevious;
  QAction  *actionGKSQt;
  QAction  *actionGKSQt_Help;
  QAction  *actionAbout_Qt;
  QMenuBar *menuBar;
  QMenu    *menuFile;
  QMenu    *menuEdit;
  QMenu    *menuWindow;
  QMenu    *menuView;
  QMenu    *menuHelp;

  QToolBar *toolBarFile;
  QToolBar *toolBarEdit;
  QToolBar *toolBarWindow;

  QPrinter *printer;

  void     SaveFileAs (const QString fname);   
  QString  savePath;
  QList<QByteArray> supportedFileFmtList;
  qreal    rotateBy;
  int      rotation;
  int      numWidgets;

 private slots:
  void on_actionQuitGKSQt_triggered();
  void on_actionClose_triggered();
  void on_actionCloseAll_triggered();
  void on_actionSave_As_triggered();
  void on_actionPage_Setup_triggered();
  void on_actionPrint_triggered();

  void on_actionCut_triggered();
  void on_actionCopy_triggered();
  void on_actionPaste_triggered();
  void on_actionKeep_on_Display_triggered();
  void on_actionRotate_by_90_triggered();
  void on_actionSpecial_Characters_triggered();

  void on_actionMinimize_triggered();
  void on_actionTabbedView_triggered();
  void on_actionTabPositionNorth_triggered();
  void on_actionTabPositionEast_triggered();
  void on_actionTabPositionSouth_triggered();
  void on_actionTabPositionWest_triggered();
  void on_actionTile_triggered();
  void on_actionCascade_triggered();
  void on_actionNext_triggered();
  void on_actionPrevious_triggered();

  void on_actionAbout_Qt_triggered();
  void on_actionGKSQt_Help_triggered();

  void updateMenuWindow();

public slots:
  void interpret(char *dl);
  void openWindow();
  void updateMenus();
  void setActiveSubWindow(QWidget *window);
};
