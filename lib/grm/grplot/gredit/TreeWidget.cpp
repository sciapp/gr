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
  // checkboxes for _selected attribute
  if (ref->hasAttribute("_selected") && static_cast<int>(ref->getAttribute("_selected")))
    {
      item->setCheckState(0, Qt::Checked);
    }
  else
    {
      item->setCheckState(0, Qt::Unchecked);
    }

  for (const auto &cur_elem : ref->children())
    {
      updateDataRecursion(cur_elem, item);
    }
}

bool TreeWidget::checkboxStatusChanged(CustomTreeWidgetItem *item)
{
  bool selected_status =
      item->getRef()->hasAttribute("_selected") && static_cast<int>(item->getRef()->getAttribute("_selected"));
  if (item->getRef()->localName() != "root" &&
      ((item->checkState(0) == 0 && selected_status == true) || (item->checkState(0) == 2 && selected_status == false)))
    {
      // checkbox status got changed
      auto bbox_id = static_cast<int>(item->getRef()->getAttribute("_bbox_id"));
      auto bbox_x_min = static_cast<double>(item->getRef()->getAttribute("_bbox_x_min"));
      auto bbox_x_max = static_cast<double>(item->getRef()->getAttribute("_bbox_x_max"));
      auto bbox_y_min = static_cast<double>(item->getRef()->getAttribute("_bbox_y_min"));
      auto bbox_y_max = static_cast<double>(item->getRef()->getAttribute("_bbox_y_max"));
      std::unique_ptr<Bounding_object> bbox(
          new Bounding_object(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, item->getRef()));
      if (!selected_status)
        {
          grplot_widget->add_current_selection(std::move(bbox));
        }
      else
        {
          const auto &selections = grplot_widget->get_current_selections();
          for (auto it = std::begin(selections); it != std::end(selections); ++it)
            {
              if ((*it)->get_ref() == bbox->get_ref())
                {
                  it = grplot_widget->erase_current_selection(it);
                  break;
                }
            }
        }
      item->getRef()->setAttribute("_selected", !selected_status);
      grplot_widget->redraw(false, false);
    }

  if (item->getRef() != nullptr)
    {
      for (int i = 0; i < item->childCount(); ++i)
        {
          if (checkboxStatusChanged(dynamic_cast<CustomTreeWidgetItem *>(item->child(i)))) break;
        }
    }
  return false;
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
      auto bbox_x_min = static_cast<double>(item->getRef()->getAttribute("_bbox_x_min"));
      auto bbox_x_max = static_cast<double>(item->getRef()->getAttribute("_bbox_x_max"));
      auto bbox_y_min = static_cast<double>(item->getRef()->getAttribute("_bbox_y_min"));
      auto bbox_y_max = static_cast<double>(item->getRef()->getAttribute("_bbox_y_max"));
      auto *bbox = new Bounding_object(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, item->getRef());

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
  grplot_widget->redraw(false, false);
}

void TreeWidget::mouseReleaseEvent(QMouseEvent *event)
{
  QTreeView::mouseReleaseEvent(event);
  checkboxStatusChanged(plotTree);
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
