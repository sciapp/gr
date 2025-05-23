#ifndef QT_EXAMPLE_TREEWIDGET_H
#define QT_EXAMPLE_TREEWIDGET_H

#include <QTreeWidget>
#include <utility>

#include <grm/dom_render/graphics_tree/Element.hxx>
#include "CustomTreeWidgetItem.hxx"
class TreeWidget;
#include "../grplotWidget.hxx"


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
  void mouseReleaseEvent(QMouseEvent *event) override;
  CustomTreeWidgetItem *plot_tree;

private:
  GRPlotWidget *grplot_widget;
  bool findSelectedItem(CustomTreeWidgetItem *item);
  bool checkboxStatusChanged(CustomTreeWidgetItem *item);
};


#endif // QT_EXAMPLE_TREEWIDGET_H
