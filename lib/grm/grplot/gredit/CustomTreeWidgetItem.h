#ifndef QT_EXAMPLE_CUSTOMTREEWIDGETITEM_H
#define QT_EXAMPLE_CUSTOMTREEWIDGETITEM_H

#include <grm/dom_render/graphics_tree/Element.hxx>

#include <QTreeWidgetItem>

class CustomTreeWidgetItem : public QTreeWidgetItem
{
public:
  explicit CustomTreeWidgetItem(QTreeWidgetItem *other);
  explicit CustomTreeWidgetItem(QTreeWidgetItem *other, std::shared_ptr<GRM::Element> pRef = nullptr);
  //    void setData(int column, int role, const QVariant &value) override
  //    {
  //        const bool isCheckChange = column == 0
  //                                   && role == Qt::CheckStateRole
  //                                   && data(column, role).isValid() // Don't "change" during initialization
  //                                   && checkState(0) != value;
  //        QTreeWidgetItem::setData(column, role, value);
  //        if (isCheckChange) {
  //            qDebug()<<"hello";
  //        }
  //    }
  std::shared_ptr<GRM::Element> getRef();

protected:
  std::shared_ptr<GRM::Element> ref = nullptr;
};

#endif // QT_EXAMPLE_CUSTOMTREEWIDGETITEM_H
