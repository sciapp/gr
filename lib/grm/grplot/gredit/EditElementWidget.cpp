#include "EditElementWidget.hxx"
#include "../CollapsibleSection.hxx"
#include <gks.h>

static std::string context_name;

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
  bool line_modification_added = false, marker_modification_added = false, text_modification_added = false,
       fill_modification_added = false, viewport_added = false, viewport_normalized_added = false, window_added = false,
       range_modification_added = false, log_modification_added = false, flip_modification_added = false,
       lim_modification_added = false, element_movement_modification_added = false, space_modification_added = false,
       ws_modification_added = false;
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

  context_name = "";

  auto combo_box_attr = grplot_widget->getComboBoxAttributes();
  auto check_box_attr = grplot_widget->getCheckBoxAttributes();
  auto color_ind_attr = grplot_widget->getColorIndAttributes();
  auto color_rgb_attr = grplot_widget->getColorRGBAttributes();
  schema_tree = grplot_widget->getSchemaTree();
  auto advanced_editor = grplot_widget->getEnableAdvancedEditor();
  auto context_attributes = GRM::getContextAttributes();

  std::string currently_clicked_name = (*current_selection)->getRef()->localName();
  QString title("Selected: ");
  title.append(currently_clicked_name.c_str());
  this->setWindowTitle(title);
  auto change_parameters_label = new QLabel("Change Parameters:");
  change_parameters_label->setStyleSheet("font-weight: bold");
  auto form = new QFormLayout;
  form->addRow(change_parameters_label);

  auto line_modification_form = new QFormLayout;
  CollapsibleSection *line_modification = new CollapsibleSection("Line Modifications", 100, this);
  auto *line_modification_layout = new QVBoxLayout();
  auto marker_modification_form = new QFormLayout;
  CollapsibleSection *marker_modification = new CollapsibleSection("Marker Modifications", 100, this);
  auto *marker_modification_layout = new QVBoxLayout();
  auto text_modification_form = new QFormLayout;
  CollapsibleSection *text_modification = new CollapsibleSection("Text Modifications", 100, this);
  auto *text_modification_layout = new QVBoxLayout();
  auto fill_modification_form = new QFormLayout;
  CollapsibleSection *fill_modification = new CollapsibleSection("Fill Modifications", 100, this);
  auto *fill_modification_layout = new QVBoxLayout();
  auto viewport_form = new QFormLayout;
  CollapsibleSection *viewport = new CollapsibleSection("Viewport", 100, this);
  auto *viewport_layout = new QVBoxLayout();
  auto viewport_normalized_form = new QFormLayout;
  CollapsibleSection *viewport_normalized = new CollapsibleSection("Viewport Normalized", 100, this);
  auto *viewport_normalized_layout = new QVBoxLayout();
  auto window_form = new QFormLayout;
  CollapsibleSection *window = new CollapsibleSection("Window", 100, this);
  auto *window_layout = new QVBoxLayout();
  auto range_modification_form = new QFormLayout;
  CollapsibleSection *range_modification = new CollapsibleSection("Range Modifications", 100, this);
  auto *range_modification_layout = new QVBoxLayout();
  auto log_modification_form = new QFormLayout;
  CollapsibleSection *log_modification = new CollapsibleSection("Log Modifications", 100, this);
  auto *log_modification_layout = new QVBoxLayout();
  auto flip_modification_form = new QFormLayout;
  CollapsibleSection *flip_modification = new CollapsibleSection("Flip Modifications", 100, this);
  auto *flip_modification_layout = new QVBoxLayout();
  auto lim_modification_form = new QFormLayout;
  CollapsibleSection *lim_modification = new CollapsibleSection("Lim Modifications", 100, this);
  auto *lim_modification_layout = new QVBoxLayout();
  auto element_movement_modification_form = new QFormLayout;
  CollapsibleSection *element_movement_modification =
      new CollapsibleSection("Element Movement Modifications", 100, this);
  auto *element_movement_modification_layout = new QVBoxLayout();
  auto space_modification_form = new QFormLayout;
  CollapsibleSection *space_modification = new CollapsibleSection("Space Modifications", 100, this);
  auto *space_modification_layout = new QVBoxLayout();
  auto ws_modification_form = new QFormLayout;
  CollapsibleSection *ws_modification = new CollapsibleSection("WS Modifications", 100, this);
  auto *ws_modification_layout = new QVBoxLayout();

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
      if ((*current_selection)->getRef()->localName() == "layout_grid" &&
          (*current_selection)->getRef()->parentElement()->localName() != "layout_grid")
        {
          // special case for flat layout_grid cause other attributes than these below doesnt affect anything
          if (cur_attr_name == "viewport_x_min" || cur_attr_name == "viewport_x_max" ||
              cur_attr_name == "viewport_y_min" || cur_attr_name == "viewport_y_max" ||
              cur_attr_name == "viewport_normalized_x_min" || cur_attr_name == "viewport_normalized_x_max" ||
              cur_attr_name == "viewport_normalized_y_min" || cur_attr_name == "viewport_normalized_y_max" ||
              cur_attr_name == "fit_parents_height" || cur_attr_name == "fit_parents_width" ||
              cur_attr_name == "x_scale_ndc" || cur_attr_name == "y_scale_ndc" || cur_attr_name == "x_shift_ndc" ||
              cur_attr_name == "y_shift_ndc")
            continue;
        }
      else if ((*current_selection)->getRef()->localName() == "coordinate_system")
        {
          // special case for coordinate_system cause the ndc movements results in a nonsense plot
          if (cur_attr_name == "x_scale_ndc" || cur_attr_name == "y_scale_ndc" || cur_attr_name == "x_shift_ndc" ||
              cur_attr_name == "y_shift_ndc")
            continue;
        }
      QString tooltip_string =
          GRM::Render::getDefaultAndTooltip((*current_selection)->getRef(), cur_attr_name)[1].c_str();
      tooltip_string.append(".  Default: ");
      tooltip_string.append(
          GRM::Render::getDefaultAndTooltip((*current_selection)->getRef(), cur_attr_name)[0].c_str());

      if (combo_box_attr.contains(cur_attr_name.c_str()) &&
          ((cur_attr_name != "x" && cur_attr_name != "y") ||
           static_cast<int>((*current_selection)->getRef()->getAttribute(cur_attr_name).type()) == 3))
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
      else if (color_ind_attr.contains(cur_attr_name.c_str()))
        {
          auto index = static_cast<int>((*current_selection)->getRef()->getAttribute(cur_attr_name));
          line_edit = new QPushButton(std::to_string(index).c_str(), this);
          ((QPushButton *)line_edit)->setToolTip(tooltip_string);
          ((QPushButton *)line_edit)->setObjectName(cur_attr_name.c_str());

          QImage image(1, 1, QImage::Format_RGB32);
          QRgb value;
          int errind;
          double r, g, b;

          gks_inq_color_rep(-1, index, -1, &errind, &r, &g, &b);
          value = qRgb(255 * r, 255 * g, 255 * b);
          image.setPixel(0, 0, value);

          auto color_pic = QPixmap::fromImage(image);
          color_pic = color_pic.scaled(20, 20);
          ((QPushButton *)line_edit)->setIcon(QIcon(color_pic));

          QObject::connect(((QPushButton *)line_edit), SIGNAL(clicked()), this, SLOT(colorIndexSlot()));
          QObject::connect(((QPushButton *)line_edit), &QPushButton::clicked, [=]() {
            QImage new_image(1, 1, QImage::Format_RGB32);
            QRgb new_value;
            int err;
            double new_r, new_g, new_b;

            auto new_index = static_cast<int>((*current_selection)->getRef()->getAttribute(cur_attr_name));
            ((QPushButton *)line_edit)->setText(std::to_string(new_index).c_str());

            gks_inq_color_rep(-1, new_index, -1, &err, &new_r, &new_g, &new_b);
            new_value = qRgb(255 * new_r, 255 * new_g, 255 * new_b);
            new_image.setPixel(0, 0, new_value);

            auto new_color_pic = QPixmap::fromImage(new_image);
            new_color_pic = new_color_pic.scaled(20, 20);
            ((QPushButton *)line_edit)->setIcon(QIcon(new_color_pic));
          });
        }
      else if (color_rgb_attr.contains(cur_attr_name.c_str()))
        {
          line_edit = new QPushButton(this);
          ((QPushButton *)line_edit)->setToolTip(tooltip_string);
          ((QPushButton *)line_edit)->setObjectName(cur_attr_name.c_str());

          double ref_r = 0, ref_g = 0, ref_b = 0;
          std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();
          if ((*current_selection)->getRef()->hasAttribute(cur_attr_name))
            {
              auto context_ref = static_cast<std::string>((*current_selection)->getRef()->getAttribute(cur_attr_name));
              auto rgb_vec = GRM::get<std::vector<double>>((*context)[context_ref]);
              ref_r = rgb_vec.at(0);
              ref_g = rgb_vec.at(1);
              ref_b = rgb_vec.at(2);
            }

          QImage image(1, 1, QImage::Format_RGB32);
          QRgb value;

          value = qRgb(255 * ref_r, 255 * ref_g, 255 * ref_b);
          image.setPixel(0, 0, value);

          auto color_pic = QPixmap::fromImage(image);
          color_pic = color_pic.scaled(20, 20);
          ((QPushButton *)line_edit)->setIcon(QIcon(color_pic));

          QObject::connect(((QPushButton *)line_edit), SIGNAL(clicked()), this, SLOT(colorRGBSlot()));
          QObject::connect(((QPushButton *)line_edit), &QPushButton::clicked, [=]() {
            QImage new_image(1, 1, QImage::Format_RGB32);
            QRgb new_value;
            double ref_r_new = 0, ref_g_new = 0, ref_b_new = 0;
            if ((*current_selection)->getRef()->hasAttribute(cur_attr_name))
              {
                auto context_ref =
                    static_cast<std::string>((*current_selection)->getRef()->getAttribute(cur_attr_name));
                auto rgb_vec = GRM::get<std::vector<double>>((*context)[context_ref]);
                ref_r_new = rgb_vec.at(0);
                ref_g_new = rgb_vec.at(1);
                ref_b_new = rgb_vec.at(2);
                ((QPushButton *)line_edit)->setText(context_ref.c_str());
              }

            new_value = qRgb(255 * ref_r_new, 255 * ref_g_new, 255 * ref_b_new);
            new_image.setPixel(0, 0, new_value);

            auto new_color_pic = QPixmap::fromImage(new_image);
            new_color_pic = new_color_pic.scaled(20, 20);
            ((QPushButton *)line_edit)->setIcon(QIcon(new_color_pic));
          });
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

      if (cur_attr_name == "line_color_ind" || cur_attr_name == "line_color_rgb" || cur_attr_name == "line_spec" ||
          cur_attr_name == "line_type" || cur_attr_name == "line_width")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!line_modification_added)
            {
              form->addRow(line_modification);
              line_modification_added = true;
            }

          line_modification_form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "marker_color_ind" || cur_attr_name == "marker_color_indices" ||
               cur_attr_name == "marker_size" || cur_attr_name == "marker_sizes" || cur_attr_name == "marker_type" ||
               cur_attr_name == "border_color_ind" || cur_attr_name == "border_width")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!marker_modification_added)
            {
              form->addRow(marker_modification);
              marker_modification_added = true;
            }

          marker_modification_form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "text_align_horizontal" || cur_attr_name == "text_align_vertical" ||
               cur_attr_name == "text_color_ind" || cur_attr_name == "font" || cur_attr_name == "font_precision")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!text_modification_added)
            {
              form->addRow(text_modification);
              text_modification_added = true;
            }

          text_modification_form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "fill_color_ind" || cur_attr_name == "fill_color_rgb" ||
               cur_attr_name == "fill_int_style" || cur_attr_name == "fill_style")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!fill_modification_added)
            {
              form->addRow(fill_modification);
              fill_modification_added = true;
            }

          fill_modification_form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "viewport_x_min" || cur_attr_name == "viewport_x_max" ||
               cur_attr_name == "viewport_y_min" || cur_attr_name == "viewport_y_max")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!viewport_added && advanced_editor)
            {
              form->addRow(viewport);
              viewport_added = true;
            }

          viewport_form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "viewport_normalized_x_min" || cur_attr_name == "viewport_normalized_x_max" ||
               cur_attr_name == "viewport_normalized_y_min" || cur_attr_name == "viewport_normalized_y_max")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!viewport_normalized_added && advanced_editor)
            {
              form->addRow(viewport_normalized);
              viewport_normalized_added = true;
            }

          viewport_normalized_form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "window_x_min" || cur_attr_name == "window_x_max" || cur_attr_name == "window_y_min" ||
               cur_attr_name == "window_y_max" || cur_attr_name == "window_z_min" || cur_attr_name == "window_z_max")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!window_added)
            {
              form->addRow(window);
              window_added = true;
            }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
          window_form->addRow(label, line_edit);
          if (!advanced_editor)
            window_form->setRowVisible(window_form->rowCount() - 1,
                                       !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
          if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
            window_form->addRow(label, line_edit);
#endif
        }
      else if (cur_attr_name == "x_range_min" || cur_attr_name == "x_range_max" || cur_attr_name == "y_range_min" ||
               cur_attr_name == "y_range_max" || cur_attr_name == "z_range_min" || cur_attr_name == "z_range_max" ||
               cur_attr_name == "c_range_min" || cur_attr_name == "c_range_max" || cur_attr_name == "r_range_min" ||
               cur_attr_name == "r_range_max" || cur_attr_name == "theta_range_min" ||
               cur_attr_name == "theta_range_max")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!range_modification_added)
            {
              form->addRow(range_modification);
              range_modification_added = true;
            }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
          range_modification_form->addRow(label, line_edit);
          if (!advanced_editor)
            range_modification_form->setRowVisible(range_modification_form->rowCount() - 1,
                                                   !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
          if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
            range_modification_form->addRow(label, line_edit);
#endif
        }
      else if (cur_attr_name == "x_log" || cur_attr_name == "y_log" || cur_attr_name == "z_log" ||
               cur_attr_name == "r_log" || cur_attr_name == "theta_log")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!log_modification_added)
            {
              form->addRow(log_modification);
              log_modification_added = true;
            }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
          log_modification_form->addRow(label, line_edit);
          if (!advanced_editor)
            log_modification_form->setRowVisible(log_modification_form->rowCount() - 1,
                                                 !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
          if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
            log_modification_form->addRow(label, line_edit);
#endif
        }
      else if (cur_attr_name == "x_flip" || cur_attr_name == "y_flip" || cur_attr_name == "z_flip" ||
               cur_attr_name == "theta_flip")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!flip_modification_added)
            {
              form->addRow(flip_modification);
              flip_modification_added = true;
            }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
          flip_modification_form->addRow(label, line_edit);
          if (!advanced_editor)
            flip_modification_form->setRowVisible(flip_modification_form->rowCount() - 1,
                                                  !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
          if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
            flip_modification_form->addRow(label, line_edit);
#endif
        }
      else if (cur_attr_name == "x_lim_min" || cur_attr_name == "x_lim_max" || cur_attr_name == "y_lim_min" ||
               cur_attr_name == "y_lim_max" || cur_attr_name == "z_lim_min" || cur_attr_name == "z_lim_max" ||
               cur_attr_name == "c_lim_min" || cur_attr_name == "c_lim_max" || cur_attr_name == "adjust_x_lim" ||
               cur_attr_name == "adjust_y_lim" || cur_attr_name == "adjust_z_lim" || cur_attr_name == "r_lim_min" ||
               cur_attr_name == "r_lim_max" || cur_attr_name == "theta_lim_min" || cur_attr_name == "theta_lim_max")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!lim_modification_added)
            {
              form->addRow(lim_modification);
              lim_modification_added = true;
            }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
          lim_modification_form->addRow(label, line_edit);
          if (!advanced_editor)
            lim_modification_form->setRowVisible(lim_modification_form->rowCount() - 1,
                                                 !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
          if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
            lim_modification_form->addRow(label, line_edit);
#endif
        }
      else if (cur_attr_name == "x_scale_ndc" || cur_attr_name == "x_shift_ndc" || cur_attr_name == "x_scale_wc" ||
               cur_attr_name == "x_shift_wc" || cur_attr_name == "y_scale_ndc" || cur_attr_name == "y_shift_ndc" ||
               cur_attr_name == "y_scale_wc" || cur_attr_name == "y_shift_wc")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!element_movement_modification_added)
            {
              if (advanced_editor || (cur_attr_name != "x_scale_wc" && cur_attr_name != "x_shift_wc" &&
                                      cur_attr_name != "y_scale_wc" && cur_attr_name != "y_shift_wc"))
                {
                  form->addRow(element_movement_modification);
                  element_movement_modification_added = true;
                }
            }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
          element_movement_modification_form->addRow(label, line_edit);
          if (!advanced_editor)
            element_movement_modification_form->setRowVisible(
                element_movement_modification_form->rowCount() - 1,
                !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
          if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
            element_movement_modification_form->addRow(label, line_edit);
#endif
        }
      else if (cur_attr_name == "space_rotation" || cur_attr_name == "space_tilt" || cur_attr_name == "space_z_min" ||
               cur_attr_name == "space_z_max" || cur_attr_name == "space_3d_camera_distance" ||
               cur_attr_name == "space_3d_fov" || cur_attr_name == "space_3d_phi" || cur_attr_name == "space_3d_theta")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!space_modification_added)
            {
              form->addRow(space_modification);
              space_modification_added = true;
            }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
          space_modification_form->addRow(label, line_edit);
          if (!advanced_editor)
            space_modification_form->setRowVisible(space_modification_form->rowCount() - 1,
                                                   !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
          if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
            space_modification_form->addRow(label, line_edit);
#endif
        }
      else if (cur_attr_name == "ws_window_x_min" || cur_attr_name == "ws_window_x_max" ||
               cur_attr_name == "ws_window_y_min" || cur_attr_name == "ws_window_y_max" ||
               cur_attr_name == "ws_viewport_x_min" || cur_attr_name == "ws_viewport_x_max" ||
               cur_attr_name == "ws_viewport_y_min" || cur_attr_name == "ws_viewport_y_max")
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          if (!ws_modification_added && advanced_editor)
            {
              form->addRow(ws_modification);
              ws_modification_added = true;
            }

          ws_modification_form->addRow(label, line_edit);
        }
      else if (highlight_location && cur_attr_name == "location")
        {
          text_label = QString("<span style='color:#0000ff;'>%1</span>").arg(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
          form->addRow(label, line_edit);
        }
      else
        {
          text_label = QString(cur_attr_name.c_str());
          auto label = new QLabel(text_label);
          label->setToolTip(tooltip_string);
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
          if (std::find(context_attributes.begin(), context_attributes.end(), cur_attr_name) !=
                  context_attributes.end() &&
              static_cast<int>((*current_selection)->getRef()->getAttribute(cur_attr_name).type()) == 3) // string
            {
              auto widget = new QWidget();
              auto grid_layout = new QGridLayout();
              auto button = new QPushButton();
              connect(button, &QPushButton::clicked, [=]() {
                if ((*current_selection)->getRef()->hasAttribute(cur_attr_name))
                  context_name = static_cast<std::string>((*current_selection)->getRef()->getAttribute(cur_attr_name));
              });
              connect(button, SIGNAL(clicked()), this, SLOT(openDataContext()));
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
              button->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditFind));
#else
              button->setText("Ref");
#endif
              grid_layout->addWidget(label, 0, 0, 1, 2);
              grid_layout->addWidget(line_edit, 0, 2, 1, 2);
              grid_layout->addWidget(button, 0, 4);
              widget->setLayout(grid_layout);
              form->addRow(widget);
            }
          else
            {
              form->addRow(label, line_edit);
            }
          if (!advanced_editor)
            form->setRowVisible(form->rowCount() - 1,
                                !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
          if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
            {
              if (std::find(context_attributes.begin(), context_attributes.end(), cur_attr_name) !=
                      context_attributes.end() &&
                  static_cast<int>((*current_selection)->getRef()->getAttribute(cur_attr_name).type()) == 3) // string
                {
                  auto widget = new QWidget();
                  auto grid_layout = new QGridLayout();
                  auto button = new QPushButton("Ref");
                  connect(button, &QPushButton::clicked, [=]() {
                    if ((*current_selection)->getRef()->hasAttribute(cur_attr_name))
                      context_name =
                          static_cast<std::string>((*current_selection)->getRef()->getAttribute(cur_attr_name));
                  });
                  connect(button, SIGNAL(clicked()), this, SLOT(openDataContext()));
                  grid_layout->addWidget(label, 0, 0, 1, 2);
                  grid_layout->addWidget(line_edit, 0, 2, 1, 2);
                  grid_layout->addWidget(button, 0, 4);
                  widget->setLayout(grid_layout);
                  form->addRow(widget);
                }
              else
                {
                  form->addRow(label, line_edit);
                }
            }
#endif
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
                  if ((*current_selection)->getRef()->localName() == "layout_grid" &&
                      (*current_selection)->getRef()->parentElement()->localName() != "layout_grid")
                    {
                      // special case for flat layout_grid cause other attributes than these below doesnt affect
                      // anything
                      if (attr_name != "trim_col" && attr_name != "trim_row" && attr_name != "flip_col_and_row" &&
                          attr_name != "num_row" && attr_name != "num_col")
                        continue;
                    }

                  auto type_name = static_cast<std::string>(child->getAttribute("type"));
                  attr_type.emplace(attr_name, type_name);
                  QString tooltip_string =
                      GRM::Render::getDefaultAndTooltip((*current_selection)->getRef(), attr_name)[1].c_str();
                  tooltip_string.append(".  Default: ");
                  tooltip_string.append(
                      GRM::Render::getDefaultAndTooltip((*current_selection)->getRef(), attr_name)[0].c_str());

                  if (combo_box_attr.contains(attr_name.c_str()) &&
                      ((attr_name != "x" && attr_name != "y") || type_name == "xs:string"))
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
                  QString text_label = QString("<span style='color:#ff0000;'>%1</span>").arg(attr_name.c_str());
                  auto label = new QLabel(text_label);
                  label->setToolTip(tooltip_string);
                  if (highlight_location && attr_name == "location")
                    {
                      text_label = QString("<span style='color:#0000ff;'>%1</span>").arg(attr_name.c_str());
                      label = new QLabel(text_label);
                      label->setToolTip(tooltip_string);
                      form->addRow(label, line_edit);
                    }
                  else if (attr_name == "x_lim_min" || attr_name == "x_lim_max" || attr_name == "y_lim_min" ||
                           attr_name == "y_lim_max" || attr_name == "z_lim_min" || attr_name == "z_lim_max" ||
                           attr_name == "c_lim_min" || attr_name == "c_lim_max" || attr_name == "adjust_x_lim" ||
                           attr_name == "adjust_y_lim" || attr_name == "adjust_z_lim" || attr_name == "r_lim_min" ||
                           attr_name == "r_lim_max" || attr_name == "theta_lim_min" || attr_name == "theta_lim_max")
                    {
                      if (!lim_modification_added)
                        {
                          form->addRow(lim_modification);
                          lim_modification_added = true;
                        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                      lim_modification_form->addRow(label, line_edit);
                      if (!advanced_editor)
                        lim_modification_form->setRowVisible(
                            lim_modification_form->rowCount() - 1,
                            !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#else
                      if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        lim_modification_form->addRow(label, line_edit);
#endif
                    }
                  else if (attr_name == "x_flip" || attr_name == "y_flip" || attr_name == "z_flip" ||
                           attr_name == "theta_flip")
                    {
                      if (!flip_modification_added)
                        {
                          form->addRow(flip_modification);
                          flip_modification_added = true;
                        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                      flip_modification_form->addRow(label, line_edit);
                      if (!advanced_editor)
                        flip_modification_form->setRowVisible(
                            flip_modification_form->rowCount() - 1,
                            !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#else
                      if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        flip_modification_form->addRow(label, line_edit);
#endif
                    }
                  else if (attr_name == "space_rotation" || attr_name == "space_tilt" || attr_name == "space_z_min" ||
                           attr_name == "space_z_max" || attr_name == "space_3d_camera_distance" ||
                           attr_name == "space_3d_fov" || attr_name == "space_3d_phi" || attr_name == "space_3d_theta")
                    {
                      if (!space_modification_added)
                        {
                          form->addRow(space_modification);
                          space_modification_added = true;
                        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                      space_modification_form->addRow(label, line_edit);
                      if (!advanced_editor)
                        space_modification_form->setRowVisible(
                            space_modification_form->rowCount() - 1,
                            !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#else
                      if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        space_modification_form->addRow(label, line_edit);
#endif
                    }
                  else if (attr_name == "window_x_min" || attr_name == "window_x_max" || attr_name == "window_y_min" ||
                           attr_name == "window_y_max" || attr_name == "window_z_min" || attr_name == "window_z_max")
                    {
                      if (!window_added)
                        {
                          form->addRow(window);
                          window_added = true;
                        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                      window_form->addRow(label, line_edit);
                      if (!advanced_editor)
                        window_form->setRowVisible(window_form->rowCount() - 1,
                                                   !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#else
                      if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        window_form->addRow(label, line_edit);
#endif
                    }
                  else if (attr_name == "x_log" || attr_name == "y_log" || attr_name == "z_log" ||
                           attr_name == "r_log" || attr_name == "theta_log")
                    {
                      if (!log_modification_added)
                        {
                          form->addRow(log_modification);
                          log_modification_added = true;
                        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                      log_modification_form->addRow(label, line_edit);
                      if (!advanced_editor)
                        log_modification_form->setRowVisible(
                            log_modification_form->rowCount() - 1,
                            !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#else
                      if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        log_modification_form->addRow(label, line_edit);
#endif
                    }
                  else if (attr_name == "ws_window_x_min" || attr_name == "ws_window_x_max" ||
                           attr_name == "ws_window_y_min" || attr_name == "ws_window_y_max" ||
                           attr_name == "ws_viewport_x_min" || attr_name == "ws_viewport_x_max" ||
                           attr_name == "ws_viewport_y_min" || attr_name == "ws_viewport_y_max")
                    {
                      text_label = QString(attr_name.c_str());
                      if (!ws_modification_added && advanced_editor)
                        {
                          form->addRow(ws_modification);
                          ws_modification_added = true;
                        }

                      ws_modification_form->addRow(label, line_edit);
                    }
                  else
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
                      if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
#endif
                    {
                      if (std::find(context_attributes.begin(), context_attributes.end(), attr_name) !=
                              context_attributes.end() &&
                          type_name == "xs:string")
                        {
                          auto widget = new QWidget();
                          auto grid_layout = new QGridLayout();
                          auto button = new QPushButton();
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                          button->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditFind));
#else
                          button->setText("Ref");
#endif
                          connect(button, &QPushButton::clicked, [=]() {
                            if ((*current_selection)->getRef()->hasAttribute(attr_name))
                              context_name =
                                  static_cast<std::string>((*current_selection)->getRef()->getAttribute(attr_name));
                          });
                          connect(button, SIGNAL(clicked()), this, SLOT(openDataContext()));
                          grid_layout->addWidget(label, 0, 0, 1, 2);
                          grid_layout->addWidget(line_edit, 0, 2, 1, 2);
                          grid_layout->addWidget(button, 0, 4);
                          widget->setLayout(grid_layout);
                          form->addRow(widget);
                        }
                      else
                        {
                          form->addRow(label, line_edit);
                        }
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                      if (!advanced_editor)
                        form->setRowVisible(form->rowCount() - 1,
                                            !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#endif
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
                          if ((*current_selection)->getRef()->localName() == "coordinate_system" ||
                              ((*current_selection)->getRef()->localName() == "layout_grid" &&
                               (*current_selection)->getRef()->parentElement()->localName() != "layout_grid"))
                            {
                              // special case for coordinate_system cause the ndc movements results in a nonsense plot
                              if (attr_name == "x_scale_ndc" || attr_name == "y_scale_ndc" ||
                                  attr_name == "x_shift_ndc" || attr_name == "y_shift_ndc")
                                continue;
                            }
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

                              if (combo_box_attr.contains(attr_name.c_str()) &&
                                  ((attr_name != "x" && attr_name != "y") || type_name == "xs:string"))
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
                              else if (color_ind_attr.contains(attr_name.c_str()))
                                {
                                  auto index =
                                      static_cast<int>((*current_selection)->getRef()->getAttribute(attr_name));
                                  line_edit = new QPushButton(std::to_string(index).c_str(), this);
                                  ((QPushButton *)line_edit)->setToolTip(tooltip_string);
                                  ((QPushButton *)line_edit)->setObjectName(attr_name.c_str());
                                  QObject::connect(((QPushButton *)line_edit), SIGNAL(clicked()), this,
                                                   SLOT(colorIndexSlot()));
                                  QObject::connect(((QPushButton *)line_edit), &QPushButton::clicked, [=]() {
                                    auto new_index =
                                        static_cast<int>((*current_selection)->getRef()->getAttribute(attr_name));
                                    ((QPushButton *)line_edit)->setText(std::to_string(new_index).c_str());
                                    QImage new_image(1, 1, QImage::Format_RGB32);
                                    QRgb new_value;
                                    int err;
                                    double new_r, new_g, new_b;

                                    gks_inq_color_rep(-1, new_index, -1, &err, &new_r, &new_g, &new_b);
                                    new_value = qRgb(255 * new_r, 255 * new_g, 255 * new_b);
                                    new_image.setPixel(0, 0, new_value);

                                    auto new_color_pic = QPixmap::fromImage(new_image);
                                    new_color_pic = new_color_pic.scaled(20, 20);
                                    ((QPushButton *)line_edit)->setIcon(QIcon(new_color_pic));
                                  });
                                }
                              else if (color_rgb_attr.contains(attr_name.c_str()))
                                {
                                  line_edit = new QPushButton(this);
                                  ((QPushButton *)line_edit)->setToolTip(tooltip_string);
                                  ((QPushButton *)line_edit)->setObjectName(attr_name.c_str());
                                  QObject::connect(((QPushButton *)line_edit), SIGNAL(clicked()), this,
                                                   SLOT(colorRGBSlot()));
                                  QObject::connect(((QPushButton *)line_edit), &QPushButton::clicked, [=]() {
                                    QImage new_image(1, 1, QImage::Format_RGB32);
                                    QRgb new_value;
                                    if ((*current_selection)->getRef()->hasAttribute(attr_name))
                                      {
                                        double ref_r = 0, ref_g = 0, ref_b = 0;
                                        std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();
                                        auto context_ref = static_cast<std::string>(
                                            (*current_selection)->getRef()->getAttribute(attr_name));
                                        auto rgb_vec = GRM::get<std::vector<double>>((*context)[context_ref]);
                                        ref_r = rgb_vec.at(0);
                                        ref_g = rgb_vec.at(1);
                                        ref_b = rgb_vec.at(2);
                                        new_value = qRgb(255 * ref_r, 255 * ref_g, 255 * ref_b);
                                        new_image.setPixel(0, 0, new_value);

                                        ((QPushButton *)line_edit)->setText(context_ref.c_str());
                                        auto new_color_pic = QPixmap::fromImage(new_image);
                                        new_color_pic = new_color_pic.scaled(20, 20);
                                        ((QPushButton *)line_edit)->setIcon(QIcon(new_color_pic));
                                      }
                                  });
                                }
                              else
                                {
                                  line_edit = new QLineEdit(this);
                                  ((QLineEdit *)line_edit)->setToolTip(tooltip_string);
                                  ((QLineEdit *)line_edit)->setText("");
                                }
                              QString text_label =
                                  QString("<span style='color:#ff0000;'>%1</span>").arg(attr_name.c_str());
                              auto label = new QLabel(text_label);
                              label->setToolTip(tooltip_string);
                              if (attr_name == "line_color_ind" || attr_name == "line_color_rgb" ||
                                  attr_name == "line_spec" || attr_name == "line_type" || attr_name == "line_width")
                                {
                                  if (!line_modification_added)
                                    {
                                      form->addRow(line_modification);
                                      line_modification_added = true;
                                    }
                                  line_modification_form->addRow(label, line_edit);
                                }
                              else if (attr_name == "marker_color_ind" || attr_name == "marker_color_indices" ||
                                       attr_name == "marker_size" || attr_name == "marker_sizes" ||
                                       attr_name == "marker_type" || attr_name == "border_color_ind" ||
                                       attr_name == "border_width")
                                {
                                  if (!marker_modification_added)
                                    {
                                      form->addRow(marker_modification);
                                      marker_modification_added = true;
                                    }

                                  marker_modification_form->addRow(label, line_edit);
                                }
                              else if (attr_name == "text_align_horizontal" || attr_name == "text_align_vertical" ||
                                       attr_name == "text_color_ind" || attr_name == "font" ||
                                       attr_name == "font_precision")
                                {
                                  if (!text_modification_added)
                                    {
                                      form->addRow(text_modification);
                                      text_modification_added = true;
                                    }

                                  text_modification_form->addRow(label, line_edit);
                                }
                              else if (attr_name == "fill_color_ind" || attr_name == "fill_color_rgb" ||
                                       attr_name == "fill_int_style" || attr_name == "fill_style")
                                {
                                  if (!fill_modification_added)
                                    {
                                      form->addRow(fill_modification);
                                      fill_modification_added = true;
                                    }

                                  fill_modification_form->addRow(label, line_edit);
                                }
                              else if (attr_name == "viewport_x_min" || attr_name == "viewport_x_max" ||
                                       attr_name == "viewport_y_min" || attr_name == "viewport_y_max")
                                {
                                  if (!viewport_added && advanced_editor)
                                    {
                                      form->addRow(viewport);
                                      viewport_added = true;
                                    }

                                  viewport_form->addRow(label, line_edit);
                                }
                              else if (attr_name == "viewport_normalized_x_min" ||
                                       attr_name == "viewport_normalized_x_max" ||
                                       attr_name == "viewport_normalized_y_min" ||
                                       attr_name == "viewport_normalized_y_max")
                                {
                                  if (!viewport_normalized_added && advanced_editor)
                                    {
                                      form->addRow(viewport_normalized);
                                      viewport_normalized_added = true;
                                    }

                                  viewport_normalized_form->addRow(label, line_edit);
                                }
                              else if (attr_name == "x_range_min" || attr_name == "x_range_max" ||
                                       attr_name == "y_range_min" || attr_name == "y_range_max" ||
                                       attr_name == "z_range_min" || attr_name == "z_range_max" ||
                                       attr_name == "c_range_min" || attr_name == "c_range_max" ||
                                       attr_name == "r_range_min" || attr_name == "r_range_max" ||
                                       attr_name == "theta_range_min" || attr_name == "theta_range_max")
                                {
                                  if (!range_modification_added)
                                    {
                                      form->addRow(range_modification);
                                      range_modification_added = true;
                                    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                                  range_modification_form->addRow(label, line_edit);
                                  if (!advanced_editor)
                                    range_modification_form->setRowVisible(
                                        range_modification_form->rowCount() - 1,
                                        !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#else
                                  if (advanced_editor ||
                                      !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                                    range_modification_form->addRow(label, line_edit);
#endif
                                }
                              else if (attr_name == "x_scale_ndc" || attr_name == "x_shift_ndc" ||
                                       attr_name == "x_scale_wc" || attr_name == "x_shift_wc" ||
                                       attr_name == "y_scale_ndc" || attr_name == "y_shift_ndc" ||
                                       attr_name == "y_scale_wc" || attr_name == "y_shift_wc")
                                {
                                  if (!element_movement_modification_added)
                                    {
                                      if (advanced_editor || (attr_name != "x_scale_wc" && attr_name != "x_shift_wc" &&
                                                              attr_name != "y_scale_wc" && attr_name != "y_shift_wc"))
                                        {
                                          form->addRow(element_movement_modification);
                                          element_movement_modification_added = true;
                                        }
                                    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                                  element_movement_modification_form->addRow(label, line_edit);
                                  if (!advanced_editor)
                                    element_movement_modification_form->setRowVisible(
                                        element_movement_modification_form->rowCount() - 1,
                                        !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#else
                                  if (advanced_editor ||
                                      !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                                    element_movement_modification_form->addRow(label, line_edit);
#endif
                                }
                              else
                                {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                                  form->addRow(label, line_edit);
                                  if (!advanced_editor)
                                    form->setRowVisible(
                                        form->rowCount() - 1,
                                        !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#else
                                  if (advanced_editor ||
                                      !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                                    form->addRow(label, line_edit);
#endif
                                }

                              labels << text_label;
                              fields << line_edit;
                            }
                        }
                    }
                }
              else if (advanced_editor)
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

  line_modification_layout->addLayout(line_modification_form);
  line_modification->setContentLayout(*line_modification_layout);
  marker_modification_layout->addLayout(marker_modification_form);
  marker_modification->setContentLayout(*marker_modification_layout);
  text_modification_layout->addLayout(text_modification_form);
  text_modification->setContentLayout(*text_modification_layout);
  fill_modification_layout->addLayout(fill_modification_form);
  fill_modification->setContentLayout(*fill_modification_layout);
  viewport_layout->addLayout(viewport_form);
  viewport->setContentLayout(*viewport_layout);
  viewport_normalized_layout->addLayout(viewport_normalized_form);
  viewport_normalized->setContentLayout(*viewport_normalized_layout);
  window_layout->addLayout(window_form);
  window->setContentLayout(*window_layout);
  range_modification_layout->addLayout(range_modification_form);
  range_modification->setContentLayout(*range_modification_layout);
  log_modification_layout->addLayout(log_modification_form);
  log_modification->setContentLayout(*log_modification_layout);
  flip_modification_layout->addLayout(flip_modification_form);
  flip_modification->setContentLayout(*flip_modification_layout);
  lim_modification_layout->addLayout(lim_modification_form);
  lim_modification->setContentLayout(*lim_modification_layout);
  element_movement_modification_layout->addLayout(element_movement_modification_form);
  element_movement_modification->setContentLayout(*element_movement_modification_layout);
  space_modification_layout->addLayout(space_modification_form);
  space_modification->setContentLayout(*space_modification_layout);
  ws_modification_layout->addLayout(ws_modification_form);
  ws_modification->setContentLayout(*ws_modification_layout);

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
  grplot_widget->createHistoryElement();
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
              if ((*current_selection)->getRef()->hasAttribute("_" + labels[i].toStdString() + "_set_by_user"))
                (*current_selection)->getRef()->removeAttribute("_" + labels[i].toStdString() + "_set_by_user");
            }
          else
            {
              if (labels[i].toStdString() == "text")
                {
                  const auto value = ((QLineEdit *)fields[i])->text().toStdString();
                  if ((attr_type[attr_name] == "xs:string" || attr_type[attr_name] == "strint") &&
                      !util::isDigits(value))
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
                  else if (attr_type[attr_name] == "xs:double" && util::isNumber(value))
                    {
                      (*current_selection)
                          ->getRef()
                          ->parentElement()
                          ->setAttribute(labels[i].toStdString(), std::stod(value));
                    }
                  else if ((attr_type[attr_name] == "xs:integer" || attr_type[attr_name] == "strint") &&
                           util::isDigits(value))
                    {
                      (*current_selection)
                          ->getRef()
                          ->parentElement()
                          ->setAttribute(labels[i].toStdString(), std::stoi(value));
                    }
                  else
                    {
                      fprintf(stderr, "Invalid value %s for attribute %s with type %s\n", value.c_str(),
                              attr_name.c_str(), attr_type[attr_name].c_str());
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
                  if ((attr_type[attr_name] == "xs:string" || attr_type[attr_name] == "strint") &&
                      !util::isDigits(value))
                    {
                      (*current_selection)->getRef()->setAttribute(labels[i].toStdString(), value);
                    }
                  else if (attr_type[attr_name] == "xs:string" &&
                           (labels[i].toStdString() == "arc_label" || labels[i].toStdString() == "angle_label" ||
                            labels[i].toStdString() == "x_label" || labels[i].toStdString() == "y_label" ||
                            labels[i].toStdString() == "z_label" || labels[i].toStdString() == "tick_label" ||
                            labels[i].toStdString() == "text" || labels[i].toStdString() == "x_label_3d" ||
                            labels[i].toStdString() == "y_label_3d" || labels[i].toStdString() == "z_label_3d"))
                    {
                      (*current_selection)->getRef()->setAttribute(labels[i].toStdString(), value);
                    }
                  else if (attr_type[attr_name] == "xs:double" && util::isNumber(value))
                    {
                      (*current_selection)->getRef()->setAttribute(labels[i].toStdString(), std::stod(value));
                    }
                  else if ((attr_type[attr_name] == "xs:integer" || attr_type[attr_name] == "strint") &&
                           util::isDigits(value))
                    {
                      (*current_selection)->getRef()->setAttribute(labels[i].toStdString(), std::stoi(value));
                    }
                  else
                    {
                      fprintf(stderr, "Invalid value %s for attribute %s with type %s\n", value.c_str(),
                              attr_name.c_str(), attr_type[attr_name].c_str());
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
              if ((*current_selection)->getRef()->hasAttribute("_" + labels[i].toStdString() + "_set_by_user"))
                (*current_selection)->getRef()->removeAttribute("_" + labels[i].toStdString() + "_set_by_user");
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

void EditElementWidget::colorIndexSlot()
{
  auto sender_ref = sender();
  std::string attribute_name = sender_ref->objectName().toStdString();
  auto current_selection = grplot_widget->getCurrentSelection();
  auto index = static_cast<int>((*current_selection)->getRef()->getAttribute(attribute_name));
  grplot_widget->colorIndexPopUp(attribute_name, index, (*current_selection)->getRef());
}

void EditElementWidget::colorRGBSlot()
{
  auto sender_ref = sender();
  std::string attribute_name = sender_ref->objectName().toStdString();
  auto current_selection = grplot_widget->getCurrentSelection();
  grplot_widget->colorRGBPopUp(attribute_name, (*current_selection)->getRef());
}

bool EditElementWidget::isAdvancedAttribute(const std::shared_ptr<GRM::Element> &element, std::string attr_name)
{
  // hide all set and non set attributes which allow a graphical modifaction of the figure if the advanced editor isn't
  // turned on
  auto elem_name = element->localName();
  static std::unordered_map<std::string, std::vector<std::string>> element_to_advanced_attributes{
      {std::string("figure"),
       std::vector<std::string>{
           "active",
       }},
      {std::string("layout_grid"),
       std::vector<std::string>{
           "absolute_height",
           "absolute_width",
           "aspect_ratio",
           "fit_parents_height",
           "fit_parents_width",
           "relative_height",
           "relative_width",
           "start_col",
           "start_row",
           "stop_col",
           "stop_row",
       }},
      {std::string("layout_grid_element"),
       std::vector<std::string>{
           "absolute_height",
           "absolute_width",
           "aspect_ratio",
           "fit_parents_height",
           "fit_parents_width",
           "relative_height",
           "relative_width",
       }},
      {std::string("plot"),
       std::vector<std::string>{
           "",
       }},
      {std::string("marginal_heatmap_plot"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "x_ind",
           "y_ind",
       }},
      {std::string("central_region"),
       std::vector<std::string>{
           "",
       }},
      {std::string("side_region"),
       std::vector<std::string>{
           "",
       }},
      {std::string("side_plot_region"),
       std::vector<std::string>{
           "",
       }},
      {std::string("text_region"),
       std::vector<std::string>{
           "",
       }},
      {std::string("coordinate_system"),
       std::vector<std::string>{
           "",
       }},
      {std::string("grid_3d"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("axis"),
       std::vector<std::string>{
           "z_index",
       }},
      {std::string("tick_group"),
       std::vector<std::string>{
           "z_index",
       }},
      {std::string("tick"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("grid_line"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("axes_3d"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("text"),
       std::vector<std::string>{
           "z_index",
       }},
      {std::string("titles_3d"),
       std::vector<std::string>{
           "z_index",
       }},
      {std::string("legend"),
       std::vector<std::string>{
           "z_index",
       }},
      {std::string("label"),
       std::vector<std::string>{
           "",
       }},
      {std::string("radial_axes"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("arc_grid_line"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("theta_axes"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("angle_line"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "text_x0",
           "text_y0",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("error_bars"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("polyline_3d"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x",
           "x_scale_wc",
           "x_shift_wc",
           "y",
           "y_scale_wc",
           "y_shift_wc",
           "z",
       }},
      {std::string("polyline"),
       std::vector<std::string>{
           "x",
           "x_scale_wc",
           "x_shift_wc",
           "y",
           "y_scale_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("polymarker"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x",
           "x_scale_wc",
           "x_shift_wc",
           "y",
           "y_scale_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("polymarker_3d"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x",
           "x_scale_wc",
           "x_shift_wc",
           "y",
           "y_scale_wc",
           "y_shift_wc",
           "z",
       }},
      {std::string("fill_rect"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x",
           "x_max",
           "x_min",
           "x_scale_wc",
           "x_shift_wc",
           "y",
           "y_max",
           "y_min",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("draw_rect"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x",
           "x_max",
           "x_min",
           "x_scale_wc",
           "x_shift_wc",
           "y",
           "y_max",
           "y_min",
           "y_scale_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("colorbar"),
       std::vector<std::string>{
           "",
       }},
      {std::string("cell_array"),
       std::vector<std::string>{
           "color_ind_values",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "num_col",
           "num_row",
           "select_specific_xform",
           "start_col",
           "start_row",
           "x_dim",
           "x_max",
           "x_min",
           "x_scale_wc",
           "x_shift_wc",
           "y_dim",
           "y_max",
           "y_min",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("nonuniform_cell_array"),
       std::vector<std::string>{
           "color_ind_values",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "num_col",
           "num_row",
           "select_specific_xform",
           "start_col",
           "start_row",
           "x",
           "x_dim",
           "x_scale_wc",
           "x_shift_wc",
           "y",
           "y_dim",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("polar_cell_array"),
       std::vector<std::string>{
           "color_ind_values",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "num_col",
           "num_row",
           "select_specific_xform",
           "r_dim",
           "r_max",
           "r_min",
           "r_org",
           "start_col",
           "start_row",
           "theta_dim",
           "theta_max",
           "theta_min",
           "theta_org",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("nonuniform_polar_cell_array"),
       std::vector<std::string>{
           "color_ind_values",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "num_col",
           "num_row",
           "select_specific_xform",
           "r",
           "r_dim",
           "r_org",
           "start_col",
           "start_row",
           "theta",
           "theta_dim",
           "theta_org",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("draw_image"),
       std::vector<std::string>{
           "data",
           "disable_x_trans",
           "disable_y_trans",
           "height",
           "movable",
           "width",
           "x_max",
           "x_min",
           "x_scale_wc",
           "x_shift_wc",
           "y_max",
           "y_min",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("draw_arc"), std::vector<std::string>{"disable_x_trans", "disable_y_trans", "end_angle", "movable",
                                                         "start_angle", "x_max", "x_min", "x_scale_wc", "x_shift_wc",
                                                         "y_max", "y_min", "y_scale_wc", "y_shift_wc", "z_index"}},
      {std::string("fill_arc"), std::vector<std::string>{"disable_x_trans", "disable_y_trans", "end_angle", "movable",
                                                         "start_angle", "x_max", "x_min", "x_scale_wc", "x_shift_wc",
                                                         "y_max", "y_min", "y_scale_wc", "y_shift_wc", "z_index"}},
      {std::string("fill_area"),
       std::vector<std::string>{"disable_x_trans", "disable_y_trans", "movable", "x", "x_scale_wc", "x_shift_wc", "y",
                                "y_scale_wc", "y_shift_wc", "z_index"}},
      {std::string("series_barplot"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_contour"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "px",
           "py",
           "pz",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_contourf"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "px",
           "py",
           "pz",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_heatmap"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_hexbin"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_histogram"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_range_max",
           "y_range_min",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_imshow"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_isosurface"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_line"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_nonuniform_heatmap"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_nonuniform_polar_heatmap"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_pie"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_range_max",
           "y_range_min",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_line3"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_polar_heatmap"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_polar_histogram"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_polar_line"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_polar_scatter"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_quiver"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_scatter"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_scatter3"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_shade"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_stairs"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_dummy",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_shade"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_stem"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_surface"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_tricontour"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_trisurface"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_volume"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("series_wireframe"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("error_bar"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("bar"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_scale_wc",
           "x_shift_wc",
           "x1",
           "x2",
           "y_scale_wc",
           "y_shift_wc",
           "y1",
           "y2",
       }},
      {std::string("integral"),
       std::vector<std::string>{
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("integral_group"),
       std::vector<std::string>{
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("polar_bar"),
       std::vector<std::string>{
           "bin_edges",
           "bin_width",
           "bin_widths",
           "class_nr",
           "count",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "norm",
           "theta_flip",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
      {std::string("pie_segment"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "end_angle",
           "movable",
           "start_angle",
           "x_scale_wc",
           "x_shift_wc",
           "y_scale_wc",
           "y_shift_wc",
       }},
  };

  if (element_to_advanced_attributes.count(elem_name))
    if (std::find(element_to_advanced_attributes[elem_name].begin(), element_to_advanced_attributes[elem_name].end(),
                  attr_name) != element_to_advanced_attributes[elem_name].end())
      return true;
  // hide x and y for polar_types - theta and r for 2d_types - z for non 3d_types
  if (element->localName() == "plot")
    {
      auto coordinate_system = element->querySelectors("coordinate_system");
      if (coordinate_system != nullptr)
        {
          auto plot_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
          if (plot_type == "2d")
            {
              if (attr_name == "adjust_z_lim" || attr_name == "r_lim_max" || attr_name == "r_lim_min" ||
                  attr_name == "r_log" || attr_name == "theta_flip" || attr_name == "theta_lim_max" ||
                  attr_name == "theta_lim_min" || attr_name == "z_log" || attr_name == "z_flip" ||
                  attr_name == "z_lim_max" || attr_name == "z_lim_min" || attr_name == "polar_with_pan")
                return true;
            }
          else if (plot_type == "3d")
            {
              if (attr_name == "r_lim_max" || attr_name == "r_lim_min" || attr_name == "r_log" ||
                  attr_name == "theta_flip" || attr_name == "theta_lim_max" || attr_name == "theta_lim_min" ||
                  attr_name == "polar_with_pan")
                return true;
            }
          else if (plot_type == "polar")
            {
              if (attr_name == "adjust_x_lim" || attr_name == "adjust_y_lim" || attr_name == "adjust_z_lim" ||
                  attr_name == "c_lim_max" || attr_name == "c_lim_min" || attr_name == "x_lim_max" ||
                  attr_name == "x_lim_min" || attr_name == "x_log" || attr_name == "y_log" ||
                  attr_name == "y_lim_max" || attr_name == "y_lim_min" || attr_name == "z_flip" ||
                  attr_name == "z_lim_max" || attr_name == "z_lim_min" || attr_name == "z_log")
                return true;
            }
        }
    }
  else if (element->localName() == "central_region")
    {
      auto coordinate_system = element->querySelectors("coordinate_system");
      if (coordinate_system != nullptr)
        {
          auto plot_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
          if (plot_type == "2d")
            {
              if (attr_name == "r_min" || attr_name == "r_max" || attr_name == "space_3d_camera_distance" ||
                  attr_name == "space_3d_fov" || attr_name == "space_3d_theta" || attr_name == "space_3d_phi" ||
                  attr_name == "window_z_max" || attr_name == "window_z_min")
                return true;
            }
          else if (plot_type == "3d")
            {
              if (attr_name == "r_min" || attr_name == "r_max" || attr_name == "space_rotation" ||
                  attr_name == "space_tilt" || attr_name == "space_z_max" || attr_name == "space_z_min")
                return true;
            }
          else if (plot_type == "polar")
            {
              if (attr_name == "space_3d_camera_distance" || attr_name == "space_3d_fov" ||
                  attr_name == "space_3d_theta" || attr_name == "space_3d_phi" || attr_name == "space_rotation" ||
                  attr_name == "space_tilt" || attr_name == "space_z_max" || attr_name == "space_z_min" ||
                  util::startsWith(attr_name, "window_"))
                return true;
            }
        }
    }
  else if (element->localName() == "coordinate_system")
    {
      if (element != nullptr)
        {
          auto plot_type = static_cast<std::string>(element->getAttribute("plot_type"));
          if (plot_type == "2d")
            {
              if (attr_name == "theta_flip" || attr_name == "z_grid" || attr_name == "z_label") return true;
            }
          else if (plot_type == "3d")
            {
              if (attr_name == "theta_flip") return true;
            }
          else if (plot_type == "polar")
            {
              if (attr_name == "x_grid" || attr_name == "x_label" || attr_name == "y_grid" || attr_name == "y_label" ||
                  attr_name == "y_line" || attr_name == "z_grid" || attr_name == "z_label")
                return true;
            }
        }
    }
  return false;
}

void EditElementWidget::openDataContext()
{
  grplot_widget->highlightTableWidgetAt(context_name);
}
