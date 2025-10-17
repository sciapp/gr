#ifndef QT_EXAMPLE_TREEWIDGET_H
#define QT_EXAMPLE_TREEWIDGET_H

#include <QTreeWidget>
#include <utility>

#include <grm/dom_render/graphics_tree/element.hxx>
#include "CustomTreeWidgetItem.hxx"
class TreeWidget;
#include "../GRPlotWidget.hxx"


class TreeWidget : public QTreeWidget
{
  Q_OBJECT
public:
  explicit TreeWidget(GRPlotWidget *widget, QWidget *parent = nullptr);
  void updateData(std::shared_ptr<GRM::Element> ref);
  bool selectItem(const std::shared_ptr<GRM::Element> &ref, CustomTreeWidgetItem *tree_elem = nullptr);
  void clearContractElements();

protected:
  void updateDataRecursion(std::shared_ptr<GRM::Element> ref, CustomTreeWidgetItem *parent);
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void checkIfCollapsed(std::shared_ptr<GRM::Element> ref, CustomTreeWidgetItem *parent);
  CustomTreeWidgetItem *plot_tree;

private:
  GRPlotWidget *grplot_widget;
  bool cleared;
  std::list<std::weak_ptr<GRM::Element>> contract_elements;
  bool findSelectedItem(CustomTreeWidgetItem *item);
  bool checkboxStatusChanged(CustomTreeWidgetItem *item);
};


#endif // QT_EXAMPLE_TREEWIDGET_H
