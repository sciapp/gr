#include "EditElementWidget.hxx"

EditElementWidget::EditElementWidget(GRPlotWidget *widget, QWidget *parent) : QWidget(parent)
{
  grplot_widget = widget;
#if !defined(NO_XERCES_C)
  schema_tree = grplot_widget->getSchemaTree();
#else
  schema_tree = nullptr;
#endif
}

void EditElementWidget::attributeEditEvent(bool highlight_location)
{
  QString text_label;
  auto current_selection = grplot_widget->getCurrentSelection();
  if (current_selection == nullptr || *current_selection == nullptr)
    {
      this->close();
      return;
    }

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
  schema_tree = grplot_widget->getSchemaTree();

  std::string currently_clicked_name = (*current_selection)->getRef()->localName();
  QString title("Selected: ");
  title.append(currently_clicked_name.c_str());
  this->setWindowTitle(title);
  auto change_parameters_label = new QLabel("Change Parameters:");
  change_parameters_label->setStyleSheet("font-weight: bold");
  auto form = new QFormLayout;
  form->addRow(change_parameters_label);

  QWidget *line_edit;

  std::vector<std::string> sorted_names;
  for (const auto &cur_attr_name : (*current_selection)->getRef()->getAttributeNames())
    {
      sorted_names.push_back(cur_attr_name);
    }
  std::sort(sorted_names.begin(), sorted_names.end());
  for (const auto &cur_attr_name : sorted_names)
    {
      if (util::startsWith(cur_attr_name, "_")) continue;
      QString tooltip_string =
          GRM::Render::getDefaultAndTooltip((*current_selection)->getRef(), cur_attr_name)[1].c_str();
      tooltip_string.append(".  Default: ");
      tooltip_string.append(
          GRM::Render::getDefaultAndTooltip((*current_selection)->getRef(), cur_attr_name)[0].c_str());

      if (combo_box_attr.contains(cur_attr_name.c_str()))
        {
          line_edit = new QComboBox(this);
          grplot_widget->advancedAttributeComboBoxHandler(cur_attr_name, (*current_selection)->getRef()->localName(),
                                                          &line_edit);
          if ((*current_selection)->getRef()->getAttribute(cur_attr_name).isInt())
            {
              attr_type.emplace(cur_attr_name, "xs:integer");
            }
          else if ((*current_selection)->getRef()->getAttribute(cur_attr_name).isDouble())
            {
              attr_type.emplace(cur_attr_name, "xs:double");
            }
          else
            {
              attr_type.emplace(cur_attr_name, "xs:string");
            }
          ((QCheckBox *)line_edit)->setToolTip(tooltip_string);
        }
      else if (check_box_attr.contains(cur_attr_name.c_str()))
        {
          line_edit = new QCheckBox(this);
          ((QCheckBox *)line_edit)->setToolTip(tooltip_string);
          ((QCheckBox *)line_edit)
              ->setChecked(static_cast<int>((*current_selection)->getRef()->getAttribute(cur_attr_name)) == 1);
        }
      else
        {
          if ((*current_selection)->getRef()->getAttribute(cur_attr_name).isInt())
            {
              attr_type.emplace(cur_attr_name, "xs:integer");
            }
          else if ((*current_selection)->getRef()->getAttribute(cur_attr_name).isDouble())
            {
              attr_type.emplace(cur_attr_name, "xs:double");
            }
          else
            {
              attr_type.emplace(cur_attr_name, "xs:string");
            }
          line_edit = new QLineEdit(this);
          ((QLineEdit *)line_edit)
              ->setText(static_cast<std::string>((*current_selection)->getRef()->getAttribute(cur_attr_name)).c_str());
          ((QLineEdit *)line_edit)->setToolTip(tooltip_string);
        }

      if (highlight_location && cur_attr_name == "location")
        {
          text_label = QString("<span style='color:#0000ff;'>%1</span>").arg(cur_attr_name.c_str());
          form->addRow(text_label, line_edit);
        }
      else
        {
          text_label = QString(cur_attr_name.c_str());
          form->addRow(text_label, line_edit);
        }

      labels << text_label;
      fields << line_edit;
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
              if (!(*current_selection)->getRef()->hasAttribute(attr_name))
                {
                  /* attributes of an element which aren't already in the tree getting added with red text color
                   */
                  auto type_name = static_cast<std::string>(child->getAttribute("type"));
                  attr_type.emplace(attr_name, type_name);
                  QString tooltip_string =
                      GRM::Render::getDefaultAndTooltip((*current_selection)->getRef(), attr_name)[1].c_str();
                  tooltip_string.append(".  Default: ");
                  tooltip_string.append(
                      GRM::Render::getDefaultAndTooltip((*current_selection)->getRef(), attr_name)[0].c_str());

                  if (combo_box_attr.contains(attr_name.c_str()))
                    {
                      line_edit = new QComboBox(this);
                      grplot_widget->advancedAttributeComboBoxHandler(
                          attr_name, (*current_selection)->getRef()->localName(), &line_edit);
                      ((QCheckBox *)line_edit)->setToolTip(tooltip_string);
                    }
                  else if (check_box_attr.contains(attr_name.c_str()))
                    {
                      line_edit = new QCheckBox(this);
                      ((QCheckBox *)line_edit)->setToolTip(tooltip_string);
                      ((QCheckBox *)line_edit)
                          ->setChecked(static_cast<int>((*current_selection)->getRef()->getAttribute(attr_name)) == 1);
                    }
                  else
                    {
                      line_edit = new QLineEdit(this);
                      ((QLineEdit *)line_edit)->setToolTip(tooltip_string);
                      ((QLineEdit *)line_edit)->setText("");
                    }
                  QString text_label;
                  if (highlight_location && attr_name == "location")
                    {
                      text_label = QString("<span style='color:#0000ff;'>%1</span>").arg(attr_name.c_str());
                      form->addRow(text_label, line_edit);
                    }
                  else
                    {
                      text_label = QString("<span style='color:#ff0000;'>%1</span>").arg(attr_name.c_str());
                      form->addRow(text_label, line_edit);
                    }

                  labels << text_label;
                  fields << line_edit;
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
                          if (!(*current_selection)->getRef()->hasAttribute(attr_name))
                            {
                              /* attributes of an element which aren't already in the tree getting added with
                               * red text color */
                              auto type_name = static_cast<std::string>(childchild->getAttribute("type"));
                              attr_type.emplace(attr_name, type_name);
                              QString tooltip_string =
                                  GRM::Render::getDefaultAndTooltip((*current_selection)->getRef(), attr_name)[1]
                                      .c_str();
                              tooltip_string.append(".  Default: ");
                              tooltip_string.append(
                                  GRM::Render::getDefaultAndTooltip((*current_selection)->getRef(), attr_name)[0]
                                      .c_str());

                              if (combo_box_attr.contains(attr_name.c_str()))
                                {
                                  line_edit = new QComboBox(this);
                                  grplot_widget->advancedAttributeComboBoxHandler(
                                      attr_name, (*current_selection)->getRef()->localName(), &line_edit);
                                  ((QCheckBox *)line_edit)->setToolTip(tooltip_string);
                                }
                              else if (check_box_attr.contains(attr_name.c_str()))
                                {
                                  line_edit = new QCheckBox(this);
                                  ((QCheckBox *)line_edit)->setToolTip(tooltip_string);
                                  ((QCheckBox *)line_edit)
                                      ->setChecked(static_cast<int>(
                                                       (*current_selection)->getRef()->getAttribute(attr_name)) == 1);
                                }
                              else
                                {
                                  line_edit = new QLineEdit(this);
                                  ((QLineEdit *)line_edit)->setToolTip(tooltip_string);
                                  ((QLineEdit *)line_edit)->setText("");
                                }
                              QString text_label =
                                  QString("<span style='color:#ff0000;'>%1</span>").arg(attr_name.c_str());
                              form->addRow(text_label, line_edit);

                              labels << text_label;
                              fields << line_edit;
                            }
                        }
                    }
                }
              else
                {
                  /* special case for colorrep cause there are way to many attributes inside the attributegroup
                   */
                  line_edit = new QLineEdit(this);
                  ((QLineEdit *)line_edit)->setText("");
                  QString text_label = QString("<span style='color:#ff0000;'>%1</span>").arg("Colorrep-index");
                  form->addRow(text_label, line_edit);

                  attr_type.emplace("Colorrep-index", "xs:string");
                  labels << text_label;
                  fields << line_edit;

                  line_edit = new QLineEdit(this);
                  ((QLineEdit *)line_edit)->setText("");
                  text_label = QString("<span style='color:#ff0000;'>%1</span>").arg("Colorrep-value");
                  form->addRow(text_label, line_edit);

                  attr_type.emplace("Colorrep-value", "xs:string");
                  labels << text_label;
                  fields << line_edit;
                }
            }
        }
    }

  QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
  QObject::connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));
  form->addRow(button_box);

  auto scroll_area_content = new QWidget;
  scroll_area_content->setLayout(form);

  auto scroll_area = new QScrollArea;
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll_area->setWidgetResizable(true);
  scroll_area->setWidget(scroll_area_content);

  auto group_box_layout = new QVBoxLayout;
  group_box_layout->addWidget(scroll_area);
  this->setLayout(group_box_layout);
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
  auto current_selection = grplot_widget->getCurrentSelection();
  for (int i = 0; i < labels.count(); i++)
    {
      auto &field = *fields[i]; // because typeid(*fields[i]) is bad :(
      if ((util::startsWith(labels[i].toStdString(), "<span style='color:#ff0000;'>") ||
           util::startsWith(labels[i].toStdString(), "<span style='color:#0000ff;'>")) &&
          util::endsWith(labels[i].toStdString(), "</span>"))
        {
          labels[i].remove(0, 29);
          labels[i].remove(labels[i].size() - 7, 7);
        }
      auto attr_name = labels[i].toStdString();
      if (typeid(field) == typeid(QLineEdit) && ((QLineEdit *)fields[i])->isModified())
        {
          auto name = static_cast<std::string>((*current_selection)->getRef()->getAttribute("name"));
          if (((QLineEdit *)fields[i])->text().toStdString().empty() && labels[i].toStdString() != "tick_label" &&
              labels[i].toStdString() != "text")
            {
              /* remove attributes from tree when the value got removed */
              (*current_selection)->getRef()->removeAttribute(labels[i].toStdString());
            }
          else
            {
              if (labels[i].toStdString() == "text")
                {
                  const auto value = ((QLineEdit *)fields[i])->text().toStdString();
                  if (attr_type[attr_name] == "xs:string" ||
                      (attr_type[attr_name] == "strint" && !util::isDigits(value)))
                    {
                      if ((*current_selection)->getRef()->parentElement()->localName() == "text_region")
                        {
                          (*current_selection)
                              ->getRef()
                              ->parentElement()
                              ->parentElement()
                              ->setAttribute("text_content", value);
                        }
                      else if (name == "xlabel" || name == "ylabel")
                        {
                          (*current_selection)
                              ->getRef()
                              ->parentElement()
                              ->parentElement()
                              ->querySelectors(name)
                              ->setAttribute(name, value);
                        }
                    }
                  else if (attr_type[attr_name] == "xs:double")
                    {
                      (*current_selection)
                          ->getRef()
                          ->parentElement()
                          ->setAttribute(labels[i].toStdString(), std::stod(value));
                    }
                  else if (attr_type[attr_name] == "xs:integer" ||
                           (attr_type[attr_name] == "strint" && util::isDigits(value)))
                    {
                      (*current_selection)
                          ->getRef()
                          ->parentElement()
                          ->setAttribute(labels[i].toStdString(), std::stoi(value));
                    }
                }
              if (labels[i].toStdString() == "Colorrep-index")
                {
                  /* special case for colorrep attribute */
                  (*current_selection)
                      ->getRef()
                      ->setAttribute("colorrep." + ((QLineEdit *)fields[i])->text().toStdString(),
                                     ((QLineEdit *)fields[i + 1])->text().toStdString());
                }
              else if (labels[i].toStdString() != "Colorrep-value")
                {
                  const auto value = ((QLineEdit *)fields[i])->text().toStdString();
                  if (attr_type[attr_name] == "xs:string" ||
                      (attr_type[attr_name] == "strint" && !util::isDigits(value)))
                    {
                      (*current_selection)->getRef()->setAttribute(labels[i].toStdString(), value);
                    }
                  else if (attr_type[attr_name] == "xs:double")
                    {
                      (*current_selection)->getRef()->setAttribute(labels[i].toStdString(), std::stod(value));
                    }
                  else if (attr_type[attr_name] == "xs:integer" ||
                           (attr_type[attr_name] == "strint" && util::isDigits(value)))
                    {
                      (*current_selection)->getRef()->setAttribute(labels[i].toStdString(), std::stoi(value));
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
              (*current_selection)->getRef()->removeAttribute(labels[i].toStdString());
            }
          else
            {
              if (attr_name == "location" && (*current_selection)->getRef()->localName() == "axis")
                (*current_selection)->getRef()->setAttribute("_ignore_next_tick_orientation", true);
              const auto value = ((QComboBox *)fields[i])->itemText(index).toStdString();
              grplot_widget->attributeSetForComboBox(attr_type[attr_name], (*current_selection)->getRef(), value,
                                                     (labels[i]).toStdString());
            }
        }
      else if (typeid(field) == typeid(QCheckBox))
        {
          (*current_selection)->getRef()->setAttribute(labels[i].toStdString(), ((QCheckBox *)fields[i])->isChecked());
        }
    }
  grplot_widget->setTreeUpdate(true);
  if (getenv("GRM_DEBUG"))
    {
      std::cerr << toXML(grm_get_document_root(),
                         GRM::SerializerOptions{std::string(2, ' '),
                                                GRM::SerializerOptions::InternalAttributesFormat::PLAIN})
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
