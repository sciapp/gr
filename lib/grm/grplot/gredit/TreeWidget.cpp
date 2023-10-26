#include <QTreeWidgetItem>
#include <queue>
#include <QHeaderView>
#include "TreeWidget.h"
#include "CustomTreeWidgetItem.h"


TreeWidget::TreeWidget(QWidget *parent) : QTreeWidget(parent)
{
  this->setWindowTitle("DOM-Tree Elements");
  this->setColumnCount(1);
  this->header()->setSectionResizeMode(QHeaderView::Stretch);
  this->setHeaderHidden(true);
}

void TreeWidget::updateData(std::shared_ptr<GRM::Element> ref)
{
  this->clear();
  plotTree = new QTreeWidgetItem(this);
  plotTree->setText(0, tr("root"));
  plotTree->setExpanded(true);

  for (const auto &cur_elem : ref->children())
    {
      updateDataRecursion(cur_elem, plotTree);
    }
}

void TreeWidget::updateDataRecursion(std::shared_ptr<GRM::Element> ref, QTreeWidgetItem *parent)
{
  bool skip = false;
  QTreeWidgetItem *item = new QTreeWidgetItem(parent);
  std::string name = ref->localName();

  if (ref->hasAttribute("name")) name += " (" + static_cast<std::string>(ref->getAttribute("name")) + ")";
  item->setText(0, tr(name.c_str()));
  item->setExpanded(true);
  for (const auto &cur_elem : ref->children())
    {
      updateDataRecursion(cur_elem, item);
    }
}
