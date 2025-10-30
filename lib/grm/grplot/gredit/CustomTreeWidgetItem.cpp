#include "CustomTreeWidgetItem.hxx"
#include <utility>

CustomTreeWidgetItem::CustomTreeWidgetItem(QTreeWidgetItem *parent, std::shared_ptr<GRM::Element> p_ref)
    : QTreeWidgetItem(parent), ref(std::move(p_ref))
{
  this->ref = std::move(p_ref);
  this->setFlags(this->flags());
}

CustomTreeWidgetItem::CustomTreeWidgetItem(QTreeWidget *parent, std::shared_ptr<GRM::Element> p_ref)
    : QTreeWidgetItem(parent), ref(std::move(p_ref))
{
  this->ref = std::move(p_ref);
  this->setFlags(this->flags());
}

std::shared_ptr<GRM::Element> CustomTreeWidgetItem::getRef()
{
  return ref.lock();
}

CustomTreeWidgetItem::CustomTreeWidgetItem(QTreeWidgetItem *other) : QTreeWidgetItem(other)
{
  this->setFlags(this->flags());
}
