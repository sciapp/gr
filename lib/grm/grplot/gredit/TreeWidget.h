#ifndef QT_EXAMPLE_TREEWIDGET_H
#define QT_EXAMPLE_TREEWIDGET_H

#include <QTreeWidget>
#include <utility>

#include <grm/dom_render/graphics_tree/Element.hxx>
#include "CustomTreeWidgetItem.h"
class TreeWidget;
#include "../grplot_widget.hxx"


class TreeWidget : public QTreeWidget
{
  Q_OBJECT
public:
  explicit TreeWidget(GRPlotWidget *widget, QWidget *parent = nullptr);
  void updateData(std::shared_ptr<GRM::Element> ref);
  bool selectItem(std::shared_ptr<GRM::Element> ref, CustomTreeWidgetItem *tree_elem = nullptr);

protected:
  void updateDataRecursion(std::shared_ptr<GRM::Element> ref, CustomTreeWidgetItem *parent);
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  CustomTreeWidgetItem *plotTree;

private:
  GRPlotWidget *grplot_widget;
  bool findSelectedItem(CustomTreeWidgetItem *item);
};


#endif // QT_EXAMPLE_TREEWIDGET_H
