#ifndef EDITELEMENTWIDGET_H
#define EDITELEMENTWIDGET_H

#include <grm/dom_render/graphics_tree/Element.hxx>
#include <QWidget>
#include <QScrollArea>
#include <QLineEdit>
#include <QCheckBox>
#include <QGridLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QAction>
#include <vector>
#include <QKeyEvent>
#include "grm.h"
class EditElementWidget;
#include "../grplotWidget.hxx"


class EditElementWidget : public QWidget
{
  Q_OBJECT
public:
  explicit EditElementWidget(GRPlotWidget *widget, QWidget *parent = nullptr);

  void attributeEditEvent(bool highlight_location = false);

private slots:
  void reject();
  void accept();

protected:
  void keyPressEvent(QKeyEvent *event) override;

private:
  GRPlotWidget *grplot_widget;
  std::shared_ptr<GRM::Document> schema_tree;
  QList<QString> labels;
  QList<QWidget *> fields;
  std::unordered_map<std::string, std::string> attr_type;
};


#endif // EDITELEMENTWIDGET_H
