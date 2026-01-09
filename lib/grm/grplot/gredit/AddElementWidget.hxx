#ifndef GR_ADDELEMENTWIDGET_H
#define GR_ADDELEMENTWIDGET_H


#include <grm/dom_render/graphics_tree/element.hxx>
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QAction>
#include <vector>
#include "grm.h"
#include "BoundingObject.hxx"
class AddElementWidget;
#include "../GRPlotWidget.hxx"


class AddElementWidget : public QWidget
{
  Q_OBJECT
public:
  explicit AddElementWidget(GRPlotWidget *widget, QWidget *parent = nullptr);

protected:
private slots:
  void elementSelected(int);
  void parentSelected(int);
  void reject();
  void accept();

private:
  GRPlotWidget *grplot_widget;
  QComboBox *add_element_combo_box;
  QComboBox *select_parent_combo_box;
  QGridLayout *add_element_layout;
  QGroupBox *add_attributes_group;
  std::vector<BoundingObject> parent_vec;
  std::vector<std::string> attribute_name_vec;
  std::vector<std::string> attribute_type_vec;
  QList<QWidget *> fields;
  std::shared_ptr<GRM::Document> schema_tree;
};


#endif // GR_ADDELEMENTWIDGET_H
