#include "CustomQListWidgetItem.hxx"

CustomQListWidgetItem::CustomQListWidgetItem(const QString &text, const std::shared_ptr<GRM::Element> &selection,
                                             QListWidget *parent)
    : QListWidgetItem(text, parent)
{
  this->element_ref = selection;
}

std::shared_ptr<GRM::Element> CustomQListWidgetItem::getElementRef()
{
  return element_ref;
}
