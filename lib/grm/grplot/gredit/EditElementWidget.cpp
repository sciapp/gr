#include "EditElementWidget.h"

EditElementWidget::EditElementWidget(GRPlotWidget *widget, QWidget *parent) : QWidget(parent)
{
  grplot_widget = widget;
#if !defined(NO_XERCES_C)
  schema_tree = grplot_widget->get_schema_tree();
#else
  schema_tree = nullptr;
#endif
}

void EditElementWidget::AttributeEditEvent()
{
  auto current_selection = grplot_widget->get_current_selection();
  if (current_selection == nullptr) return;

  if (this->layout() != nullptr)
    {
      QLayoutItem *item;
      while ((item = this->layout()->takeAt(0)) != nullptr)
        {
          delete item->widget();
          delete item;
        }
      delete this->layout();
      fields.clear();
      labels.clear();
      attr_type.clear();
    }

  auto combo_box_attr = grplot_widget->getComboBoxAttributes();
  auto check_box_attr = grplot_widget->getCheckBoxAttributes();
  schema_tree = grplot_widget->get_schema_tree();

  std::string currently_clicked_name = (*current_selection)->get_ref()->localName();
  QString title("Selected: ");
  title.append(currently_clicked_name.c_str());
  this->setWindowTitle(title);
  auto changeParametersLabel = new QLabel("Change Parameters:");
  changeParametersLabel->setStyleSheet("font-weight: bold");
  auto form = new QFormLayout;
  form->addRow(changeParametersLabel);

  QWidget *lineEdit;

  std::vector<std::string> sorted_names;
  for (const auto &cur_attr_name : (*current_selection)->get_ref()->getAttributeNames())
    {
      sorted_names.push_back(cur_attr_name);
    }
  std::sort(sorted_names.begin(), sorted_names.end());
  for (const auto &cur_attr_name : sorted_names)
    {
      if (util::startsWith(cur_attr_name, "_")) continue;
      QString tooltipString =
          GRM::Render::getDefaultAndTooltip((*current_selection)->get_ref(), cur_attr_name)[1].c_str();
      tooltipString.append(".  Default: ");
      tooltipString.append(
          GRM::Render::getDefaultAndTooltip((*current_selection)->get_ref(), cur_attr_name)[0].c_str());

      if (combo_box_attr.contains(cur_attr_name.c_str()))
        {
          lineEdit = new QComboBox(this);
          grplot_widget->advancedAttributeComboBoxHandler(cur_attr_name, (*current_selection)->get_ref()->localName(),
                                                          &lineEdit);
          if ((*current_selection)->get_ref()->getAttribute(cur_attr_name).isInt())
            {
              attr_type.emplace(cur_attr_name, "xs:integer");
            }
          else if ((*current_selection)->get_ref()->getAttribute(cur_attr_name).isDouble())
            {
              attr_type.emplace(cur_attr_name, "xs:double");
            }
          else
            {
              attr_type.emplace(cur_attr_name, "xs:string");
            }
          ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
        }
      else if (check_box_attr.contains(cur_attr_name.c_str()))
        {
          lineEdit = new QCheckBox(this);
          ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
          ((QCheckBox *)lineEdit)
              ->setChecked(static_cast<int>((*current_selection)->get_ref()->getAttribute(cur_attr_name)) == 1);
        }
      else
        {
          if ((*current_selection)->get_ref()->getAttribute(cur_attr_name).isInt())
            {
              attr_type.emplace(cur_attr_name, "xs:integer");
            }
          else if ((*current_selection)->get_ref()->getAttribute(cur_attr_name).isDouble())
            {
              attr_type.emplace(cur_attr_name, "xs:double");
            }
          else
            {
              attr_type.emplace(cur_attr_name, "xs:string");
            }
          lineEdit = new QLineEdit(this);
          ((QLineEdit *)lineEdit)
              ->setText(static_cast<std::string>((*current_selection)->get_ref()->getAttribute(cur_attr_name)).c_str());
          ((QLineEdit *)lineEdit)->setToolTip(tooltipString);
        }
      QString text_label = QString(cur_attr_name.c_str());
      form->addRow(text_label, lineEdit);

      labels << text_label;
      fields << lineEdit;
    }

  if (schema_tree != nullptr)
    {
      std::shared_ptr<GRM::Element> element;
      auto selections = schema_tree->querySelectorsAll("[name=" + currently_clicked_name + "]");
      for (const auto &selection : selections)
        {
          if (selection->localName() == "xs:element") element = selection->children()[0];
        }

      /* iterate through complextype elements */
      for (const auto &child : element->children())
        {
          if (child->localName() == "xs:attribute")
            {
              auto attr_name = static_cast<std::string>(child->getAttribute("name"));
              if (!(*current_selection)->get_ref()->hasAttribute(attr_name))
                {
                  /* attributes of an element which aren't already in the tree getting added with red text color
                   */
                  auto type_name = static_cast<std::string>(child->getAttribute("type"));
                  attr_type.emplace(attr_name, type_name);
                  QString tooltipString =
                      GRM::Render::getDefaultAndTooltip((*current_selection)->get_ref(), attr_name)[1].c_str();
                  tooltipString.append(".  Default: ");
                  tooltipString.append(
                      GRM::Render::getDefaultAndTooltip((*current_selection)->get_ref(), attr_name)[0].c_str());

                  if (combo_box_attr.contains(attr_name.c_str()))
                    {
                      lineEdit = new QComboBox(this);
                      grplot_widget->advancedAttributeComboBoxHandler(
                          attr_name, (*current_selection)->get_ref()->localName(), &lineEdit);
                      ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                    }
                  else if (check_box_attr.contains(attr_name.c_str()))
                    {
                      lineEdit = new QCheckBox(this);
                      ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                      ((QCheckBox *)lineEdit)
                          ->setChecked(static_cast<int>((*current_selection)->get_ref()->getAttribute(attr_name)) == 1);
                    }
                  else
                    {
                      lineEdit = new QLineEdit(this);
                      ((QLineEdit *)lineEdit)->setToolTip(tooltipString);
                      ((QLineEdit *)lineEdit)->setText("");
                    }
                  QString text_label = QString("<span style='color:#ff0000;'>%1</span>").arg(attr_name.c_str());
                  form->addRow(text_label, lineEdit);

                  labels << text_label;
                  fields << lineEdit;
                }
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
                          if (!(*current_selection)->get_ref()->hasAttribute(attr_name))
                            {
                              /* attributes of an element which aren't already in the tree getting added with
                               * red text color */
                              auto type_name = static_cast<std::string>(childchild->getAttribute("type"));
                              attr_type.emplace(attr_name, type_name);
                              QString tooltipString =
                                  GRM::Render::getDefaultAndTooltip((*current_selection)->get_ref(), attr_name)[1]
                                      .c_str();
                              tooltipString.append(".  Default: ");
                              tooltipString.append(
                                  GRM::Render::getDefaultAndTooltip((*current_selection)->get_ref(), attr_name)[0]
                                      .c_str());

                              if (combo_box_attr.contains(attr_name.c_str()))
                                {
                                  lineEdit = new QComboBox(this);
                                  grplot_widget->advancedAttributeComboBoxHandler(
                                      attr_name, (*current_selection)->get_ref()->localName(), &lineEdit);
                                  ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                                }
                              else if (check_box_attr.contains(attr_name.c_str()))
                                {
                                  lineEdit = new QCheckBox(this);
                                  ((QCheckBox *)lineEdit)->setToolTip(tooltipString);
                                  ((QCheckBox *)lineEdit)
                                      ->setChecked(static_cast<int>(
                                                       (*current_selection)->get_ref()->getAttribute(attr_name)) == 1);
                                }
                              else
                                {
                                  lineEdit = new QLineEdit(this);
                                  ((QLineEdit *)lineEdit)->setToolTip(tooltipString);
                                  ((QLineEdit *)lineEdit)->setText("");
                                }
                              QString text_label =
                                  QString("<span style='color:#ff0000;'>%1</span>").arg(attr_name.c_str());
                              form->addRow(text_label, lineEdit);

                              labels << text_label;
                              fields << lineEdit;
                            }
                        }
                    }
                }
              else
                {
                  /* special case for colorrep cause there are way to many attributes inside the attributegroup
                   */
                  lineEdit = new QLineEdit(this);
                  ((QLineEdit *)lineEdit)->setText("");
                  QString text_label = QString("<span style='color:#ff0000;'>%1</span>").arg("Colorrep-index");
                  form->addRow(text_label, lineEdit);

                  attr_type.emplace("Colorrep-index", "xs:string");
                  labels << text_label;
                  fields << lineEdit;

                  lineEdit = new QLineEdit(this);
                  ((QLineEdit *)lineEdit)->setText("");
                  text_label = QString("<span style='color:#ff0000;'>%1</span>").arg("Colorrep-value");
                  form->addRow(text_label, lineEdit);

                  attr_type.emplace("Colorrep-value", "xs:string");
                  labels << text_label;
                  fields << lineEdit;
                }
            }
        }
    }

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  form->addRow(buttonBox);

  auto scrollAreaContent = new QWidget;
  scrollAreaContent->setLayout(form);

  auto scrollArea = new QScrollArea;
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrollArea->setWidgetResizable(true);
  scrollArea->setWidget(scrollAreaContent);

  auto groupBoxLayout = new QVBoxLayout;
  groupBoxLayout->addWidget(scrollArea);
  this->setLayout(groupBoxLayout);
  this->resize(std::min<int>(grplot_widget->width(), form->sizeHint().width() + this->layout()->sizeHint().width()),
               std::min<int>(grplot_widget->height(), form->sizeHint().height() + this->layout()->sizeHint().height()));
}

void EditElementWidget::reject()
{
  grplot_widget->setTreeUpdate(false);
  fields.clear();
  labels.clear();
  attr_type.clear();
  this->close();
}

void EditElementWidget::accept()
{
  auto current_selection = grplot_widget->get_current_selection();
  for (int i = 0; i < labels.count(); i++)
    {
      auto &field = *fields[i]; // because typeid(*fields[i]) is bad :(
      if (util::startsWith(labels[i].toStdString(), "<span style='color:#ff0000;'>") &&
          util::endsWith(labels[i].toStdString(), "</span>"))
        {
          labels[i].remove(0, 29);
          labels[i].remove(labels[i].size() - 7, 7);
        }
      std::string attr_name = labels[i].toStdString();
      if (typeid(field) == typeid(QLineEdit) && ((QLineEdit *)fields[i])->isModified())
        {
          std::string name = std::string((*current_selection)->get_ref()->getAttribute("name"));
          if (((QLineEdit *)fields[i])->text().toStdString().empty() && labels[i].toStdString() != "tick_label" &&
              labels[i].toStdString() != "text")
            {
              /* remove attributes from tree when the value got removed */
              (*current_selection)->get_ref()->removeAttribute(labels[i].toStdString());
            }
          else
            {
              if (labels[i].toStdString() == "text")
                {
                  const std::string value = ((QLineEdit *)fields[i])->text().toStdString();
                  if (attr_type[attr_name] == "xs:string" ||
                      (attr_type[attr_name] == "strint" && !util::is_digits(value)))
                    {
                      if ((*current_selection)->get_ref()->parentElement()->localName() == "text_region")
                        {
                          (*current_selection)
                              ->get_ref()
                              ->parentElement()
                              ->parentElement()
                              ->setAttribute("text_content", value);
                        }
                      else if (name == "xlabel" || name == "ylabel")
                        {
                          (*current_selection)
                              ->get_ref()
                              ->parentElement()
                              ->parentElement()
                              ->querySelectors(name)
                              ->setAttribute(name, value);
                        }
                    }
                  else if (attr_type[attr_name] == "xs:double")
                    {
                      (*current_selection)
                          ->get_ref()
                          ->parentElement()
                          ->setAttribute(labels[i].toStdString(), std::stod(value));
                    }
                  else if (attr_type[attr_name] == "xs:integer" ||
                           (attr_type[attr_name] == "strint" && util::is_digits(value)))
                    {
                      (*current_selection)
                          ->get_ref()
                          ->parentElement()
                          ->setAttribute(labels[i].toStdString(), std::stoi(value));
                    }
                }
              if (labels[i].toStdString() == "Colorrep-index")
                {
                  /* special case for colorrep attribute */
                  (*current_selection)
                      ->get_ref()
                      ->setAttribute("colorrep." + ((QLineEdit *)fields[i])->text().toStdString(),
                                     ((QLineEdit *)fields[i + 1])->text().toStdString());
                }
              else if (labels[i].toStdString() != "Colorrep-value")
                {
                  const std::string value = ((QLineEdit *)fields[i])->text().toStdString();
                  if (attr_type[attr_name] == "xs:string" ||
                      (attr_type[attr_name] == "strint" && !util::is_digits(value)))
                    {
                      (*current_selection)->get_ref()->setAttribute(labels[i].toStdString(), value);
                    }
                  else if (attr_type[attr_name] == "xs:double")
                    {
                      (*current_selection)->get_ref()->setAttribute(labels[i].toStdString(), std::stod(value));
                    }
                  else if (attr_type[attr_name] == "xs:integer" ||
                           (attr_type[attr_name] == "strint" && util::is_digits(value)))
                    {
                      (*current_selection)->get_ref()->setAttribute(labels[i].toStdString(), std::stoi(value));
                    }
                }
            }
        }
      else if (typeid(field) == typeid(QComboBox))
        {
          int index = ((QComboBox *)fields[i])->currentIndex();
          if (((QComboBox *)fields[i])->itemText(index).toStdString().empty())
            {
              /* remove attributes from tree when the value got removed */
              (*current_selection)->get_ref()->removeAttribute(labels[i].toStdString());
            }
          else
            {
              if (attr_name == "location" && (*current_selection)->get_ref()->localName() == "axis")
                (*current_selection)->get_ref()->setAttribute("_ignore_next_tick_orientation", true);
              const std::string value = ((QComboBox *)fields[i])->itemText(index).toStdString();
              grplot_widget->attributeSetForComboBox(attr_type[attr_name], (*current_selection)->get_ref(), value,
                                                     (labels[i]).toStdString());
            }
        }
      else if (typeid(field) == typeid(QCheckBox))
        {
          (*current_selection)->get_ref()->setAttribute(labels[i].toStdString(), ((QCheckBox *)fields[i])->isChecked());
        }
    }
  grplot_widget->setTreeUpdate(true);
  if (getenv("GRM_DEBUG"))
    {
      std::cerr << toXML(grm_get_document_root(),
                         GRM::SerializerOptions{std::string(2, ' '),
                                                GRM::SerializerOptions::InternalAttributesFormat::Plain})
                << "\n";
    }
  grplot_widget->editElementAccepted();
  fields.clear();
  labels.clear();
  attr_type.clear();
  this->close();
}

void EditElementWidget::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) accept();
}
