#include "CustomTreeWidgetItem.h"
#include <utility>

CustomTreeWidgetItem::CustomTreeWidgetItem(QTreeWidgetItem *parent, std::shared_ptr<GRM::Element> pRef)
    : QTreeWidgetItem(parent), ref(pRef)
{
  this->ref = pRef;
  this->setFlags(this->flags());
}

std::shared_ptr<GRM::Element> CustomTreeWidgetItem::getRef()
{
  return ref;
}

CustomTreeWidgetItem::CustomTreeWidgetItem(QTreeWidgetItem *other) : QTreeWidgetItem(other)
{
  this->setFlags(this->flags());
}
