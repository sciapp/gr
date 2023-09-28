#ifndef GR_ADDELEMENTWIDGET_H
#define GR_ADDELEMENTWIDGET_H


#include <grm/dom_render/graphics_tree/Element.hxx>
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QAction>
#include <vector>
#include "grm.h"
#include "Bounding_object.h"
class AddElementWidget;
#include "../grplot_widget.hxx"


class AddElementWidget : public QWidget
{
  Q_OBJECT
public:
  explicit AddElementWidget(GRPlotWidget *widget, QWidget *parent = nullptr);
  EXPORT void setBoundingBoxRef(Bounding_object **ref);

protected:
private slots:
  void elementSelected(int);
  void parentSelected(int);
  void reject();
  void accept();

private:
  GRPlotWidget *grplot_widget;
  QComboBox *addElementComboBox;
  QComboBox *selectParentComboBox;
  QGridLayout *addElementLayout;
  QGroupBox *addAttributesGroup;
  std::vector<Bounding_object> parent_vec;
  std::vector<std::string> attribute_name_vec;
  std::vector<std::string> attribute_type_vec;
  QList<QWidget *> fields;
};


#endif // GR_ADDELEMENTWIDGET_H
