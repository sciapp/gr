#include "CustomTreeWidgetItem.h"
#include <utility>

CustomTreeWidgetItem::CustomTreeWidgetItem(QTreeWidgetItem *parent, std::shared_ptr<GRM::Element> pRef)
    : QTreeWidgetItem(parent, 1001), ref(std::move(pRef))
{
  this->setFlags(this->flags() | Qt::ItemIsEditable);
}

std::shared_ptr<GRM::Element> CustomTreeWidgetItem::getRef()
{
  return ref;
}

CustomTreeWidgetItem::CustomTreeWidgetItem(const QTreeWidgetItem &other) : QTreeWidgetItem(other)
{
  this->setFlags(this->flags() | Qt::ItemIsEditable);
}
