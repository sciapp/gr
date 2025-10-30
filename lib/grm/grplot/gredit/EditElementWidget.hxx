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
#include <QPushButton>
#include <QObject>
#include <QKeyEvent>
#include <vector>
#include "grm.h"

class EditElementWidget;
#include "../grplotWidget.hxx"


class EditElementWidget : public QWidget
{
  Q_OBJECT
public:
  explicit EditElementWidget(GRPlotWidget *widget, QWidget *parent = nullptr);

  void attributeEditEvent(std::vector<std::shared_ptr<GRM::Element>> multiple_selections,
                          bool highlight_location = false);

private slots:
  void reject();
  void accept();
  void colorIndexSlot();
  void colorRGBSlot();
  void openDataContext();
  void openTextPreview();

protected:
  void keyPressEvent(QKeyEvent *event) override;
  bool isAdvancedAttribute(const std::shared_ptr<GRM::Element> &element, std::string attr_name,
                           bool inherit_elem = false);
  void setAttributesDuringAccept(std::shared_ptr<GRM::Element> current_selection);

private:
  GRPlotWidget *grplot_widget;
  std::shared_ptr<GRM::Document> schema_tree;
  QList<QString> labels;
  QList<QWidget *> fields;
  std::unordered_map<std::string, std::string> attr_type;
  std::vector<std::shared_ptr<GRM::Element>> multiple_selections;
};


#endif // EDITELEMENTWIDGET_H
