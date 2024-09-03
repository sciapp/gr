#include "TableWidget.h"

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

  context_data = extractContextNames(context);
  this->setColumnCount(this->col_num);

  for (const auto &entry : context_data)
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

  for (int i = 1; i <= max_rows; i++)
    {
      this->setVerticalHeaderItem(i, new QTableWidgetItem(std::to_string(i).c_str()));
    }

  connect(this, SIGNAL(cellChanged(int, int)), this, SLOT(applyTableChanges(int, int)));
}

void TableWidget::applyTableChanges(int row, int column)
{
  std::string new_value = this->item(row, column)->text().toStdString();

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
          vec[row - 1] = atof(new_value.c_str());
          (*this->context)[context_key] = vec;
        }
      else if ((*this->context)[context_key].intUsed())
        {
          auto vec = GRM::get<std::vector<int>>((*this->context)[context_key]);
          vec[row - 1] = atoi(new_value.c_str());
          (*this->context)[context_key] = vec;
        }
      else
        {
          auto vec = GRM::get<std::vector<std::string>>((*this->context)[context_key]);
          vec[row - 1] = new_value;
          (*this->context)[context_key] = vec;
        }
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
      addValidContextKey(new_value); // add the name to valid keys so it could also be exported and updated

      // get the plain XML-tree and use the string find option to get attr=value -> use this information to select the
      // tree element and update the value with the new context name
      // all that is needed cause u can't search just for the value with the selector
      auto tree_str = GRM::toXML(
          grm_get_document_root(),
          GRM::SerializerOptions{std::string(2, ' '), GRM::SerializerOptions::InternalAttributesFormat::Plain});
      std::string token = "=\"" + std::string(context_key) + "\"";
      while (tree_str.find(token) != std::string::npos)
        {
          int max_attr_length = 50;
          auto pos = tree_str.find(token);
          auto interesting_part = tree_str.substr(pos - max_attr_length, max_attr_length + token.size());
          auto start = interesting_part.find_last_of(" ");
          auto selector_token = interesting_part.substr(start + 1, (max_attr_length - start - 1) + token.size());
          auto attr = interesting_part.substr(start + 1, (max_attr_length - start - 1));
          tree_str = tree_str.substr(pos + token.size(), std::string::npos);

          if (getenv("GRM_DEBUG"))
            fprintf(stderr, "Replace the value of %s with the new user-defined name\n", selector_token.c_str());

          for (const auto &elem : grm_get_document_root()->querySelectorsAll("[" + selector_token + "]"))
            {
              if (static_cast<std::string>(elem->getAttribute(attr)) == std::string(context_key))
                {
                  elem->setAttribute(attr, new_value);
                  (*this->context)[context_key].decrement_key(context_key);
                  break;
                }
            }
        }
    }
}

std::vector<std::string> TableWidget::getContextNames()
{
  return this->context_names;
}
