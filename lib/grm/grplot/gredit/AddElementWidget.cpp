#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QCompleter>
#include "AddElementWidget.h"
#include "grm/dom_render/graphics_tree/util.hxx"
#include "../util.hxx"
#include "grm/dom_render/graphics_tree/NotFoundError.hxx"

AddElementWidget::AddElementWidget(GRPlotWidget *widget, QWidget *parent) : QWidget(parent)
{
  grplot_widget = widget;
#if !defined(NO_XERCES_C)
  schema_tree = grplot_widget->get_schema_tree();
#else
  schema_tree = nullptr;
#endif

  auto *addElementGroup = new QGroupBox(tr("Add Element"));

  auto *addElementLabel = new QLabel(tr("Element:"));
  addElementComboBox = new QComboBox;
  if (schema_tree != nullptr)
    {
      auto selections = schema_tree->querySelectorsAll("[name]");
      for (const auto &selection : selections)
        {
          if (selection->localName() == "xs:element")
            {
              auto element_name = static_cast<std::string>(selection->getAttribute("name"));
              if (element_name != "root") addElementComboBox->addItem(tr(element_name.c_str()));
            }
        }
    }
  addElementComboBox->model()->sort(0);

  auto *selectParentLabel = new QLabel(tr("Parent:"));
  selectParentComboBox = new QComboBox;

  QObject::connect(addElementComboBox, SIGNAL(activated(int)), this, SLOT(elementSelected(int)));
  QObject::connect(selectParentComboBox, SIGNAL(activated(int)), this, SLOT(parentSelected(int)));

  addElementLayout = new QGridLayout;
  addElementLayout->addWidget(addElementLabel, 0, 0);
  addElementLayout->addWidget(addElementComboBox, 0, 1);
  addElementLayout->addWidget(selectParentLabel, 1, 0);
  addElementLayout->addWidget(selectParentComboBox, 1, 1);
  addElementGroup->setLayout(addElementLayout);

  selectParentComboBox->setVisible(false);

  addAttributesGroup = new QGroupBox(tr("Add Attributes"));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QGridLayout *layout = new QGridLayout;
  layout->addWidget(addElementGroup, 0, 0, 1, 0);
  layout->addWidget(addAttributesGroup, 1, 0, 3, 0);
  layout->addWidget(buttonBox, 5, 0, 1, 2);
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
  auto selected_element_name = addElementComboBox->itemText(addElementComboBox->currentIndex()).toStdString();
  if (schema_tree != nullptr)
    {
      parent_vec.clear();
      selectParentComboBox->clear();
      clearLayout(addAttributesGroup->layout());
      delete addAttributesGroup->layout();
      selectParentComboBox->setVisible(false);

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
                  const Bounding_object bbox =
                      Bounding_object(bbox_id, bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max, pos_par);

                  // check if the new child can be added without passing max occurs
                  for (const auto &child : pos_par->children())
                    {
                      if (child->localName() == selected_element_name) occurs += 1;
                    }

                  if (max_occurs == "unbounded" || occurs < std::stoi(max_occurs))
                    {
                      selectParentComboBox->addItem(tr(parent_name.c_str()));
                      selectParentComboBox->setVisible(true);
                      parent_vec.push_back(bbox);
                    }
                }
            }
        }
    }
  selectParentComboBox->model()->sort(0);
}

void AddElementWidget::parentSelected(int i)
{
  if (schema_tree != nullptr)
    {
      QStringList combo_box_attr = grplot_widget->getComboBoxAttributes();
      QStringList check_box_attr = grplot_widget->getCheckBoxAttributes();

      Bounding_object *tmp = &parent_vec[i];
      grplot_widget->set_selected_parent(tmp);
      attribute_name_vec.clear();
      attribute_type_vec.clear();

      int line = 0;
      QWidget *attrLineEdit;
      QLabel *attrLabel;
      auto *addAttributesLayout = new QGridLayout;
      std::string selectedElementName = addElementComboBox->itemText(addElementComboBox->currentIndex()).toStdString();
      std::shared_ptr<GRM::Element> element;

      auto selections = schema_tree->querySelectorsAll("[name=" + selectedElementName + "]");
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
                  attrLineEdit = new QComboBox;
                  ((QComboBox *)attrLineEdit)->addItem(""); // default all attributes are empty
                  grplot_widget->attributeComboBoxHandler(attr_name, selectedElementName, &attrLineEdit);
                  int index = ((QComboBox *)attrLineEdit)->count();
                  ((QComboBox *)attrLineEdit)->setCurrentIndex(index);

                  if (attr_name == "kind" && util::startsWith(selectedElementName, "series_"))
                    {
                      int index = ((QComboBox *)attrLineEdit)->findText(selectedElementName.erase(0, 7).c_str());
                      if (index == -1) index += ((QComboBox *)attrLineEdit)->count();
                      ((QComboBox *)attrLineEdit)->setCurrentIndex(index);
                    }
                }
              else if (check_box_attr.contains(attr_name.c_str()))
                {
                  attrLineEdit = new QCheckBox;
                }
              else
                {
                  attrLineEdit = new QLineEdit;
                  ((QLineEdit *)attrLineEdit)->setPlaceholderText("Placeholder Text");
                  ((QLineEdit *)attrLineEdit)->setFocus();
                }
              if (child->hasAttribute("use")) attr_name = "*" + attr_name + ":";
              attrLabel = new QLabel(tr(attr_name.c_str()));

              addAttributesLayout->addWidget(attrLabel, line, 0);
              addAttributesLayout->addWidget(attrLineEdit, line++, 1);

              fields << attrLineEdit;
            }
          else if (child->localName() == "xs:attributegroup")
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
                      if (selection->localName() == "xs:attributegroup") group = selection;
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
                              attrLineEdit = new QComboBox;
                              ((QComboBox *)attrLineEdit)->addItem(""); // default all attributes are empty
                              grplot_widget->attributeComboBoxHandler(attr_name, selectedElementName, &attrLineEdit);
                              int index = ((QComboBox *)attrLineEdit)->count();
                              ((QComboBox *)attrLineEdit)->setCurrentIndex(index);

                              if (attr_name == "kind" && util::startsWith(selectedElementName, "series_"))
                                {
                                  int index =
                                      ((QComboBox *)attrLineEdit)->findText(selectedElementName.erase(0, 7).c_str());
                                  if (index == -1) index += ((QComboBox *)attrLineEdit)->count();
                                  ((QComboBox *)attrLineEdit)->setCurrentIndex(index);
                                }
                            }
                          else if (check_box_attr.contains(attr_name.c_str()))
                            {
                              attrLineEdit = new QCheckBox;
                            }
                          else
                            {
                              attrLineEdit = new QLineEdit;
                              ((QLineEdit *)attrLineEdit)->setPlaceholderText("Placeholder Text");
                              ((QLineEdit *)attrLineEdit)->setFocus();
                            }
                          if (childchild->hasAttribute("use")) attr_name = "*" + attr_name + ":";
                          attrLabel = new QLabel(tr(attr_name.c_str()));

                          addAttributesLayout->addWidget(attrLabel, line, 0);
                          addAttributesLayout->addWidget(attrLineEdit, line++, 1);

                          fields << attrLineEdit;
                        }
                    }
                }
              else
                {
                  /* special case for colorrep cause there are way to many attributes inside the attributegroup
                   */
                  attribute_name_vec.emplace_back("Colorrep-index");
                  attribute_type_vec.emplace_back("xs:string");

                  attrLabel = new QLabel(tr("Colorrep-index:"));
                  attrLineEdit = new QLineEdit;
                  ((QLineEdit *)attrLineEdit)->setPlaceholderText("Placeholder Text");
                  ((QLineEdit *)attrLineEdit)->setFocus();

                  addAttributesLayout->addWidget(attrLabel, line, 0);
                  addAttributesLayout->addWidget(attrLineEdit, line++, 1);

                  fields << attrLineEdit;

                  attribute_name_vec.emplace_back("Colorrep-value");
                  attribute_type_vec.emplace_back("xs:string");

                  attrLabel = new QLabel(tr("Colorrep-value:"));
                  attrLineEdit = new QLineEdit;
                  ((QLineEdit *)attrLineEdit)->setPlaceholderText("Placeholder Text");
                  ((QLineEdit *)attrLineEdit)->setFocus();

                  addAttributesLayout->addWidget(attrLabel, line, 0);
                  addAttributesLayout->addWidget(attrLineEdit, line++, 1);

                  fields << attrLineEdit;
                }
            }
        }

      if (!addAttributesGroup->layout())
        {
          auto scrollAreaContent = new QWidget;
          scrollAreaContent->setLayout(addAttributesLayout);
          auto scrollArea = new QScrollArea;
          scrollArea = new QScrollArea;
          scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
          scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
          scrollArea->setWidgetResizable(true);
          scrollArea->setWidget(scrollAreaContent);

          auto groupBoxLayout = new QVBoxLayout;
          groupBoxLayout->addWidget(scrollArea);
          addAttributesGroup->setLayout(groupBoxLayout);
        }
      grplot_widget->redraw();
    }
}

void AddElementWidget::reject()
{
  grplot_widget->set_selected_parent(nullptr);
  this->close();
}

void AddElementWidget::accept()
{
  QLabel *msg;
  Bounding_object *grplot_ref = grplot_widget->get_selected_parent();
  std::shared_ptr<GRM::Render> render = grm_get_render();
  bool error = false, auto_update;
  auto new_element =
      render->createElement(addElementComboBox->itemText(addElementComboBox->currentIndex()).toStdString());

  render->getAutoUpdate(&auto_update);
  render->setAutoUpdate(false);
  for (int i = 0; i < attribute_name_vec.size(); i++)
    {
      auto &field = *fields[i];
      if (typeid(field) == typeid(QLineEdit) && ((QLineEdit *)fields[i])->isModified())
        {
          {
            if (attribute_name_vec[i] == "Colorrep-index") /* special case for colorrep attribute */
              {
                new_element->setAttribute("colorrep." + ((QLineEdit *)fields[i])->text().toStdString(),
                                          ((QLineEdit *)fields[i + 1])->text().toStdString());
              }
            else if (attribute_name_vec[i] != "Colorrep-value")
              {
                std::string value = ((QLineEdit *)fields[i])->text().toStdString();
                if (attribute_type_vec[i] == "xs:string" ||
                    (attribute_type_vec[i] == "strint" && !util::is_digits(value)))
                  {
                    new_element->setAttribute(attribute_name_vec[i], value);
                  }
                else if (attribute_type_vec[i] == "xs:double")
                  {
                    new_element->setAttribute(attribute_name_vec[i], std::stod(value));
                  }
                else if (attribute_type_vec[i] == "xs:integer" ||
                         (attribute_type_vec[i] == "strint" && util::is_digits(value)))
                  {
                    new_element->setAttribute(attribute_name_vec[i], std::stoi(value));
                  }
              }
          }
        }
      else if (typeid(field) == typeid(QComboBox))
        {
          int index = ((QComboBox *)fields[i])->currentIndex();

          const std::string value = ((QComboBox *)fields[i])->itemText(index).toStdString();
          if (!value.empty())
            {
              grplot_widget->attributeSetForComboBox(attribute_type_vec[i], new_element, value, attribute_name_vec[i]);
            }
        }
      else if (typeid(field) == typeid(QCheckBox))
        {
          new_element->setAttribute(attribute_name_vec[i], ((QCheckBox *)fields[i])->isChecked());
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
      grplot_ref->get_ref()->append(new_element);
      if (!grm_validate())
        {
          msg = new QLabel("Element could not be created. Missing required attributes.");
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
              error = true;
            }
        }
      if (error) new_element->remove();
    }
  if (!error)
    {
      parent_vec.clear();
      selectParentComboBox->clear();
      addElementComboBox->setCurrentIndex(0);
      clearLayout(addAttributesGroup->layout());
      delete addAttributesGroup->layout();
      attribute_name_vec.clear();
      attribute_type_vec.clear();
      fields.clear();

      grplot_widget->set_selected_parent(nullptr);
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

      QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
      form.addRow(&buttonBox);
      QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
      QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

      dialog.exec();
      render->setAutoUpdate(auto_update);
    }
}
