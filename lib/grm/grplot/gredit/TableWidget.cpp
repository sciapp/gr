#include "TableWidget.hxx"

#include <QHeaderView>
#include <QObject>
#include <grm/dom_render/context.hxx>


TableWidget::TableWidget(GRPlotWidget *widget, QWidget *parent) : QTableWidget(parent)
{
  grplot_widget = widget;
  this->setWindowTitle("DOM-Tree Data-Context Viewer");
  this->setRowCount(2);
  this->setVerticalHeaderItem(0, new QTableWidgetItem("Context-Key"));
}

std::map<std::string, std::list<std::string>>
TableWidget::extractContextNames(const std::shared_ptr<GRM::Context> context)
{
  int col_cnt = 0;
  std::map<std::string, std::list<std::string>> context_data;
  std::vector<std::string> cntxt_names;

  context_data = grm_get_context_data();

  for (const auto &entry : context_data)
    {
      col_cnt += 1;
      cntxt_names.push_back(entry.first);
    }
  this->col_num = col_cnt;
  this->context_names = cntxt_names;
  return context_data;
}

void TableWidget::updateData(const std::shared_ptr<GRM::Context> context)
{
  int max_rows = 0, col = 0;
  std::map<std::string, std::list<std::string>> context_data;

  // disconnect so that a reread wont trigger applyTableChanges
  disconnect(this, SIGNAL(cellChanged(int, int)), this, SLOT(applyTableChanges(int, int)));
  this->context = context;
  this->context_attributes = GRM::getContextAttributes();

  context_data = extractContextNames(context);
  this->setColumnCount(this->col_num);

  for (const auto &entry : context_data)
    {
      bool skip_elem = false;
      auto tree_str = GRM::toXML(
          grm_get_document_root(),
          GRM::SerializerOptions{std::string(2, ' '), GRM::SerializerOptions::InternalAttributesFormat::PLAIN});
      std::string token = "=\"" + std::string(entry.first.c_str()) + "\"";

      while (tree_str.find(token) != std::string::npos)
        {
          int max_attr_length = 50;
          auto pos = tree_str.find(token);
          auto interesting_part =
              tree_str.substr(std::max<int>(0, pos - max_attr_length), max_attr_length + token.size());
          auto start = interesting_part.find_last_of(" ");
          auto selector_token = interesting_part.substr(start + 1, (max_attr_length - start - 1) + token.size());
          tree_str = tree_str.substr(pos + token.size(), std::string::npos);

          auto advanced_editor = grplot_widget->getEnableAdvancedEditor();
          for (const auto &elem : grm_get_document_root()->querySelectorsAll("[" + selector_token + "]"))
            {
              auto elem_name = elem->localName();
              if (!advanced_editor &&
                  (elem_name == "polyline" || elem_name == "polymarker" || elem_name == "draw_rect" ||
                   elem_name == "polyline_3d" || elem_name == "polymarker_3d" || elem_name == "fill_rect" ||
                   elem_name == "cell_array" || elem_name == "nonuniform_cell_array" ||
                   elem_name == "polar_cell_array" || elem_name == "nonuniform_polar_cell_array" ||
                   elem_name == "draw_image" || elem_name == "draw_arc" || elem_name == "fill_arc" ||
                   elem_name == "fill_area"))
                {
                  skip_elem = true;
                }
              else
                {
                  skip_elem = false;
                }
            }
          if (skip_elem) break;
        }

      if (!skip_elem)
        {
          int row = 1;
          this->setItem(0, col, new QTableWidgetItem(entry.first.c_str()));
          max_rows = std::max(max_rows, (int)entry.second.size() + 1);
          this->setRowCount(max_rows);
          for (const auto &str : entry.second)
            {
              this->setItem(row, col, new QTableWidgetItem(str.c_str()));
              row += 1;
            }
          col += 1;
        }
    }

  for (int i = 1; i <= max_rows; i++)
    {
      this->setVerticalHeaderItem(i, new QTableWidgetItem(std::to_string(i).c_str()));
    }
  this->clearSelection();

  connect(this, SIGNAL(cellChanged(int, int)), this, SLOT(applyTableChanges(int, int)));
  connect(this, SIGNAL(cellClicked(int, int)), this, SLOT(showUsagesOfContext(int, int)));
}

void TableWidget::applyTableChanges(int row, int column)
{
  auto new_value = this->item(row, column)->text().toStdString();

  if (getenv("GRM_DEBUG"))
    fprintf(stderr, "Detected change at (%i/%i) with value '%s'. Old value was '%s'\n", row, column, new_value.c_str(),
            this->context_names[column].c_str());

  if (row != 0)
    {
      // data has been changed -> apply these changes to the context
      auto context_key = this->item(0, column)->text().toStdString();

      if ((*this->context)[context_key].doubleUsed())
        {
          auto vec = GRM::get<std::vector<double>>((*this->context)[context_key]);
          if (vec.size() >= row)
            {
              vec[row - 1] = atof(new_value.c_str());
              (*this->context)[context_key] = vec;
            }
          else
            {
              // only allow to edit non empty lines to prevent multiple complications with size mismatches
              this->item(row, column)->setText("");
            }
        }
      else if ((*this->context)[context_key].intUsed())
        {
          auto vec = GRM::get<std::vector<int>>((*this->context)[context_key]);
          if (vec.size() >= row)
            {
              vec[row - 1] = atoi(new_value.c_str());
              (*this->context)[context_key] = vec;
            }
          else
            {
              // only allow to edit non empty lines to prevent multiple complications with size mismatches
              this->item(row, column)->setText("");
            }
        }
      else
        {
          auto vec = GRM::get<std::vector<std::string>>((*this->context)[context_key]);
          if (vec.size() >= row)
            {
              vec[row - 1] = new_value;
              (*this->context)[context_key] = vec;
            }
          else
            {
              // only allow to edit non empty lines to prevent multiple complications with size mismatches
              this->item(row, column)->setText("");
            }
        }

      // update the element so the data changes get applied to the picture
      for (const auto &referenced_elem : referenced_attributes)
        {
          referenced_elem.getRef()->setAttribute("_update_required", true);
        }
      this->grplot_widget->redraw();
    }
  else
    {
      // the name of the context got changed -> replace the key inside the context and update tree attributes
      auto context_key = this->context_names[column].c_str();

      if ((*this->context)[context_key].doubleUsed())
        {
          auto vec = GRM::get<std::vector<double>>((*this->context)[context_key]);
          (*this->context)[new_value] = vec;
        }
      else if ((*this->context)[context_key].intUsed())
        {
          auto vec = GRM::get<std::vector<int>>((*this->context)[context_key]);
          (*this->context)[new_value] = vec;
        }
      else
        {
          auto vec = GRM::get<std::vector<std::string>>((*this->context)[context_key]);
          (*this->context)[new_value] = vec;
        }
      GRM::addValidContextKey(new_value); // add the name to valid keys so it could also be exported and updated

      // get the plain XML-tree and use the string find option to get attr=value -> use this information to select the
      // tree element and update the value with the new context name
      // all that is needed cause u can't search just for the value with the selector
      auto tree_str = GRM::toXML(
          grm_get_document_root(),
          GRM::SerializerOptions{std::string(2, ' '), GRM::SerializerOptions::InternalAttributesFormat::PLAIN});
      std::string token = "=\"" + std::string(context_key) + "\"";
      while (tree_str.find(token) != std::string::npos)
        {
          int max_attr_length = 50;
          auto pos = tree_str.find(token);
          auto interesting_part =
              tree_str.substr(std::max<int>(0, pos - max_attr_length), max_attr_length + token.size());
          auto start = interesting_part.find_last_of(" ");
          auto selector_token = interesting_part.substr(start + 1, (max_attr_length - start - 1) + token.size());
          auto attr = interesting_part.substr(start + 1, (max_attr_length - start - 1));
          tree_str = tree_str.substr(pos + token.size(), std::string::npos);

          if (getenv("GRM_DEBUG"))
            fprintf(stderr, "Replace the value of %s with the new user-defined name\n", selector_token.c_str());

          if (std::find(this->context_attributes.begin(), this->context_attributes.end(), attr) ==
              this->context_attributes.end())
            continue;
          for (const auto &elem : grm_get_document_root()->querySelectorsAll("[" + selector_token + "]"))
            {
              if (static_cast<std::string>(elem->getAttribute(attr)) == std::string(context_key))
                {
                  elem->setAttribute(attr, new_value);
                  (*this->context)[context_key].decrementKey(context_key);
                  break;
                }
            }
        }
    }
}

void TableWidget::showUsagesOfContext(int row, int column)
{
  referenced_attributes.clear();
  if (row != 0) row = 0; // complety columns highlight this way not only the header line

  auto context_ref_name = this->item(row, column)->text().toStdString();

  // get the plain XML-tree and use the string find option to get attr=value -> use this information to select the
  // tree element and update the value with the new context name
  // all that is needed cause u can't search just for the value with the selector
  auto tree_str =
      GRM::toXML(grm_get_document_root(),
                 GRM::SerializerOptions{std::string(2, ' '), GRM::SerializerOptions::InternalAttributesFormat::PLAIN});
  std::string token = "=\"" + std::string(context_ref_name) + "\"";
  while (tree_str.find(token) != std::string::npos)
    {
      int max_attr_length = 50;
      auto pos = tree_str.find(token);
      auto interesting_part = tree_str.substr(std::max<int>(0, pos - max_attr_length), max_attr_length + token.size());
      auto start = interesting_part.find_last_of(" ");
      auto selector_token = interesting_part.substr(start + 1, (max_attr_length - start - 1) + token.size());
      auto attr = interesting_part.substr(start + 1, (max_attr_length - start - 1));
      tree_str = tree_str.substr(pos + token.size(), std::string::npos);

      if (std::find(this->context_attributes.begin(), this->context_attributes.end(), attr) ==
          this->context_attributes.end())
        continue;
      for (const auto &elem : grm_get_document_root()->querySelectorsAll("[" + selector_token + "]"))
        {
          if (static_cast<std::string>(elem->getAttribute(attr)) == std::string(context_ref_name))
            {
              bool skip = false;
              auto bbox_id = static_cast<int>(elem->getAttribute("_bbox_id"));
              auto bbox_x_min = static_cast<double>(elem->getAttribute("_bbox_x_min"));
              auto bbox_x_max = static_cast<double>(elem->getAttribute("_bbox_x_max"));
              auto bbox_y_min = static_cast<double>(elem->getAttribute("_bbox_y_min"));
              auto bbox_y_max = static_cast<double>(elem->getAttribute("_bbox_y_max"));
              const BoundingObject bbox = BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, elem);
              for (const auto &vec_elem : referenced_attributes)
                {
                  if (vec_elem.getRef() == elem)
                    {
                      skip = true;
                      break;
                    }
                }
              if (!skip) referenced_attributes.emplace_back(bbox);
            }
        }
    }
  this->grplot_widget->setReferencedElements(this->referenced_attributes);
  this->grplot_widget->redraw();
}

std::vector<std::string> TableWidget::getContextNames()
{
  return this->context_names;
}
