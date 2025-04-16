#ifndef QT_EXAMPLE_CUSTOMTREEWIDGETITEM_H
#define QT_EXAMPLE_CUSTOMTREEWIDGETITEM_H

#include <grm/dom_render/graphics_tree/Element.hxx>

#include <QTreeWidgetItem>

class CustomTreeWidgetItem : public QTreeWidgetItem
{
public:
  explicit CustomTreeWidgetItem(QTreeWidgetItem *other);
  explicit CustomTreeWidgetItem(QTreeWidgetItem *other, std::shared_ptr<GRM::Element> p_ref = nullptr);
  std::shared_ptr<GRM::Element> getRef();

protected:
  std::shared_ptr<GRM::Element> ref = nullptr;
};

#endif // QT_EXAMPLE_CUSTOMTREEWIDGETITEM_H
