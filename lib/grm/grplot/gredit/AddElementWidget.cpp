#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QCompleter>
#include "AddElementWidget.hxx"
#include "grm/dom_render/graphics_tree/util.hxx"
#include "../util.hxx"
#include "grm/dom_render/graphics_tree/NotFoundError.hxx"

AddElementWidget::AddElementWidget(GRPlotWidget *widget, QWidget *parent) : QWidget(parent)
{
  grplot_widget = widget;
#if !defined(NO_XERCES_C)
  schema_tree = grplot_widget->getSchemaTree();
#else
  schema_tree = nullptr;
#endif

  auto *add_element_group = new QGroupBox(tr("Add Element"));

  auto *add_element_label = new QLabel(tr("Element:"));
  add_element_combo_box = new QComboBox;
  if (schema_tree != nullptr)
    {
      auto selections = schema_tree->querySelectorsAll("[name]");
      for (const auto &selection : selections)
        {
          if (selection->localName() == "xs:element")
            {
              auto element_name = static_cast<std::string>(selection->getAttribute("name"));
              if (element_name != "root") add_element_combo_box->addItem(tr(element_name.c_str()));
            }
        }
    }
  add_element_combo_box->model()->sort(0);

  auto *select_parent_label = new QLabel(tr("&Parent:"));
  select_parent_combo_box = new QComboBox;

  QObject::connect(add_element_combo_box, SIGNAL(activated(int)), this, SLOT(elementSelected(int)));
  QObject::connect(select_parent_combo_box, SIGNAL(activated(int)), this, SLOT(parentSelected(int)));

  add_element_layout = new QGridLayout;
  add_element_layout->addWidget(add_element_label, 0, 0);
  add_element_layout->addWidget(add_element_combo_box, 0, 1);
  add_element_layout->addWidget(select_parent_label, 1, 0);
  add_element_layout->addWidget(select_parent_combo_box, 1, 1);
  add_element_group->setLayout(add_element_layout);

  select_parent_combo_box->setVisible(false);

  add_attributes_group = new QGroupBox(tr("&Add Attributes"));

  QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
  QObject::connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));

  QGridLayout *layout = new QGridLayout;
  layout->addWidget(add_element_group, 0, 0, 1, 0);
  layout->addWidget(add_attributes_group, 1, 0, 3, 0);
  layout->addWidget(button_box, 5, 0, 1, 2);
  setLayout(layout);

  this->setWindowTitle("Add Element Menu");
}

static void clearLayout(QLayout *layout)
{
  if (layout == nullptr) return;
  QLayoutItem *item;
  while ((item = layout->takeAt(0)))
    {
      if (item->layout())
        {
          clearLayout(item->layout());
          delete item->layout();
        }
      if (item->widget())
        {
          delete item->widget();
        }
      delete item;
    }
}

void AddElementWidget::elementSelected(int i)
{
  auto selected_element_name = add_element_combo_box->itemText(add_element_combo_box->currentIndex()).toStdString();
  if (schema_tree != nullptr)
    {
      parent_vec.clear();
      select_parent_combo_box->clear();
      clearLayout(add_attributes_group->layout());
      delete add_attributes_group->layout();
      select_parent_combo_box->setVisible(false);

      auto global_root = grm_get_document_root();
      auto selections = schema_tree->querySelectorsAll("[ref=" + selected_element_name + "]");
      for (const auto &selection : selections)
        {
          if (selection->localName() == "xs:element")
            {
              int occurs = 0;
              auto parent_name = static_cast<std::string>(
                  selection->parentElement()->parentElement()->parentElement()->getAttribute("name"));
              auto max_occurs = static_cast<std::string>(selection->getAttribute("maxOccurs"));
              auto possible_parents = global_root->querySelectorsAll(parent_name);
              for (const auto &pos_par : possible_parents)
                {
                  auto bbox_id = static_cast<int>(pos_par->getAttribute("_bbox_id"));
                  auto bbox_x_min = static_cast<double>(pos_par->getAttribute("_bbox_x_min"));
                  auto bbox_x_max = static_cast<double>(pos_par->getAttribute("_bbox_x_max"));
                  auto bbox_y_min = static_cast<double>(pos_par->getAttribute("_bbox_y_min"));
                  auto bbox_y_max = static_cast<double>(pos_par->getAttribute("_bbox_y_max"));
                  const auto bbox = BoundingObject(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, pos_par);

                  // check if the new child can be added without passing max occurs
                  for (const auto &child : pos_par->children())
                    {
                      if (child->localName() == selected_element_name) occurs += 1;
                    }

                  if (max_occurs == "unbounded" || occurs < std::stoi(max_occurs))
                    {
                      select_parent_combo_box->addItem(tr(parent_name.c_str()));
                      select_parent_combo_box->setVisible(true);
                      parent_vec.push_back(bbox);
                    }
                }
            }
        }
    }
  select_parent_combo_box->model()->sort(0);
}

void AddElementWidget::parentSelected(int i)
{
  if (schema_tree != nullptr)
    {
      QStringList combo_box_attr = grplot_widget->getComboBoxAttributes();
      QStringList check_box_attr = grplot_widget->getCheckBoxAttributes();

      BoundingObject *tmp = &parent_vec[i];
      grplot_widget->setSelectedParent(tmp);
      attribute_name_vec.clear();
      attribute_type_vec.clear();

      int line = 0;
      QWidget *attr_line_edit;
      QLabel *attr_label;
      auto *add_attributes_layout = new QGridLayout;
      std::string selected_element_name =
          add_element_combo_box->itemText(add_element_combo_box->currentIndex()).toStdString();
      std::shared_ptr<GRM::Element> element;

      auto selections = schema_tree->querySelectorsAll("[name=" + selected_element_name + "]");
      for (const auto &selection : selections)
        {
          if (selection->localName() == "xs:element") element = selection->children()[0];
        }

      for (const auto &child : element->children())
        {
          if (child->localName() == "xs:attribute")
            {
              auto attr_name = static_cast<std::string>(child->getAttribute("name"));
              attribute_name_vec.push_back(attr_name);

              auto type_name = static_cast<std::string>(child->getAttribute("type"));
              attribute_type_vec.push_back(type_name);

              if (combo_box_attr.contains(attr_name.c_str()))
                {
                  attr_line_edit = new QComboBox;
                  static_cast<QComboBox *>(attr_line_edit)->addItem(""); // default all attributes are empty
                  grplot_widget->attributeComboBoxHandler(attr_name, selected_element_name, &attr_line_edit);
                  int index = static_cast<QComboBox *>(attr_line_edit)->count();
                  static_cast<QComboBox *>(attr_line_edit)->setCurrentIndex(index);

                  if (attr_name == "kind" && util::startsWith(selected_element_name, "series_"))
                    {
                      index =
                          static_cast<QComboBox *>(attr_line_edit)->findText(selected_element_name.erase(0, 7).c_str());
                      if (index == -1) index += static_cast<QComboBox *>(attr_line_edit)->count();
                      static_cast<QComboBox *>(attr_line_edit)->setCurrentIndex(index);
                    }
                }
              else if (check_box_attr.contains(attr_name.c_str()))
                {
                  attr_line_edit = new QCheckBox;
                }
              else
                {
                  attr_line_edit = new QLineEdit;
                  static_cast<QLineEdit *>(attr_line_edit)->setPlaceholderText("Placeholder Text");
                  attr_line_edit->setFocus();
                }
              if (child->hasAttribute("use")) attr_name = "*" + attr_name + ":";
              attr_label = new QLabel(tr(attr_name.c_str()));

              add_attributes_layout->addWidget(attr_label, line, 0);
              add_attributes_layout->addWidget(attr_line_edit, line++, 1);

              fields << attr_line_edit;
            }
          else if (child->localName() == "xs:attributeGroup")
            {
              /* when an element contains one or more attributegroups all attributes from these groups must be
               * added */
              std::shared_ptr<GRM::Element> group;
              auto group_name = static_cast<std::string>(child->getAttribute("ref"));

              if (group_name != "colorrep")
                {
                  auto attr_group_selections = schema_tree->querySelectorsAll("[name=" + group_name + "]");
                  for (const auto &selection : attr_group_selections)
                    {
                      if (selection->localName() == "xs:attributeGroup") group = selection;
                    }

                  /* iterate through attribute elements */
                  for (const auto &childchild : group->children())
                    {
                      if (childchild->localName() == "xs:attribute")
                        {
                          auto attr_name = static_cast<std::string>(childchild->getAttribute("name"));
                          attribute_name_vec.push_back(attr_name);
                          auto type_name = static_cast<std::string>(childchild->getAttribute("type"));
                          attribute_type_vec.push_back(type_name);

                          if (combo_box_attr.contains(attr_name.c_str()))
                            {
                              attr_line_edit = new QComboBox;
                              static_cast<QComboBox *>(attr_line_edit)->addItem(""); // default all attributes are empty
                              grplot_widget->attributeComboBoxHandler(attr_name, selected_element_name,
                                                                      &attr_line_edit);
                              int index = static_cast<QComboBox *>(attr_line_edit)->count();
                              static_cast<QComboBox *>(attr_line_edit)->setCurrentIndex(index);

                              if (attr_name == "kind" && util::startsWith(selected_element_name, "series_"))
                                {
                                  index = static_cast<QComboBox *>(attr_line_edit)
                                              ->findText(selected_element_name.erase(0, 7).c_str());
                                  if (index == -1) index += static_cast<QComboBox *>(attr_line_edit)->count();
                                  static_cast<QComboBox *>(attr_line_edit)->setCurrentIndex(index);
                                }
                            }
                          else if (check_box_attr.contains(attr_name.c_str()))
                            {
                              attr_line_edit = new QCheckBox;
                            }
                          else
                            {
                              attr_line_edit = new QLineEdit;
                              static_cast<QLineEdit *>(attr_line_edit)->setPlaceholderText("Placeholder Text");
                              attr_line_edit->setFocus();
                            }
                          if (childchild->hasAttribute("use")) attr_name = "*" + attr_name + ":";
                          attr_label = new QLabel(tr(attr_name.c_str()));

                          add_attributes_layout->addWidget(attr_label, line, 0);
                          add_attributes_layout->addWidget(attr_line_edit, line++, 1);

                          fields << attr_line_edit;
                        }
                    }
                }
              else
                {
                  /* special case for colorrep cause there are way to many attributes inside the attributegroup
                   */
                  attribute_name_vec.emplace_back("Colorrep-index");
                  attribute_type_vec.emplace_back("xs:string");

                  attr_label = new QLabel(tr("Colorrep-index:"));
                  attr_line_edit = new QLineEdit;
                  static_cast<QLineEdit *>(attr_line_edit)->setPlaceholderText("Placeholder Text");
                  attr_line_edit->setFocus();

                  add_attributes_layout->addWidget(attr_label, line, 0);
                  add_attributes_layout->addWidget(attr_line_edit, line++, 1);

                  fields << attr_line_edit;

                  attribute_name_vec.emplace_back("Colorrep-value");
                  attribute_type_vec.emplace_back("xs:string");

                  attr_label = new QLabel(tr("Colorrep-value:"));
                  attr_line_edit = new QLineEdit;
                  static_cast<QLineEdit *>(attr_line_edit)->setPlaceholderText("Placeholder Text");
                  attr_line_edit->setFocus();

                  add_attributes_layout->addWidget(attr_label, line, 0);
                  add_attributes_layout->addWidget(attr_line_edit, line++, 1);

                  fields << attr_line_edit;
                }
            }
        }

      if (!add_attributes_group->layout())
        {
          auto scroll_area_content = new QWidget;
          scroll_area_content->setLayout(add_attributes_layout);
          auto scroll_area = new QScrollArea;
          scroll_area = new QScrollArea;
          scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
          scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
          scroll_area->setWidgetResizable(true);
          scroll_area->setWidget(scroll_area_content);

          auto group_box_layout = new QVBoxLayout;
          group_box_layout->addWidget(scroll_area);
          group_box_layout->setContentsMargins(2, 2, 2, 2);
          add_attributes_group->setLayout(group_box_layout);
        }
      grplot_widget->redraw();
    }
}

void AddElementWidget::reject()
{
  grplot_widget->setSelectedParent(nullptr);
  this->close();
}

void AddElementWidget::accept()
{
  QLabel *msg;
  BoundingObject *grplot_ref = grplot_widget->getSelectedParent();
  std::shared_ptr<GRM::Render> render = grm_get_render();
  bool error = false, auto_update;
  auto new_element =
      render->createElement(add_element_combo_box->itemText(add_element_combo_box->currentIndex()).toStdString());

  render->getAutoUpdate(&auto_update);
  render->setAutoUpdate(false);
  for (int i = 0; i < attribute_name_vec.size(); i++)
    {
      auto &field = *fields[i];
      if (typeid(field) == typeid(QLineEdit) && static_cast<QLineEdit *>(fields[i])->isModified())
        {
          {
            if (attribute_name_vec[i] == "Colorrep-index") /* special case for colorrep attribute */
              {
                new_element->setAttribute("colorrep." + static_cast<QLineEdit *>(fields[i])->text().toStdString(),
                                          static_cast<QLineEdit *>(fields[i + 1])->text().toStdString());
              }
            else if (attribute_name_vec[i] != "Colorrep-value")
              {
                auto value = static_cast<QLineEdit *>(fields[i])->text().toStdString();
                if ((attribute_type_vec[i] == "xs:string" || attribute_type_vec[i] == "strint") &&
                    !util::isDigits(value))
                  {
                    new_element->setAttribute(attribute_name_vec[i], value);
                  }
                else if (attribute_type_vec[i] == "xs:double" && util::isNumber(value))
                  {
                    new_element->setAttribute(attribute_name_vec[i], std::stod(value));
                  }
                else if ((attribute_type_vec[i] == "xs:integer" || attribute_type_vec[i] == "strint") &&
                         util::isDigits(value))
                  {
                    new_element->setAttribute(attribute_name_vec[i], std::stoi(value));
                  }
                else
                  {
                    fprintf(stderr, "Invalid value %s for attribute %s with type %s\n", value.c_str(),
                            attribute_name_vec[i].c_str(), attribute_type_vec[i].c_str());
                  }
              }
          }
        }
      else if (typeid(field) == typeid(QComboBox))
        {
          int index = static_cast<QComboBox *>(fields[i])->currentIndex();

          const auto value = static_cast<QComboBox *>(fields[i])->itemText(index).toStdString();
          if (!value.empty())
            {
              grplot_widget->attributeSetForComboBox(attribute_type_vec[i], new_element, value, attribute_name_vec[i]);
            }
        }
      else if (typeid(field) == typeid(QCheckBox))
        {
          new_element->setAttribute(attribute_name_vec[i], static_cast<QCheckBox *>(fields[i])->isChecked());
        }
    }

  if (grplot_ref == nullptr)
    {
      msg = new QLabel("Element could not be created. Missing parent element.");
      error = true;
    }
  else if (!new_element->hasAttributes())
    {
      msg = new QLabel("Element could not be created. Attributes are missing.");
      error = true;
    }
  else
    {
      grplot_widget->createHistoryElement();
      grplot_ref->getRef()->append(new_element);
      if (!grm_validate())
        {
          msg = new QLabel("Element could not be created. Missing required attributes.");
          grplot_widget->removeHistoryElement();
          error = true;
        }
      else
        {
          try
            {
              grm_process_tree();
            }
          catch (NotFoundError &err)
            {
              msg = new QLabel("Element could not be created. Missing or wrong attributes.");
              grplot_widget->removeHistoryElement();
              error = true;
            }
        }
      if (error) new_element->remove();
    }
  if (!error)
    {
      parent_vec.clear();
      select_parent_combo_box->clear();
      add_element_combo_box->setCurrentIndex(0);
      clearLayout(add_attributes_group->layout());
      delete add_attributes_group->layout();
      attribute_name_vec.clear();
      attribute_type_vec.clear();
      fields.clear();

      grplot_widget->setSelectedParent(nullptr);
      this->close();
      new_element->parentElement()->setAttribute("_bbox_id", -1);
      render->setAutoUpdate(auto_update);
      grplot_widget->redraw();
    }
  else
    {
      QDialog dialog(this);
      QFormLayout form(&dialog);
      dialog.setWindowTitle("Warning");

      form.addWidget(msg);

      QDialogButtonBox button_box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
      form.addRow(&button_box);
      QObject::connect(&button_box, SIGNAL(accepted()), &dialog, SLOT(accept()));
      QObject::connect(&button_box, SIGNAL(rejected()), &dialog, SLOT(reject()));

      dialog.exec();
      render->setAutoUpdate(auto_update);
    }
}
