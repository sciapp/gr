#include <grm/dom_render/graphics_tree/util.hxx>
#include "main_window.hxx"
#include <cfloat>
#include <algorithm>
#include <fstream>
#include <QStackedLayout>
#include <QInputDialog>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QtWidgets>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrinter>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
  qDebug() << "Internal Qt Version:" << QT_VERSION_STR;
  init_env_vars();
  setMouseTracking(true);
  to_draw = nullptr;
  pixmap = nullptr;
  highlightBoundingObjects = false;
  bounding_logic = new Bounding_logic();
  current_selection = nullptr;
  amount_scrolled = 0;
  treewidget = new TreeWidget();
  treewidget->hide();

  menubar = new QMenuBar;
  file_menu = new QMenu("File");
  save_file_action = new QAction("Save Plot");
  save_file_action->setShortcut(Qt::CTRL | Qt::Key_S);
  file_menu->addAction(save_file_action);
  menubar->addMenu(file_menu);
  QObject::connect(save_file_action, SIGNAL(triggered()), this, SLOT(save_file_slot()));

  open_file_action = new QAction("Open Plot");
  open_file_action->setShortcut(Qt::CTRL | Qt::Key_O);
  file_menu->addAction(open_file_action);
  menubar->addMenu(file_menu);
  QObject::connect(open_file_action, SIGNAL(triggered()), this, SLOT(open_file_slot()));

  print_action = new QAction("Print");
  print_action->setShortcut(Qt::CTRL | Qt::Key_P);
  file_menu->addAction(print_action);
  menubar->addMenu(file_menu);
  QObject::connect(print_action, SIGNAL(triggered()), this, SLOT(print_slot()));


  configuration_menu = new QMenu("Configuration");
  show_container_action = new QAction("Show Container");
  show_container_action->setCheckable(true);
  show_container_action->setShortcut(Qt::CTRL | Qt::Key_C);
  configuration_menu->addAction(show_container_action);
  show_bounding_boxes_action = new QAction("Show Bounding Boxes");
  show_bounding_boxes_action->setCheckable(true);
  show_bounding_boxes_action->setShortcut(Qt::CTRL | Qt::Key_B);
  configuration_menu->addAction(show_bounding_boxes_action);
  menubar->addMenu(configuration_menu);
  menubar->show();
  QObject::connect(show_container_action, SIGNAL(triggered()), this, SLOT(show_container_slot()));
  QObject::connect(show_bounding_boxes_action, SIGNAL(triggered()), this, SLOT(show_bounding_boxes_slot()));
  //    show_container_action->setChecked(true);
  //    show_bounding_boxes_action->setChecked(true);
  show_container_slot();

  init_simple_plot_data();
}

void MainWindow::init_env_vars()
{
  qputenv("GKS_WSTYPE", "381");
  qputenv("GKS_DOUBLE_BUF", "True");
}

void MainWindow::show_container_slot()
{
  if (show_container_action->isChecked())
    {
      treewidget->show();
    }
  else
    {
      treewidget->hide();
    }
  treewidget->resize(400, height());
  treewidget->move(this->pos().x() + this->width(),
                   this->pos().y() - 28 + (treewidget->geometry().y() - treewidget->pos().y()));
}

void MainWindow::print_slot()
{
  //    QString old_wstype = qgetenv("GKS_WSTYPE");
  //    gr_beginprint("out.jpeg");
  //
  //    update();
  //    repaint();
  //
  //    gr_endprint();
  //    qputenv("GKS_WSTYPE", old_wstype.toStdString().c_str());
}

void MainWindow::open_file_slot()
{
  init_simple_plot_data();
  update();
}

void MainWindow::save_file_slot()
{
  if (grm_get_render() == nullptr)
    {
      QApplication::beep();
      return;
    }
  std::string save_file_name =
      QFileDialog::getSaveFileName(this, "Save XML", QDir::homePath(), "XML files (*.xml)").toStdString();
  if (save_file_name.empty())
    {
      return;
    }
  std::ofstream save_file_stream(save_file_name);
  if (!save_file_stream)
    {
      std::stringstream text_stream;
      text_stream << "Could not save the graphics tree to the XML file \"" << save_file_name << "\".";
      QMessageBox::critical(this, "File save not possible", QString::fromStdString(text_stream.str()));
      return;
    }
  save_file_stream << GR::toXML(grm_get_render()) << std::endl;
  save_file_stream.close();
}

void MainWindow::show_bounding_boxes_slot()
{
  highlightBoundingObjects = show_bounding_boxes_action->isChecked();
  update();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event)
  if (!to_draw)
    {
      return;
    }
  QPainter painter;
  if (!pixmap)
    {
      pixmap = new QPixmap((int)(geometry().width() * this->devicePixelRatioF()),
                           (int)(geometry().height() * this->devicePixelRatioF()));
      std::stringstream addresses;
      pixmap->setDevicePixelRatio(this->devicePixelRatioF());
      addresses << static_cast<void *>(this) << "!" << static_cast<void *>(&painter);
      qputenv("GKS_CONID", addresses.str().c_str());
      painter.begin(pixmap);

      painter.fillRect(0, 0, width(), height(), QColor("white"));
      qDebug() << "drawing";
      painter.save();
      draw();
      //        std::cerr << GR::toXML(grm_get_document_root()) << std::endl;
      painter.restore();
      painter.end();

      qDebug() << "print in paintEvent:\033[1;34m";
      std::cerr << GR::toXML(grm_get_render()) << std::endl;
      qDebug() << "\033[0m";
      treewidget->updateData(grm_get_document_root());
    }
  if (pixmap)
    {
      painter.begin(this);
      painter.drawPixmap(0, 0, *pixmap);
      bounding_logic->clear();
      extract_bounding_boxes_from_grm((QPainter &)painter);
      highlight_current_selection((QPainter &)painter);
      painter.end();
    }
}

void MainWindow::draw()
{
  bool write_to_file = false;
  if (to_draw)
    {
      if (write_to_file)
        {
          char *file = strdup("tmp.jpg");
          gr_beginprint(file);
          int ret_code = grm_plot(nullptr);
          qDebug() << "Return code of gr_plotmeta()" << ret_code;
          //            gr_endprint();
          QImage image("tmp.jpg");
          QImageWriter writer("out.png", "png");
          writer.setText("XML-Data", GR::toXML(grm_get_render()).c_str());
          writer.write(image);

          //         Read text
          //        QImageReader im("/Users/clever/workspace/Masterarbeit/qt_example/cmake-build-debug/out.png");
          //        qDebug() << im.text("XML");
        }
      else
        {
          //            gr_beginprint("/Users/clever/Desktop/out.png");
          int ret_code = grm_plot(nullptr);
          //            gr_endprint();
          qDebug() << "Return code of gr_plotmeta()" << ret_code;
        }
    }
}

void MainWindow::reset_pixmap()
{
  pixmap = nullptr;
  current_selection = nullptr;
  update();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
  if (!to_draw)
    {
      return;
    }
  if (event->type() == QEvent::KeyPress)
    {
      QPoint widget_cursor_pos = mapFromGlobal(QCursor::pos());
      auto *key = event;
      switch (key->key())
        {
        case Qt::Key_R:
          reset_pixmap();
          break;
        case Qt::Key_Escape:
          current_selection = nullptr;
          mouse_move_selection = nullptr;
          update();
          break;
        case Qt::Key_Space:
          show_bounding_boxes_action->trigger();
          break;
        case Qt::Key_Return:
          {
            if (current_selection == nullptr)
              {
                return;
              }
            std::string currently_clicked_name = current_selection->get_ref()->localName();

            QDialog dialog(this);
            QFormLayout form(&dialog);
            QString title("Selected: ");
            title.append(currently_clicked_name.c_str());
            dialog.setWindowTitle(title);
            auto changeParametersLabel = new QLabel("Change Parameters:");
            changeParametersLabel->setStyleSheet("font-weight: bold");
            form.addRow(changeParametersLabel);

            QList<QString> labels;
            QList<QWidget *> fields;
            QWidget *lineEdit;

            for (const auto &cur_attr_name : current_selection->get_ref()->getAttributeNames())
              {
                if (cur_attr_name == "bbox_id" || cur_attr_name == "bbox_xmin" || cur_attr_name == "bbox_xmax" ||
                    cur_attr_name == "bbox_ymin" || cur_attr_name == "bbox_ymax")
                  {
                    continue;
                  }
                if (cur_attr_name == "render_method")
                  {
                    lineEdit = new QComboBox(&dialog);
                    QString tooltipString =
                        GR::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
                    tooltipString.append(" Default: ");
                    tooltipString.append(
                        GR::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
                    ((QComboBox *)lineEdit)->setToolTip(tooltipString);
                    ((QComboBox *)lineEdit)->addItem("gr_text");     // 0
                    ((QComboBox *)lineEdit)->addItem("gr_mathtext"); // 1
                    ((QComboBox *)lineEdit)->addItem("gr_textext");  // 2
                    ((QComboBox *)lineEdit)
                        ->setCurrentIndex(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
                  }
                else if (cur_attr_name == "textalign_vertical")
                  {
                    lineEdit = new QComboBox(&dialog);
                    QString tooltipString =
                        GR::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
                    tooltipString.append(" Default: ");
                    tooltipString.append(
                        GR::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
                    ((QComboBox *)lineEdit)->setToolTip(tooltipString);
                    ((QComboBox *)lineEdit)->addItem("normal"); // 0
                    ((QComboBox *)lineEdit)->addItem("top");    // 1
                    ((QComboBox *)lineEdit)->addItem("cap");    // 2
                    ((QComboBox *)lineEdit)->addItem("half");   // 3
                    ((QComboBox *)lineEdit)->addItem("base");   // 4
                    ((QComboBox *)lineEdit)->addItem("bottom"); // 5
                    ((QComboBox *)lineEdit)
                        ->setCurrentIndex(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
                  }
                else if (cur_attr_name == "textalign_horizontal")
                  {
                    lineEdit = new QComboBox(&dialog);
                    QString tooltipString =
                        GR::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
                    tooltipString.append(" Default: ");
                    tooltipString.append(
                        GR::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
                    ((QComboBox *)lineEdit)->setToolTip(tooltipString);
                    ((QComboBox *)lineEdit)->addItem("normal"); // 0
                    ((QComboBox *)lineEdit)->addItem("left");   // 1
                    ((QComboBox *)lineEdit)->addItem("center"); // 2
                    ((QComboBox *)lineEdit)->addItem("right");  // 3
                    ((QComboBox *)lineEdit)
                        ->setCurrentIndex(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)));
                  }
                else if (cur_attr_name == "textalign")
                  {
                    lineEdit = new QCheckBox(&dialog);
                    QString tooltipString =
                        GR::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
                    tooltipString.append(" Default: ");
                    tooltipString.append(
                        GR::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
                    ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                    ((QCheckBox *)lineEdit)
                        ->setChecked(static_cast<int>(current_selection->get_ref()->getAttribute(cur_attr_name)) == 1);
                  }
                else
                  {
                    lineEdit = new QLineEdit(&dialog);
                    ((QLineEdit *)lineEdit)
                        ->setText(static_cast<std::string>(current_selection->get_ref()->getAttribute(cur_attr_name))
                                      .c_str());
                    QString tooltipString =
                        GR::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[1].c_str();
                    tooltipString.append(" Default: ");
                    tooltipString.append(
                        GR::Render::getDefaultAndTooltip(current_selection->get_ref(), cur_attr_name)[0].c_str());
                    ((QLineEdit *)lineEdit)->setToolTip(tooltipString);
                  }
                QString label = QString(cur_attr_name.c_str());
                form.addRow(label, lineEdit);

                labels << label;
                fields << lineEdit;
              }

            QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
            form.addRow(&buttonBox);
            QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
            QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

            if (dialog.exec() == QDialog::Accepted)
              {
                for (int i = 0; i < labels.count(); i++)
                  {
                    qDebug() << typeid(fields[i]).name();
                    auto &field = *fields[i]; // because typeid(*fields[i]) is bad :(
                    if (typeid(field) == typeid(QLineEdit))
                      {
                        current_selection->get_ref()->setAttribute(labels[i].toStdString(),
                                                                   ((QLineEdit *)fields[i])->text().toStdString());
                      }
                    else if (typeid(field) == typeid(QComboBox))
                      {
                        current_selection->get_ref()->setAttribute(labels[i].toStdString(),
                                                                   ((QComboBox *)fields[i])->currentIndex());
                      }
                    else if (typeid(field) == typeid(QCheckBox))
                      {
                        current_selection->get_ref()->setAttribute(labels[i].toStdString(),
                                                                   ((QCheckBox *)fields[i])->isChecked());
                      }
                  }
                current_selection = nullptr;
                mouse_move_selection = nullptr;
                amount_scrolled = 0;
                clicked.clear();
                std::cerr << GR::toXML(grm_get_document_root()) << std::endl;
                reset_pixmap();
              }
          }
          break;
        case Qt::Key_Delete:
          if (current_selection == nullptr)
            {
              return;
            }
          amount_scrolled = 0;
          current_selection->get_ref()->remove();
          std::cerr << GR::toXML(grm_get_document_root()) << std::endl;
          mouse_move_selection = nullptr;
          reset_pixmap();
          break;
        default:
          // Do not do anything if key is unknown
          break;
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
  if (to_draw && event->oldSize() != event->size())
    {
      grm_args_t *size_container = grm_args_new();
      grm_args_push(size_container, "size", "dd", (double)event->size().width(), (double)event->size().height());
      grm_merge_hold(size_container);
    }
  current_selection = nullptr;
  mouse_move_selection = nullptr;
  amount_scrolled = 0;
  clicked.clear();
  pixmap = nullptr;
}

MainWindow::~MainWindow()
{
  if (to_draw)
    {
      grm_args_delete(to_draw);
    }
  grm_finalize();
}


void MainWindow::wheelEvent(QWheelEvent *event)
{
  QPoint numPixels = event->pixelDelta();
  QPoint numDegrees = event->angleDelta();

  if (!numPixels.isNull())
    {
      // Scrolling with pixels (For high-res scrolling like on macOS)

      // Prevent flickering when scrolling fast
      if (numPixels.y() > 0)
        {
          amount_scrolled += numPixels.y() < 10 ? numPixels.y() : 10;
        }
      else if (numPixels.y() < 0)
        {
          amount_scrolled += numPixels.y() > -10 ? numPixels.y() : -10;
        }
    }
  else if (!numDegrees.isNull())
    {
      QPoint numSteps = numDegrees / 16;
      // Scrolling with degrees
      if (numSteps.y() != 0)
        {
          amount_scrolled += numSteps.y();
        }
    }

  if (amount_scrolled > 50)
    {
      if (!clicked.empty() && current_selection != nullptr)
        {
          for (int i = 0; i < clicked.size(); i++)
            {
              if (clicked[i].boundingRect() == current_selection->boundingRect())
                {
                  if (i + 1 < clicked.size())
                    {
                      current_selection = &clicked[i + 1];
                      break;
                    }
                  else
                    {
                      current_selection = &clicked[0];
                    }
                }
            }
        }
      amount_scrolled = 0;
    }
  else if (amount_scrolled < -50)
    {
      if (!clicked.empty() && current_selection != nullptr)
        {
          for (int i = clicked.size() - 1; i >= 0; i--)
            {
              if (clicked[i].boundingRect() == current_selection->boundingRect())
                {
                  if (i - 1 < 0)
                    {
                      current_selection = &clicked[clicked.size() - 1];
                    }
                  else
                    {
                      current_selection = &clicked[i - 1];
                      break;
                    }
                }
            }
        }
      amount_scrolled = 0;
    }
  else
    {
      return;
    }
  update();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
  if (event->buttons() & Qt::LeftButton)
    {
      amount_scrolled = 0;
      auto cur_clicked = bounding_logic->get_bounding_objects_at_point(event->pos().x(), event->pos().y());

      if (cur_clicked.empty())
        {
          clicked.clear();
          current_selection = nullptr;
          update();
          return;
        }
      else
        {
          clicked = cur_clicked;
        }
      current_selection = &clicked[0];
    }
  if (event->buttons() & Qt::RightButton)
    {
      current_selection = nullptr;
      mouse_move_selection = nullptr;
    }
  update();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
  amount_scrolled = 0;
  auto cur_clicked = bounding_logic->get_bounding_objects_at_point(event->pos().x(), event->pos().y());

  if (current_selection == nullptr)
    {
      mouse_move_selection = &cur_clicked[0];
      update();
    }
}

void MainWindow::leaveEvent(QEvent *event)
{
  // Prevent highlighting if mouse leaves the widget without a mouseMoveEvent
  mouse_move_selection = nullptr;
  update();
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {}

void MainWindow::highlight_current_selection(QPainter &painter)
{
  if (current_selection != nullptr)
    {
      painter.fillRect(current_selection->boundingRect(), QBrush(QColor("blue"), Qt::Dense5Pattern));
    }
  else if (mouse_move_selection != nullptr)
    {
      painter.fillRect(mouse_move_selection->boundingRect(), QBrush(QColor("blue"), Qt::Dense7Pattern));
    }
}

void MainWindow::extract_bounding_boxes_from_grm(QPainter &painter)
{
  auto global_root = grm_get_document_root();
  double xmin, xmax, ymin, ymax;
  int id;
  painter.setPen(QPen(QColor(255, 0, 0, 100)));

  for (const auto &cur_child : global_root->querySelectorsAll("[bbox_id]"))
    {
      id = static_cast<int>(cur_child->getAttribute("bbox_id"));
      xmin = static_cast<double>(cur_child->getAttribute("bbox_xmin"));
      xmax = static_cast<double>(cur_child->getAttribute("bbox_xmax"));
      ymin = static_cast<double>(cur_child->getAttribute("bbox_ymin"));
      ymax = static_cast<double>(cur_child->getAttribute("bbox_ymax"));

      if (xmin == DBL_MAX || xmax == -DBL_MAX || ymin == DBL_MAX || ymax == -DBL_MAX)
        {
          qDebug() << "skipping" << cur_child->localName().c_str();
        }
      else
        {
          auto b = Bounding_object(id, xmin, xmax, ymin, ymax, cur_child);
          bounding_logic->add_bounding_object(b);
          auto bounding_rect = b.boundingRect();
          //            qDebug() << "drawing" << cur_child->localName().c_str() << "(" << id << ") at" << bounding_rect;
          if (highlightBoundingObjects)
            {
              painter.drawRect(bounding_rect);
              painter.drawText(bounding_rect.bottomRight() + QPointF(5, 0), cur_child->localName().c_str());
            }
        }
    }
}

void MainWindow::init_simple_plot_data()
{
  double plots[2][2][50];
  double xlim[2] = {-10, 10};
  double ylim[2] = {-5, 5};
  int n = sizeof(plots[0][0]) / sizeof(plots[0][0][0]);

  grm_args_t *args, *subplots[1], *series[2];
  int i, j;

  printf("filling argument container...\n");

  for (i = 0; i < 1; ++i)
    {
      for (j = 0; j < n; ++j)
        {
          plots[i][0][j] = -j * 2 * M_PI / n;
          plots[i][1][j] = sin((j * (i + 1) * 2) * M_PI / n);
        }
    }
  for (i = 1; i < 2; ++i)
    {
      for (j = 0; j < n; ++j)
        {
          plots[i][0][j] = j * 2 * M_PI / n - 3;
          plots[i][1][j] = 1. + sin((j * (i + 1) * 2) * M_PI / n) * 1.5;
        }
    }

  subplots[0] = grm_args_new();
  series[0] = grm_args_new();
  grm_args_push(series[0], "x", "nD", n, plots[0][0]);
  grm_args_push(series[0], "y", "nD", n, plots[0][1]);
  series[1] = grm_args_new();
  grm_args_push(series[1], "x", "nD", n, plots[1][0]);
  grm_args_push(series[1], "y", "nD", n, plots[1][1]);
  grm_args_push(subplots[0], "series", "nA", 2, series);
  grm_args_push(subplots[0], "xlim", "nD", 2, xlim);
  grm_args_push(subplots[0], "ylim", "nD", 2, ylim);
  grm_args_push(subplots[0], "title", "s", "Title");
  grm_args_push(subplots[0], "keep_aspect_ratio", "i", 0);

  to_draw = grm_args_new();
  grm_args_push(to_draw, "subplots", "nA", 1, subplots);
  grm_merge(to_draw);
}

void MainWindow::moveEvent(QMoveEvent *event)
{
  treewidget->move(this->width() + event->pos().x(),
                   event->pos().y() - (treewidget->geometry().y() - treewidget->pos().y()));
}
