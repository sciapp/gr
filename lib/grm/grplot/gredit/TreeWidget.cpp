#include <QTreeWidgetItem>
#include <queue>
#include <QHeaderView>
#include "TreeWidget.h"


TreeWidget::TreeWidget(GRPlotWidget *widget, QWidget *parent) : QTreeWidget(parent)
{
  grplot_widget = widget;
  this->setWindowTitle("DOM-Tree Elements");
  this->setColumnCount(1);
  this->header()->setSectionResizeMode(QHeaderView::Stretch);
  this->setHeaderHidden(true);
}

void TreeWidget::updateData(std::shared_ptr<GRM::Element> ref)
{
  this->clear();
  auto tmp = new QTreeWidgetItem(this);
  tmp->setExpanded(true);
  plotTree = new CustomTreeWidgetItem(tmp, ref);
  plotTree->setText(0, tr("root"));
  plotTree->setExpanded(true);

  for (const auto &cur_elem : ref->children())
    {
      updateDataRecursion(cur_elem, plotTree);
    }
}

void TreeWidget::updateDataRecursion(std::shared_ptr<GRM::Element> ref, CustomTreeWidgetItem *parent)
{
  bool skip = false;
  auto *item = new CustomTreeWidgetItem(parent, ref);
  std::string name = ref->localName();

  if (ref->hasAttribute("name")) name += " (" + static_cast<std::string>(ref->getAttribute("name")) + ")";
  item->setText(0, tr(name.c_str()));
  item->setExpanded(true);
  for (const auto &cur_elem : ref->children())
    {
      updateDataRecursion(cur_elem, item);
    }
}

bool TreeWidget::findSelectedItem(CustomTreeWidgetItem *item)
{
  if (!item->isSelected() && item->getRef() != nullptr)
    {
      for (int i = 0; i < item->childCount(); ++i)
        {
          if (findSelectedItem(dynamic_cast<CustomTreeWidgetItem *>(item->child(i)))) break;
        }
    }
  else if (item->getRef() != nullptr)
    {
      auto bbox_id = static_cast<int>(item->getRef()->getAttribute("_bbox_id"));
      auto bbox_xmin = static_cast<double>(item->getRef()->getAttribute("_bbox_xmin"));
      auto bbox_xmax = static_cast<double>(item->getRef()->getAttribute("_bbox_xmax"));
      auto bbox_ymin = static_cast<double>(item->getRef()->getAttribute("_bbox_ymin"));
      auto bbox_ymax = static_cast<double>(item->getRef()->getAttribute("_bbox_ymax"));
      auto *bbox = new Bounding_object(bbox_id, bbox_xmin, bbox_xmax, bbox_ymin, bbox_ymax, item->getRef());

      grplot_widget->set_current_selection(bbox);
      return true;
    }
  return false;
}

void TreeWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  findSelectedItem(plotTree);
  grplot_widget->AttributeEditEvent();
}

void TreeWidget::mousePressEvent(QMouseEvent *event)
{
  QTreeView::mousePressEvent(event);
  findSelectedItem(plotTree);
  grplot_widget->redraw(false);
}

bool TreeWidget::selectItem(std::shared_ptr<GRM::Element> ref, CustomTreeWidgetItem *tree_elem)
{
  auto item = plotTree;
  if (tree_elem) item = tree_elem;
  if (item->getRef() != nullptr && item->getRef() != ref)
    {
      for (int i = 0; i < item->childCount(); ++i)
        {
          if (selectItem(ref, dynamic_cast<CustomTreeWidgetItem *>(item->child(i)))) break;
        }
    }
  else if (item->getRef() == ref)
    {
      item->setSelected(true);
      return true;
    }
  return false;
}
