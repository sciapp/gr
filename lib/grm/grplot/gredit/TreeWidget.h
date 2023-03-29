#ifndef QT_EXAMPLE_TREEWIDGET_H
#define QT_EXAMPLE_TREEWIDGET_H

#include <QTreeWidget>
#include <utility>
#include <grm/dom_render/graphics_tree/Element.hxx>


class TreeWidget : public QTreeWidget
{
  Q_OBJECT
public:
  explicit TreeWidget(QWidget *parent = nullptr);
  void updateData(std::shared_ptr<GR::Element> ref);

protected:
  void updateDataRecursion(std::shared_ptr<GR::Element> ref, QTreeWidgetItem *parent);
  QTreeWidgetItem *plotTree;
};


#endif // QT_EXAMPLE_TREEWIDGET_H
