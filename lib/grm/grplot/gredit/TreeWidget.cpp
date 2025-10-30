#include <QTreeWidgetItem>
#include <queue>
#include <QHeaderView>
#include "TreeWidget.hxx"


TreeWidget::TreeWidget(GRPlotWidget *widget, QWidget *parent) : QTreeWidget(parent)
{
  grplot_widget = widget;
  plot_tree = nullptr;
  this->setWindowTitle("DOM-Tree Elements");
  this->setColumnCount(1);
  this->header()->setSectionResizeMode(QHeaderView::Stretch);
  this->setHeaderHidden(true);
}

void TreeWidget::updateData(std::shared_ptr<GRM::Element> ref)
{
  if (plot_tree != nullptr && !cleared) checkIfCollapsed(plot_tree->getRef(), plot_tree);

  this->clear();

  plot_tree = new CustomTreeWidgetItem(this, ref);
  plot_tree->setText(0, tr(""));
  plot_tree->setExpanded(true);

  for (const auto &cur_elem : ref->children())
    {
      updateDataRecursion(cur_elem, plot_tree);
    }
  cleared = false;
}

void TreeWidget::updateDataRecursion(std::shared_ptr<GRM::Element> ref, CustomTreeWidgetItem *parent)
{
  auto elem_name = ref->localName();
  auto advanced_editor = grplot_widget->getEnableAdvancedEditor();
  if (!advanced_editor &&
      (elem_name == "polyline" || elem_name == "polymarker" || elem_name == "draw_rect" || elem_name == "polyline_3d" ||
       elem_name == "polymarker_3d" || elem_name == "fill_rect" || elem_name == "cell_array" ||
       elem_name == "nonuniform_cell_array" || elem_name == "polar_cell_array" ||
       elem_name == "nonuniform_polar_cell_array" || elem_name == "draw_image" || elem_name == "draw_arc" ||
       elem_name == "fill_arc" || elem_name == "fill_area"))
    return;
  auto *item = new CustomTreeWidgetItem(parent, ref);
  std::string name = ref->localName();

  if (ref->hasAttribute("name")) name += " (" + static_cast<std::string>(ref->getAttribute("name")) + ")";
  item->setText(0, tr(name.c_str()));

  item->setExpanded(true);
  if (!contract_elements.empty())
    {
      for (const auto &elem : contract_elements)
        {
          auto elem_locked = elem.lock();
          if (elem_locked == ref)
            {
              item->setExpanded(false);
              break;
            }
        }
    }

  // checkboxes for _selected attribute
  if (advanced_editor || (elem_name == "figure" || elem_name == "plot" || elem_name == "layout_grid" ||
                          elem_name == "layout_grid_element" || elem_name == "colorbar" || elem_name == "label" ||
                          elem_name == "titles_3d" || elem_name == "text" || elem_name == "central_region" ||
                          elem_name == "side_region" || elem_name == "marginal_heatmap_plot" || elem_name == "legend" ||
                          elem_name == "text_region" || elem_name == "overlay_element"))
    {
      if (elem_name != "coordinate_system" &&
          !(elem_name == "layout_grid" && item->getRef()->parentElement()->localName() != "layout_grid"))
        {
          if (ref->hasAttribute("_selected_for_move") && static_cast<int>(ref->getAttribute("_selected_for_move")))
            {
              item->setCheckState(0, Qt::Checked);
            }
          else
            {
              item->setCheckState(0, Qt::Unchecked);
            }
        }
    }
  parent->addChild(item);

  for (const auto &cur_elem : ref->children())
    {
      updateDataRecursion(cur_elem, item);
    }
}

bool TreeWidget::checkboxStatusChanged(CustomTreeWidgetItem *item)
{
  if (item->getRef() == nullptr) return false;
  bool selected_status = item->getRef()->hasAttribute("_selected_for_move") &&
                         static_cast<int>(item->getRef()->getAttribute("_selected_for_move"));
  if ((item->getRef()->localName() != "root" && item->getRef()->localName() != "coordinate_system" &&
       (item->getRef()->localName() != "layout_grid" &&
        item->getRef()->parentElement()->localName() != "layout_grid")) &&
      ((item->checkState(0) == 0 && selected_status == true) || (item->checkState(0) == 2 && selected_status == false)))
    {
      // checkbox status got changed
      auto bbox_id = static_cast<int>(item->getRef()->getAttribute("_bbox_id"));
      auto bbox_x_min = static_cast<double>(item->getRef()->getAttribute("_bbox_x_min"));
      auto bbox_x_max = static_cast<double>(item->getRef()->getAttribute("_bbox_x_max"));
      auto bbox_y_min = static_cast<double>(item->getRef()->getAttribute("_bbox_y_min"));
      auto bbox_y_max = static_cast<double>(item->getRef()->getAttribute("_bbox_y_max"));
      std::unique_ptr<BoundingObject> bbox(
          new BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, item->getRef()));
      if (!selected_status)
        {
          grplot_widget->addCurrentSelection(std::move(bbox));
        }
      else
        {
          const auto &selections = grplot_widget->getCurrentSelections();
          for (auto it = std::begin(selections); it != std::end(selections); ++it)
            {
              if ((*it)->getRef() == bbox->getRef())
                {
                  it = grplot_widget->eraseCurrentSelection(it);
                  break;
                }
            }
        }
      item->getRef()->setAttribute("_selected_for_move", !selected_status);
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
      auto *bbox = new BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, item->getRef());

      grplot_widget->setCurrentSelection(bbox);
      return true;
    }
  return false;
}

void TreeWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  findSelectedItem(plot_tree);
  grplot_widget->attributeEditEvent();
}

void TreeWidget::mousePressEvent(QMouseEvent *event)
{
  QTreeView::mousePressEvent(event);
  findSelectedItem(plot_tree);
  grplot_widget->redraw(false, false);
}

void TreeWidget::mouseReleaseEvent(QMouseEvent *event)
{
  QTreeView::mouseReleaseEvent(event);
  checkboxStatusChanged(plot_tree);
}

bool TreeWidget::selectItem(std::shared_ptr<GRM::Element> ref, CustomTreeWidgetItem *tree_elem)
{
  auto item = plot_tree;
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

void TreeWidget::clearContractElements()
{
  contract_elements.clear();
  cleared = true;
}

void TreeWidget::checkIfCollapsed(std::shared_ptr<GRM::Element> ref, CustomTreeWidgetItem *tree_elem)
{
  if (!tree_elem->isExpanded()) contract_elements.emplace_back(ref);

  for (int i = 0; i < tree_elem->childCount(); i++)
    {
      auto cur_elem = tree_elem->child(i);
      checkIfCollapsed(((CustomTreeWidgetItem *)cur_elem)->getRef(), (CustomTreeWidgetItem *)cur_elem);
    }
}
