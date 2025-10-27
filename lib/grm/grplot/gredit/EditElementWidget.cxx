#include "EditElementWidget.hxx"
#include "../CollapsibleSection.hxx"
#include "PreviewTextWidget.hxx"

#include <gks.h>
#include <cmath>
#include <QDial>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const int DIAL_OFFSET = 90;
static const int VERTICAL_SPACING = 5;
static const int HORIZONTAL_SPACING = 2;
static const int LABEL_WIDTH = 80;
static std::string context_name;
static bool dial_text_changed = false;

static void clearLayout(QLayout *layout)
{
  if (layout == nullptr) return;
  QLayoutItem *item;
  if (!layout->isEmpty() && layout->count() > 0)
    {
      while (layout->count() > 0 && ((item = layout->takeAt(0))))
        {
          if (item->layout())
            {
              clearLayout(item->layout());
              delete item->layout();
            }
          if (item->widget()) delete item->widget();
          delete item;
        }
    }
  delete layout;
}

static std::string attrNameToLabel(std::string attr_name)
{
  auto label = attr_name;
  std::replace(label.begin(), label.end(), '_', ' ');

  return label;
}

static std::string labelToAttrName(std::string label)
{
  auto attr_name = label;
  std::replace(attr_name.begin(), attr_name.end(), ' ', '_');

  return attr_name;
}

static std::vector<std::string> intersection(std::vector<std::string> v1, std::vector<std::string> v2)
{
  std::vector<std::string> v3;

  std::sort(v1.begin(), v1.end());
  std::sort(v2.begin(), v2.end());

  std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(v3));
  return v3;
}

EditElementWidget::EditElementWidget(GRPlotWidget *widget, QWidget *parent) : QWidget(parent)
{
  grplot_widget = widget;
#if !defined(NO_XERCES_C)
  schema_tree = grplot_widget->getSchemaTree();
#else
  schema_tree = nullptr;
#endif
}

void EditElementWidget::attributeEditEvent(std::vector<std::shared_ptr<GRM::Element>> multiple_selections,
                                           bool highlight_location)
{
  QString text_label;
  bool line_modification_added = false, marker_modification_added = false, text_modification_added = false,
       fill_modification_added = false, viewport_added = false, viewport_normalized_added = false, window_added = false,
       range_modification_added = false, log_modification_added = false, flip_modification_added = false,
       lim_modification_added = false, element_movement_modification_added = false, space_modification_added = false,
       ws_modification_added = false, tick_modification_added = false, tick_label_modification_added = false,
       inherit_from_plot_added = false, absolute_errors_added = false, relative_errors_added = false,
       light_added = false;
  auto current_selection = grplot_widget->getCurrentSelection();
  if (current_selection == nullptr || *current_selection == nullptr)
    {
      this->close();
      return;
    }

  if (this->layout() != nullptr)
    {
      clearLayout(this->layout());
      fields.clear();
      labels.clear();
      attr_type.clear();
      this->multiple_selections.clear();
    }

  context_name = "";

  auto combo_box_attr = grplot_widget->getComboBoxAttributes();
  auto check_box_attr = grplot_widget->getCheckBoxAttributes();
  auto color_ind_attr = grplot_widget->getColorIndAttributes();
  auto color_rgb_attr = grplot_widget->getColorRGBAttributes();
  auto slider_attr = grplot_widget->getSliderAttributes();
  schema_tree = grplot_widget->getSchemaTree();
  auto advanced_editor = grplot_widget->getEnableAdvancedEditor();
  auto context_attributes = GRM::getContextAttributes();

  std::string currently_clicked_name = (*current_selection)->getRef()->localName();
  QString title("Element Edit: ");
  if (multiple_selections.empty() ||
      !((*current_selection)->getRef()->hasAttribute("_selected_for_move") &&
        static_cast<int>((*current_selection)->getRef()->getAttribute("_selected_for_move"))))
    {
      title.append(currently_clicked_name.c_str());
    }
  else
    {
      this->multiple_selections = multiple_selections;
      title = "Intersection Element Edit:";
    }
  this->setWindowTitle(title);
  auto change_parameters_label = new QLabel("Change Parameters:", this);
  change_parameters_label->setStyleSheet("font-weight: bold");
  auto form = new QFormLayout;
  form->addRow(change_parameters_label);
  form->setVerticalSpacing(VERTICAL_SPACING);
  form->setContentsMargins(0, 0, 0, 0);
  form->setHorizontalSpacing(HORIZONTAL_SPACING);

  auto line_modification_form = new QFormLayout;
  line_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  line_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  line_modification_form->setContentsMargins(0, 0, 0, 0);
  line_modification_form->setFormAlignment(Qt::AlignLeft);
  auto line_modification = new CollapsibleSection("Line Modifications", 100, this);
  auto *line_modification_layout = new QVBoxLayout();
  auto marker_modification_form = new QFormLayout;
  marker_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  marker_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  marker_modification_form->setContentsMargins(0, 0, 0, 0);
  marker_modification_form->setFormAlignment(Qt::AlignLeft);
  auto marker_modification = new CollapsibleSection("Marker Modifications", 100, this);
  auto *marker_modification_layout = new QVBoxLayout();
  auto text_modification_form = new QFormLayout;
  text_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  text_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  text_modification_form->setContentsMargins(0, 0, 0, 0);
  text_modification_form->setFormAlignment(Qt::AlignLeft);
  auto text_modification = new CollapsibleSection("Text Modifications", 100, this);
  auto *text_modification_layout = new QVBoxLayout();
  auto fill_modification_form = new QFormLayout;
  fill_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  fill_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  fill_modification_form->setContentsMargins(0, 0, 0, 0);
  fill_modification_form->setFormAlignment(Qt::AlignLeft);
  auto fill_modification = new CollapsibleSection("Fill Modifications", 100, this);
  auto *fill_modification_layout = new QVBoxLayout();
  auto viewport_form = new QFormLayout;
  viewport_form->setVerticalSpacing(VERTICAL_SPACING);
  viewport_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  viewport_form->setContentsMargins(0, 0, 0, 0);
  viewport_form->setFormAlignment(Qt::AlignLeft);
  auto viewport = new CollapsibleSection("Viewport", 100, this);
  auto *viewport_layout = new QVBoxLayout();
  auto viewport_normalized_form = new QFormLayout;
  viewport_normalized_form->setVerticalSpacing(VERTICAL_SPACING);
  viewport_normalized_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  viewport_normalized_form->setContentsMargins(0, 0, 0, 0);
  viewport_normalized_form->setFormAlignment(Qt::AlignLeft);
  auto viewport_normalized = new CollapsibleSection("Viewport Normalized", 100, this);
  auto *viewport_normalized_layout = new QVBoxLayout();
  auto window_form = new QFormLayout;
  window_form->setVerticalSpacing(VERTICAL_SPACING);
  window_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  window_form->setContentsMargins(0, 0, 0, 0);
  window_form->setFormAlignment(Qt::AlignLeft);
  auto window = new CollapsibleSection("Window", 100, this);
  auto *window_layout = new QVBoxLayout();
  auto range_modification_form = new QFormLayout;
  range_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  range_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  range_modification_form->setContentsMargins(0, 0, 0, 0);
  range_modification_form->setFormAlignment(Qt::AlignLeft);
  auto range_modification = new CollapsibleSection("Range Modifications", 100, this);
  auto *range_modification_layout = new QVBoxLayout();
  auto log_modification_form = new QFormLayout;
  log_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  log_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  log_modification_form->setContentsMargins(0, 0, 0, 0);
  log_modification_form->setFormAlignment(Qt::AlignLeft);
  auto log_modification = new CollapsibleSection("Log Modifications", 100, this);
  auto *log_modification_layout = new QVBoxLayout();
  auto flip_modification_form = new QFormLayout;
  flip_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  flip_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  flip_modification_form->setContentsMargins(0, 0, 0, 0);
  flip_modification_form->setFormAlignment(Qt::AlignLeft);
  auto flip_modification = new CollapsibleSection("Flip Modifications", 100, this);
  auto *flip_modification_layout = new QVBoxLayout();
  auto lim_modification_form = new QFormLayout;
  lim_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  lim_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  lim_modification_form->setContentsMargins(0, 0, 0, 0);
  lim_modification_form->setFormAlignment(Qt::AlignLeft);
  auto lim_modification = new CollapsibleSection("Lim Modifications", 100, this);
  auto *lim_modification_layout = new QVBoxLayout();
  auto element_movement_modification_form = new QFormLayout;
  element_movement_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  element_movement_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  element_movement_modification_form->setContentsMargins(0, 0, 0, 0);
  element_movement_modification_form->setFormAlignment(Qt::AlignLeft);
  auto element_movement_modification = new CollapsibleSection("Element Movement Modifications", 100, this);
  auto *element_movement_modification_layout = new QVBoxLayout();
  auto space_modification_form = new QFormLayout;
  space_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  space_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  space_modification_form->setContentsMargins(0, 0, 0, 0);
  space_modification_form->setFormAlignment(Qt::AlignLeft);
  auto space_modification = new CollapsibleSection("Space Modifications", 100, this);
  auto *space_modification_layout = new QVBoxLayout();
  auto ws_modification_form = new QFormLayout;
  ws_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  ws_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  ws_modification_form->setContentsMargins(0, 0, 0, 0);
  ws_modification_form->setFormAlignment(Qt::AlignLeft);
  auto ws_modification = new CollapsibleSection("WS Modifications", 100, this);
  auto *ws_modification_layout = new QVBoxLayout();
  auto tick_modification_form = new QFormLayout;
  tick_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  tick_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  tick_modification_form->setContentsMargins(0, 0, 0, 0);
  tick_modification_form->setFormAlignment(Qt::AlignLeft);
  auto tick_modification = new CollapsibleSection("Tick Modifications", 100, this);
  auto *tick_modification_layout = new QVBoxLayout();
  auto tick_label_modification_form = new QFormLayout;
  tick_label_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  tick_label_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  tick_label_modification_form->setContentsMargins(0, 0, 0, 0);
  tick_label_modification_form->setFormAlignment(Qt::AlignLeft);
  auto tick_label_modification = new CollapsibleSection("Tick Label Modifications", 100, this);
  auto *tick_label_modification_layout = new QVBoxLayout();
  auto inherit_from_plot_modification_form = new QFormLayout;
  inherit_from_plot_modification_form->setVerticalSpacing(VERTICAL_SPACING);
  inherit_from_plot_modification_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  inherit_from_plot_modification_form->setContentsMargins(0, 0, 0, 0);
  inherit_from_plot_modification_form->setFormAlignment(Qt::AlignLeft);
  auto inherit_from_plot_modification = new CollapsibleSection("Modifications inherit from Plot", 100, this);
  auto *inherit_from_plot_modification_layout = new QVBoxLayout();
  auto absolute_errors_form = new QFormLayout;
  absolute_errors_form->setVerticalSpacing(VERTICAL_SPACING);
  absolute_errors_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  absolute_errors_form->setContentsMargins(0, 0, 0, 0);
  absolute_errors_form->setFormAlignment(Qt::AlignLeft);
  auto absolute_errors = new CollapsibleSection("Absolute Errors", 100, this);
  auto *absolute_errors_layout = new QVBoxLayout();
  auto relative_errors_form = new QFormLayout;
  relative_errors_form->setVerticalSpacing(VERTICAL_SPACING);
  relative_errors_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  relative_errors_form->setContentsMargins(0, 0, 0, 0);
  relative_errors_form->setFormAlignment(Qt::AlignLeft);
  auto relative_errors = new CollapsibleSection("Relative Errors", 100, this);
  auto *relative_errors_layout = new QVBoxLayout();
  auto light_form = new QFormLayout;
  light_form->setVerticalSpacing(VERTICAL_SPACING);
  light_form->setHorizontalSpacing(HORIZONTAL_SPACING);
  light_form->setContentsMargins(0, 0, 0, 0);
  light_form->setFormAlignment(Qt::AlignLeft);
  auto light = new CollapsibleSection("Light", 100, this);
  auto *light_layout = new QVBoxLayout();

  QWidget *line_edit;

  if ((*current_selection)->getRef()->localName() == "axis" ||
      (*current_selection)->getRef()->localName() == "coordinate_system")
    {
      std::shared_ptr<GRM::Element> plot_elem;

      if ((*current_selection)->getRef()->localName() == "axis")
        {
          if ((*current_selection)->getRef()->parentElement()->localName() == "coordinate_system")
            plot_elem = (*current_selection)->getRef()->parentElement()->parentElement()->parentElement();
          else
            plot_elem = (*current_selection)->getRef()->parentElement()->parentElement()->parentElement();
        }
      else
        {
          plot_elem = (*current_selection)->getRef()->parentElement()->parentElement();
        }
      for (const auto &attr_name : std::vector<std::string>{
               "x_log",     "y_log",         "z_log",        "r_log",        "x_flip",       "y_flip",
               "z_flip",    "theta_flip",    "adjust_x_lim", "adjust_y_lim", "adjust_z_lim", "x_lim_min",
               "x_lim_max", "y_lim_min",     "y_lim_max",    "z_lim_min",    "z_lim_max",    "r_lim_min",
               "r_lim_max", "theta_lim_min", "theta_lim_max"})
        {
          bool was_added = true;
          QString tooltip_string = GRM::Render::getDefaultAndTooltip(plot_elem, attr_name)[1].c_str();
          tooltip_string.append(".  Default: ");
          tooltip_string.append(GRM::Render::getDefaultAndTooltip(plot_elem, attr_name)[0].c_str());

          if (check_box_attr.contains(attr_name.c_str()))
            {
              line_edit = new QCheckBox(this);
              line_edit->setToolTip(tooltip_string);
              if (multiple_selections.empty() && plot_elem->hasAttribute(attr_name))
                static_cast<QCheckBox *>(line_edit)->setChecked(static_cast<int>(plot_elem->getAttribute(attr_name)) ==
                                                                1);
            }
          else
            {
              if (plot_elem->getAttribute(attr_name).isInt())
                {
                  attr_type.emplace(attr_name, "xs:integer");
                }
              else if (plot_elem->getAttribute(attr_name).isDouble())
                {
                  attr_type.emplace(attr_name, "xs:double");
                }
              else
                {
                  attr_type.emplace(attr_name, "xs:string");
                }
              line_edit = new QLineEdit(this);
              if (multiple_selections.empty() && plot_elem->hasAttribute(attr_name))
                static_cast<QLineEdit *>(line_edit)->setText(
                    static_cast<std::string>(plot_elem->getAttribute(attr_name)).c_str());
              line_edit->setToolTip(tooltip_string);
            }

          text_label = QString(attrNameToLabel(attr_name).c_str());
          if (!plot_elem->hasAttribute(attr_name))
            text_label = QString("<span style='color:#ff0000;'>%1</span>").arg(attrNameToLabel(attr_name).c_str());
          auto label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!inherit_from_plot_added &&
              (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name, true)))
            {
              form->addRow(inherit_from_plot_modification);
              inherit_from_plot_added = true;
            }

          if (inherit_from_plot_added)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
              inherit_from_plot_modification_form->addRow(label, line_edit);
              if (!advanced_editor)
                inherit_from_plot_modification_form->setRowVisible(
                    inherit_from_plot_modification_form->rowCount() - 1,
                    !isAdvancedAttribute((*current_selection)->getRef(), attr_name, true));
#else
              if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name, true))
                inherit_from_plot_modification_form->addRow(label, line_edit);
#endif
            }
          if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), attr_name, true))
            was_added = false;

          if (was_added)
            {
              labels << text_label;
              fields << line_edit;
            }
          else
            {
              text_label.clear();
              label->clear();
              line_edit->close();
            }
        }
    }

  std::vector<std::string> sorted_names;
  if (!multiple_selections.empty())
    {
      for (const auto &cur_attr_name : multiple_selections[0]->getAttributeNames())
        {
          sorted_names.push_back(cur_attr_name);
        }
      for (int i = 1; i < multiple_selections.size() - 1; i++)
        {
          std::vector<std::string> tmp;
          for (const auto &cur_attr_name : multiple_selections[i]->getAttributeNames())
            {
              tmp.push_back(cur_attr_name);
            }
          sorted_names = intersection(sorted_names, tmp);
        }
    }
  else
    {
      for (const auto &cur_attr_name : (*current_selection)->getRef()->getAttributeNames())
        {
          sorted_names.push_back(cur_attr_name);
        }
    }
  std::sort(sorted_names.begin(), sorted_names.end());
  for (const auto &cur_attr_name : sorted_names)
    {
      QLabel *label;
      ;
      bool was_added = true;
      if (util::startsWith(cur_attr_name, "_")) continue;
      if ((*current_selection)->getRef()->localName() == "layout_grid" &&
          (*current_selection)->getRef()->parentElement()->localName() != "layout_grid")
        {
          // special case for flat layout_grid cause other attributes than these below doesn't affect anything
          if (cur_attr_name == "viewport_x_min" || cur_attr_name == "viewport_x_max" ||
              cur_attr_name == "viewport_y_min" || cur_attr_name == "viewport_y_max" ||
              cur_attr_name == "viewport_normalized_x_min" || cur_attr_name == "viewport_normalized_x_max" ||
              cur_attr_name == "viewport_normalized_y_min" || cur_attr_name == "viewport_normalized_y_max" ||
              cur_attr_name == "fit_parents_height" || cur_attr_name == "fit_parents_width" ||
              cur_attr_name == "x_max_shift_ndc" || cur_attr_name == "x_min_shift_ndc" ||
              cur_attr_name == "y_max_shift_ndc" || cur_attr_name == "y_min_shift_ndc" ||
              cur_attr_name == "x_shift_ndc" || cur_attr_name == "y_shift_ndc")
            continue;
        }
      else if ((*current_selection)->getRef()->localName() == "coordinate_system")
        {
          // special case for coordinate_system cause the ndc movements results in a nonsense plot
          if (cur_attr_name == "x_max_shift_ndc" || cur_attr_name == "x_min_shift_ndc" ||
              cur_attr_name == "y_max_shift_ndc" || cur_attr_name == "y_min_shift_ndc" ||
              cur_attr_name == "x_shift_ndc" || cur_attr_name == "y_shift_ndc")
            continue;
        }
      if (cur_attr_name == "text_angle") continue;
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
          line_edit->setToolTip(tooltip_string);
          if (!multiple_selections.empty())
            static_cast<QComboBox *>(line_edit)->setCurrentIndex(static_cast<QComboBox *>(line_edit)->count() - 1);

          if (cur_attr_name == "scientific_format")
            {
              line_edit->setFixedWidth(120);
#if QT_VERSION >= 0x060000
              connect(static_cast<QComboBox *>(line_edit), &QComboBox::currentIndexChanged, this,
                      [=]() { openTextPreview(); });
#endif
            }
        }
      else if (check_box_attr.contains(cur_attr_name.c_str()))
        {
          line_edit = new QCheckBox(this);
          line_edit->setToolTip(tooltip_string);
          if (multiple_selections.empty())
            static_cast<QCheckBox *>(line_edit)->setChecked(
                static_cast<int>((*current_selection)->getRef()->getAttribute(cur_attr_name)) == 1);
        }
      else if (color_ind_attr.contains(cur_attr_name.c_str()))
        {
          auto index = static_cast<int>((*current_selection)->getRef()->getAttribute(cur_attr_name));
          if (multiple_selections.empty())
            line_edit = new QPushButton(std::to_string(index).c_str(), this);
          else
            line_edit = new QPushButton("", this);
          line_edit->setToolTip(tooltip_string);
          line_edit->setObjectName(cur_attr_name.c_str());

          QImage image(1, 1, QImage::Format_RGB32);
          QRgb value;
          int errind;
          double r, g, b;

          gks_inq_color_rep(-1, index, -1, &errind, &r, &g, &b);
          value = qRgb(255 * r, 255 * g, 255 * b);
          image.setPixel(0, 0, value);

          auto color_pic = QPixmap::fromImage(image);
          color_pic = color_pic.scaled(20, 20);
          if (multiple_selections.empty()) static_cast<QPushButton *>(line_edit)->setIcon(QIcon(color_pic));

          QObject::connect(line_edit, SIGNAL(clicked()), this, SLOT(colorIndexSlot()));
          QObject::connect(static_cast<QPushButton *>(line_edit), &QPushButton::clicked, [=]() {
            QImage new_image(1, 1, QImage::Format_RGB32);
            QRgb new_value;
            int err;
            double new_r, new_g, new_b;

            auto new_index = static_cast<int>((*current_selection)->getRef()->getAttribute(cur_attr_name));
            static_cast<QPushButton *>(line_edit)->setText(std::to_string(new_index).c_str());

            gks_inq_color_rep(-1, new_index, -1, &err, &new_r, &new_g, &new_b);
            new_value = qRgb(255 * new_r, 255 * new_g, 255 * new_b);
            new_image.setPixel(0, 0, new_value);

            auto new_color_pic = QPixmap::fromImage(new_image);
            new_color_pic = new_color_pic.scaled(20, 20);
            static_cast<QPushButton *>(line_edit)->setIcon(QIcon(new_color_pic));
          });
        }
      else if (color_rgb_attr.contains(cur_attr_name.c_str()))
        {
          line_edit = new QPushButton(this);
          line_edit->setToolTip(tooltip_string);
          line_edit->setObjectName(cur_attr_name.c_str());

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
          if (multiple_selections.empty()) static_cast<QPushButton *>(line_edit)->setIcon(QIcon(color_pic));

          QObject::connect(line_edit, SIGNAL(clicked()), this, SLOT(colorRGBSlot()));
          QObject::connect(static_cast<QPushButton *>(line_edit), &QPushButton::clicked, [=]() {
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
                static_cast<QPushButton *>(line_edit)->setText(context_ref.c_str());
              }

            new_value = qRgb(255 * ref_r_new, 255 * ref_g_new, 255 * ref_b_new);
            new_image.setPixel(0, 0, new_value);

            auto new_color_pic = QPixmap::fromImage(new_image);
            new_color_pic = new_color_pic.scaled(20, 20);
            static_cast<QPushButton *>(line_edit)->setIcon(QIcon(new_color_pic));
          });
        }
      else if (cur_attr_name == "char_up_x")
        {
          line_edit = new QLineEdit(this);

          auto up_x = static_cast<double>((*current_selection)->getRef()->getAttribute(cur_attr_name));
          auto up_y = static_cast<double>((*current_selection)->getRef()->getAttribute("char_up_y"));
          auto angle = std::atan2(up_y, up_x);
          if (angle < 0) angle += M_PI * 2;
          if (multiple_selections.empty())
            static_cast<QLineEdit *>(line_edit)->setText(QString::number(angle / M_PI * 180, 'f', 2));
          else
            static_cast<QLineEdit *>(line_edit)->setText(QString::number(90, 'f', 2));
          line_edit->setToolTip(tooltip_string);
          line_edit->setMaximumWidth(50);
        }
      else if (cur_attr_name == "char_up_y")
        {
          continue;
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
          if (multiple_selections.empty())
            static_cast<QLineEdit *>(line_edit)->setText(
                static_cast<std::string>((*current_selection)->getRef()->getAttribute(cur_attr_name)).c_str());
          line_edit->setToolTip(tooltip_string);
        }

      if (cur_attr_name == "char_up_x")
        {
          auto widget = new QWidget(this);
          auto grid_layout = new QGridLayout();

          text_label = QString("text angle");
          tooltip_string = QString("The rotation of the text defined by an angle. Default: 90 degrees");

          auto dial = new QDial(this);
          dial->setToolTip("The character up vector which is used to rotate the text. Default: (0,1) or 90 degrees");
          dial->setRange(0, 360);
          dial->setWrapping(true);
          dial->setInvertedAppearance(true);
          dial->setFixedSize(40, 40);
          dial->setNotchesVisible(true);
          dial->setNotchTarget(15);
          dial->setPageStep(15); // for arrow keys to inc angle by 15

          dial->setValue(static_cast<QLineEdit *>(line_edit)->text().toDouble() + DIAL_OFFSET);

          grid_layout->addWidget(line_edit, 0, 1);
          grid_layout->addWidget(dial, 0, 0);
          grid_layout->setContentsMargins(0, 0, 0, 0);
          grid_layout->setSpacing(10);
          widget->setLayout(grid_layout);
          widget->setContentsMargins(0, 0, 0, 0);

          connect(dial, &QSlider::valueChanged, this, [=] {
            double val = dial->value();
            val = qRound((val - 0) / static_cast<double>(179 - 0) * (270 - 90) + 270) % 360;
            val = int((static_cast<int>(val) % 15 >= 7) ? (val / 15 + 1) : (val / 15)) * 15;
            if (!dial_text_changed)
              {
                static_cast<QLineEdit *>(line_edit)->setText(QString::number(val, 'f', 2));
                static_cast<QLineEdit *>(line_edit)->setModified(true);
              }
            dial_text_changed = false;
          });
          connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged, this, [=] {
            double val = static_cast<QLineEdit *>(line_edit)->text().toDouble();
            dial_text_changed = true;
            dial->setValue(val + DIAL_OFFSET);
          });
          form->addRow(text_label, widget);
        }
      else if (cur_attr_name == "line_color_ind" || cur_attr_name == "line_color_rgb" || cur_attr_name == "line_spec" ||
               cur_attr_name == "line_type" || cur_attr_name == "line_width")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!line_modification_added)
            {
              form->addRow(line_modification);
              line_modification_added = true;
            }

          if (line_modification_added) line_modification_form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "marker_color_ind" || cur_attr_name == "marker_color_indices" ||
               cur_attr_name == "marker_size" || cur_attr_name == "marker_sizes" || cur_attr_name == "marker_type" ||
               cur_attr_name == "border_color_ind" || cur_attr_name == "border_width")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!marker_modification_added)
            {
              form->addRow(marker_modification);
              marker_modification_added = true;
            }

          if (marker_modification_added) marker_modification_form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "text_align_horizontal" || cur_attr_name == "text_align_vertical" ||
               cur_attr_name == "text_color_ind" || cur_attr_name == "font" || cur_attr_name == "font_precision")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!text_modification_added)
            {
              form->addRow(text_modification);
              text_modification_added = true;
            }

#if QT_VERSION >= 0x060000
          if (cur_attr_name == "font_precision")
            connect(static_cast<QComboBox *>(line_edit), &QComboBox::currentIndexChanged, this,
                    [=]() { openTextPreview(); });
#endif

          if (text_modification_added) text_modification_form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "fill_color_ind" || cur_attr_name == "fill_color_rgb" ||
               cur_attr_name == "fill_int_style" || cur_attr_name == "fill_style")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!fill_modification_added)
            {
              form->addRow(fill_modification);
              fill_modification_added = true;
            }

          if (fill_modification_added) fill_modification_form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "viewport_x_min" || cur_attr_name == "viewport_x_max" ||
               cur_attr_name == "viewport_y_min" || cur_attr_name == "viewport_y_max")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!viewport_added && advanced_editor)
            {
              form->addRow(viewport);
              viewport_added = true;
            }
          if (!advanced_editor)
            {
              was_added = false;
              if (viewport_added) viewport_form->addRow(label, line_edit);
            }
        }
      else if (cur_attr_name == "viewport_normalized_x_min" || cur_attr_name == "viewport_normalized_x_max" ||
               cur_attr_name == "viewport_normalized_y_min" || cur_attr_name == "viewport_normalized_y_max")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!viewport_normalized_added && advanced_editor)
            {
              form->addRow(viewport_normalized);
              viewport_normalized_added = true;
            }

          auto widget = new QWidget(this);
          auto grid_layout = new QGridLayout();
          auto slider = new QSlider(this);
          slider->setOrientation(Qt::Horizontal);
          slider->setRange(0, 100);
          slider->setFixedWidth(80);
          if (multiple_selections.empty())
            {
              auto value = static_cast<double>((*current_selection)->getRef()->getAttribute(cur_attr_name));
              static_cast<QLineEdit *>(line_edit)->setText(QString::number(value, 'f', 2));
              slider->setValue(value * 100.0);
            }
          line_edit->setMaximumWidth(40);
          grid_layout->addWidget(line_edit, 0, 0);
          grid_layout->addWidget(slider, 0, 1);
          grid_layout->setContentsMargins(0, 0, 0, 0);
          grid_layout->setSpacing(10);
          widget->setLayout(grid_layout);
          widget->setContentsMargins(0, 0, 0, 0);
          widget->setFixedHeight(30);

          connect(slider, &QSlider::sliderMoved, this, [=] {
            double val = slider->value();
            static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
            static_cast<QLineEdit *>(line_edit)->setModified(true);
          });
          connect(slider, &QSlider::sliderPressed, this, [=] {
            double val = slider->value();
            static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
            static_cast<QLineEdit *>(line_edit)->setModified(true);
          });
          connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged, this, [=] {
            double val = static_cast<QLineEdit *>(line_edit)->text().toDouble();
            slider->setValue(val * 100.0);
          });
          if (!advanced_editor)
            {
              was_added = false;
              if (viewport_normalized_added) viewport_normalized_form->addRow(label, widget);
            }
        }
      else if (cur_attr_name == "window_x_min" || cur_attr_name == "window_x_max" || cur_attr_name == "window_y_min" ||
               cur_attr_name == "window_y_max" || cur_attr_name == "window_z_min" || cur_attr_name == "window_z_max")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!window_added && (*current_selection)->getRef()->localName() != "overlay_element" &&
              (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)))
            {
              form->addRow(window);
              window_added = true;
            }

          if (window_added)
            {
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
          if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)) was_added = false;
        }
      else if (cur_attr_name == "x_range_min" || cur_attr_name == "x_range_max" || cur_attr_name == "y_range_min" ||
               cur_attr_name == "y_range_max" || cur_attr_name == "z_range_min" || cur_attr_name == "z_range_max" ||
               cur_attr_name == "c_range_min" || cur_attr_name == "c_range_max" || cur_attr_name == "r_range_min" ||
               cur_attr_name == "r_range_max" || cur_attr_name == "theta_range_min" ||
               cur_attr_name == "theta_range_max")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (advanced_editor && !range_modification_added)
            {
              form->addRow(range_modification);
              range_modification_added = true;
            }

          if (range_modification_added)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
              range_modification_form->addRow(label, line_edit);
              if (!advanced_editor)
                range_modification_form->setRowVisible(range_modification_form->rowCount() - 1, false);
#else
              if (advanced_editor) range_modification_form->addRow(label, line_edit);
#endif
            }
          if (!advanced_editor) was_added = false;
        }
      else if (cur_attr_name == "x_log" || cur_attr_name == "y_log" || cur_attr_name == "z_log" ||
               cur_attr_name == "r_log" || cur_attr_name == "theta_log")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!log_modification_added)
            {
              form->addRow(log_modification);
              log_modification_added = true;
            }

          if (log_modification_added)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
              log_modification_form->addRow(label, line_edit);
              if (!advanced_editor)
                log_modification_form->setRowVisible(
                    log_modification_form->rowCount() - 1,
                    !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
              if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
                log_modification_form->addRow(label, line_edit);
#endif
            }
          if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)) was_added = false;
        }
      else if (cur_attr_name == "x_flip" || cur_attr_name == "y_flip" || cur_attr_name == "z_flip" ||
               cur_attr_name == "theta_flip")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!flip_modification_added &&
              (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)))
            {
              form->addRow(flip_modification);
              flip_modification_added = true;
            }

          if (flip_modification_added)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
              flip_modification_form->addRow(label, line_edit);
              if (!advanced_editor)
                flip_modification_form->setRowVisible(
                    flip_modification_form->rowCount() - 1,
                    !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
              if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
                flip_modification_form->addRow(label, line_edit);
#endif
            }
          if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)) was_added = false;
        }
      else if (cur_attr_name == "x_lim_min" || cur_attr_name == "x_lim_max" || cur_attr_name == "y_lim_min" ||
               cur_attr_name == "y_lim_max" || cur_attr_name == "z_lim_min" || cur_attr_name == "z_lim_max" ||
               cur_attr_name == "c_lim_min" || cur_attr_name == "c_lim_max" || cur_attr_name == "adjust_x_lim" ||
               cur_attr_name == "adjust_y_lim" || cur_attr_name == "adjust_z_lim" || cur_attr_name == "r_lim_min" ||
               cur_attr_name == "r_lim_max" || cur_attr_name == "theta_lim_min" || cur_attr_name == "theta_lim_max")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!lim_modification_added &&
              (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)))
            {
              form->addRow(lim_modification);
              lim_modification_added = true;
            }

          if (lim_modification_added)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
              lim_modification_form->addRow(label, line_edit);
              if (!advanced_editor)
                lim_modification_form->setRowVisible(
                    lim_modification_form->rowCount() - 1,
                    !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
              if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
                lim_modification_form->addRow(label, line_edit);
#endif
            }
          if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)) was_added = false;
        }
      else if (cur_attr_name == "x_max_shift_ndc" || cur_attr_name == "x_min_shift_ndc" ||
               cur_attr_name == "x_shift_ndc" || cur_attr_name == "x_max_shift_wc" ||
               cur_attr_name == "x_min_shift_wc" || cur_attr_name == "x_shift_wc" ||
               cur_attr_name == "y_max_shift_ndc" || cur_attr_name == "y_min_shift_ndc" ||
               cur_attr_name == "y_shift_ndc" || cur_attr_name == "y_max_shift_wc" ||
               cur_attr_name == "y_min_shift_wc" || cur_attr_name == "y_shift_wc")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!element_movement_modification_added &&
              (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)))
            {
              if (advanced_editor || (cur_attr_name != "x_max_shift_wc" && cur_attr_name != "x_min_shift_wc" &&
                                      cur_attr_name != "x_shift_wc" && cur_attr_name != "y_max_shift_wc" &&
                                      cur_attr_name != "y_min_shift_wc" && cur_attr_name != "y_shift_wc") &&
                                         ((*current_selection)->getRef()->hasAttribute("viewport_x_min") ||
                                          (*current_selection)->getRef()->localName() == "text"))
                {
                  form->addRow(element_movement_modification);
                  element_movement_modification_added = true;
                }
              else
                was_added = false;
            }

          if (element_movement_modification_added)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
              if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
                {
                  if (slider_attr.contains(cur_attr_name.c_str()))
                    {
                      auto widget = new QWidget(this);
                      auto grid_layout = new QGridLayout();
                      auto slider = new QSlider(this);
                      slider->setOrientation(Qt::Horizontal);
                      slider->setRange(0, 100);
                      slider->setFixedWidth(80);
                      if (multiple_selections.empty())
                        {
                          auto value = static_cast<double>((*current_selection)->getRef()->getAttribute(cur_attr_name));
                          static_cast<QLineEdit *>(line_edit)->setText(QString::number(value, 'f', 2));
                          slider->setValue(value * 100.0);
                        }
                      line_edit->setMaximumWidth(40);
                      grid_layout->addWidget(line_edit, 0, 0);
                      grid_layout->addWidget(slider, 0, 1);
                      grid_layout->setContentsMargins(0, 0, 0, 0);
                      grid_layout->setSpacing(10);
                      widget->setLayout(grid_layout);
                      widget->setContentsMargins(0, 0, 0, 0);
                      widget->setFixedHeight(30);

                      connect(slider, &QSlider::sliderMoved, this, [=] {
                        double val = slider->value();
                        static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                        static_cast<QLineEdit *>(line_edit)->setModified(true);
                      });
                      connect(slider, &QSlider::sliderPressed, this, [=] {
                        double val = slider->value();
                        static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                        static_cast<QLineEdit *>(line_edit)->setModified(true);
                      });
                      connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged, this, [=] {
                        double val = static_cast<QLineEdit *>(line_edit)->text().toDouble();
                        slider->setValue(val * 100.0);
                      });
                      element_movement_modification_form->addRow(label, widget);
                    }
                  else
                    {
                      element_movement_modification_form->addRow(label, line_edit);
                    }
                }
              if (!advanced_editor)
                element_movement_modification_form->setRowVisible(
                    element_movement_modification_form->rowCount() - 1,
                    !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
              if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
                {
                  if (slider_attr.contains(cur_attr_name.c_str()))
                    {
                      auto widget = new QWidget(this);
                      auto grid_layout = new QGridLayout();
                      auto slider = new QSlider(this);
                      slider->setOrientation(Qt::Horizontal);
                      slider->setRange(0, 100);
                      slider->setFixedWidth(80);
                      if (multiple_selections.empty())
                        {
                          auto value = static_cast<double>((*current_selection)->getRef()->getAttribute(cur_attr_name));
                          ((QLineEdit *)line_edit)->setText(QString::number(value, 'f', 2));
                          slider->setValue(value * 100.0);
                        }
                      line_edit->setMaximumWidth(40);
                      grid_layout->addWidget(line_edit, 0, 0);
                      grid_layout->addWidget(slider, 0, 1);
                      grid_layout->setContentsMargins(0, 0, 0, 0);
                      grid_layout->setSpacing(10);
                      widget->setLayout(grid_layout);
                      widget->setContentsMargins(0, 0, 0, 0);
                      widget->setFixedHeight(30);

                      connect(slider, &QSlider::sliderMoved, this, [=] {
                        double val = slider->value();
                        ((QLineEdit *)line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                        ((QLineEdit *)line_edit)->setModified(true);
                      });
                      connect(slider, &QSlider::sliderPressed, this, [=] {
                        double val = slider->value();
                        ((QLineEdit *)line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                        ((QLineEdit *)line_edit)->setModified(true);
                      });
                      connect(((QLineEdit *)line_edit), &QLineEdit::textChanged, this, [=] {
                        double val = ((QLineEdit *)line_edit)->text().toDouble();
                        slider->setValue(val * 100.0);
                      });
                      element_movement_modification_form->addRow(label, widget);
                    }
                  else
                    {
                      element_movement_modification_form->addRow(label, line_edit);
                    }
                }
#endif
            }
          if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)) was_added = false;
        }
      else if (cur_attr_name == "space_rotation" || cur_attr_name == "space_tilt" || cur_attr_name == "space_z_min" ||
               cur_attr_name == "space_z_max" || cur_attr_name == "space_3d_camera_distance" ||
               cur_attr_name == "space_3d_fov" || cur_attr_name == "space_3d_phi" || cur_attr_name == "space_3d_theta")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!space_modification_added)
            {
              form->addRow(space_modification);
              space_modification_added = true;
            }

          if (space_modification_added)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
              space_modification_form->addRow(label, line_edit);
              if (!advanced_editor)
                space_modification_form->setRowVisible(
                    space_modification_form->rowCount() - 1,
                    !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
              if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
                space_modification_form->addRow(label, line_edit);
#endif
            }
          if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)) was_added = false;
        }
      else if (cur_attr_name == "ws_window_x_min" || cur_attr_name == "ws_window_x_max" ||
               cur_attr_name == "ws_window_y_min" || cur_attr_name == "ws_window_y_max" ||
               cur_attr_name == "ws_viewport_x_min" || cur_attr_name == "ws_viewport_x_max" ||
               cur_attr_name == "ws_viewport_y_min" || cur_attr_name == "ws_viewport_y_max")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!ws_modification_added && advanced_editor)
            {
              form->addRow(ws_modification);
              ws_modification_added = true;
            }
          if (!advanced_editor)
            {
              was_added = false;
              if (ws_modification_added) ws_modification_form->addRow(label, line_edit);
            }
        }
      else if (cur_attr_name == "x_tick" || cur_attr_name == "y_tick" || cur_attr_name == "tick" ||
               cur_attr_name == "tick_orientation" || cur_attr_name == "tick_size" || cur_attr_name == "num_ticks" ||
               cur_attr_name == "scale" ||
               (cur_attr_name == "x_major" || cur_attr_name == "y_major" || cur_attr_name == "major_count") &&
                   (*current_selection)->getRef()->localName() == "axis")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!tick_modification_added &&
              (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)))
            {
              form->addRow(tick_modification);
              tick_modification_added = true;
            }

          if (tick_modification_added)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
              tick_modification_form->addRow(label, line_edit);
              if (!advanced_editor)
                tick_modification_form->setRowVisible(
                    tick_modification_form->rowCount() - 1,
                    !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
              if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
                tick_modification_form->addRow(label, line_edit);
#endif
            }
          if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)) was_added = false;
        }
      else if ((*current_selection)->getRef()->localName() == "axis" &&
               (cur_attr_name == "label_orientation" || cur_attr_name == "label_pos" ||
                cur_attr_name == "num_tick_labels" || cur_attr_name == "scientific_format"))
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!tick_label_modification_added &&
              (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)))
            {
              form->addRow(tick_label_modification);
              tick_label_modification_added = true;
            }

          if (cur_attr_name == "scientific_format")
            {
              line_edit->setFixedWidth(120);
#if QT_VERSION >= 0x060000
              connect(static_cast<QComboBox *>(line_edit), &QComboBox::currentIndexChanged, this,
                      [=]() { openTextPreview(); });
#endif
            }

          if (tick_label_modification_added)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
              tick_label_modification_form->addRow(label, line_edit);
              if (!advanced_editor)
                tick_label_modification_form->setRowVisible(
                    tick_label_modification_form->rowCount() - 1,
                    !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
#else
              if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
                tick_label_modification_form->addRow(label, line_edit);
#endif
            }
          if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name)) was_added = false;
        }
      else if (cur_attr_name == "abs_upwards_e" || cur_attr_name == "abs_downwards_e" ||
               cur_attr_name == "uniform_abs_downwards_e" || cur_attr_name == "uniform_abs_upwards_e")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!absolute_errors_added)
            {
              form->addRow(absolute_errors);
              absolute_errors_added = true;
            }

          if (absolute_errors_added) absolute_errors_form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "rel_upwards_e" || cur_attr_name == "rel_downwards_e" ||
               cur_attr_name == "uniform_rel_downwards_e" || cur_attr_name == "uniform_rel_upwards_e")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!relative_errors_added)
            {
              form->addRow(relative_errors);
              relative_errors_added = true;
            }

          if (relative_errors_added) relative_errors_form->addRow(label, line_edit);
        }
      else if (highlight_location && cur_attr_name == "location")
        {
          text_label = QString("<span style='color:#0000ff;'>%1</span>").arg(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          form->addRow(label, line_edit);
        }
      else if (cur_attr_name == "ambient" || cur_attr_name == "diffuse" || cur_attr_name == "specular" ||
               cur_attr_name == "specular_power")
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
          if (!light_added)
            {
              form->addRow(light);
              light_added = true;
            }

          if (light_added && slider_attr.contains(cur_attr_name.c_str()))
            {
              auto widget = new QWidget(this);
              auto grid_layout = new QGridLayout();
              auto slider = new QSlider(this);
              slider->setOrientation(Qt::Horizontal);
              slider->setRange(0, 100);
              slider->setFixedWidth(80);
              if (multiple_selections.empty())
                {
                  auto value = static_cast<double>((*current_selection)->getRef()->getAttribute(cur_attr_name));
                  static_cast<QLineEdit *>(line_edit)->setText(QString::number(value, 'f', 2));
                  slider->setValue(value * 100.0);
                }
              line_edit->setMaximumWidth(40);
              grid_layout->addWidget(line_edit, 0, 0);
              grid_layout->addWidget(slider, 0, 1);
              grid_layout->setContentsMargins(0, 0, 0, 0);
              grid_layout->setSpacing(10);
              widget->setLayout(grid_layout);
              widget->setContentsMargins(0, 0, 0, 0);
              widget->setFixedHeight(30);

              connect(slider, &QSlider::sliderMoved, this, [=] {
                double val = slider->value();
                static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                static_cast<QLineEdit *>(line_edit)->setModified(true);
              });
              connect(slider, &QSlider::sliderPressed, this, [=] {
                double val = slider->value();
                static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                static_cast<QLineEdit *>(line_edit)->setModified(true);
              });
              connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged, this, [=] {
                double val = static_cast<QLineEdit *>(line_edit)->text().toDouble();
                slider->setValue(val * 100.0);
              });
              light_form->addRow(label, widget);
            }
          else
            {
              light_form->addRow(label, line_edit);
            }
        }
      else
        {
          text_label = QString(attrNameToLabel(cur_attr_name).c_str());
          label = new QLabel(text_label, this);
          label->setFixedWidth(LABEL_WIDTH);
          label->setWordWrap(true);
          label->setToolTip(tooltip_string);
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
          if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
            {
#endif
              if (std::find(context_attributes.begin(), context_attributes.end(), cur_attr_name) !=
                      context_attributes.end() &&
                  static_cast<int>((*current_selection)->getRef()->getAttribute(cur_attr_name).type()) == 3 &&
                  cur_attr_name != "color_rgb") // string
                {
                  auto widget = new QWidget(this);
                  auto grid_layout = new QGridLayout();
                  auto button = new QPushButton(this);
                  connect(button, &QPushButton::clicked, [=]() {
                    if ((*current_selection)->getRef()->hasAttribute(cur_attr_name))
                      context_name =
                          static_cast<std::string>((*current_selection)->getRef()->getAttribute(cur_attr_name));
                  });
                  connect(button, SIGNAL(clicked()), this, SLOT(openDataContext()));
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                  button->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditFind));
#else
              button->setText("Ref");
#endif
                  grid_layout->addWidget(line_edit, 0, 0);
                  button->setFixedWidth(30);
                  grid_layout->addWidget(button, 0, 1);
                  grid_layout->setContentsMargins(0, 0, 0, 0);
                  grid_layout->setSpacing(0);
                  widget->setLayout(grid_layout);
                  widget->setContentsMargins(0, 0, 0, 0);
                  widget->setFixedHeight(30);
                  form->addRow(label, widget);
                }
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
            }
#endif
          else if (cur_attr_name == "text" || cur_attr_name == "x_label_3d" || cur_attr_name == "y_label_3d" ||
                   cur_attr_name == "z_label_3d" || cur_attr_name == "tick_label")
            {
#if QT_VERSION >= 0x060000
              connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged, this, [=]() { openTextPreview(); });
#endif
              form->addRow(label, line_edit);
            }
          else if (slider_attr.contains(cur_attr_name.c_str()))
            {
              auto widget = new QWidget(this);
              auto grid_layout = new QGridLayout();
              auto slider = new QSlider(this);
              slider->setOrientation(Qt::Horizontal);
              if (cur_attr_name == "transparency" || cur_attr_name == "height_abs" || cur_attr_name == "width_abs" ||
                  cur_attr_name == "viewport_height_abs" || cur_attr_name == "viewport_height_rel" ||
                  cur_attr_name == "viewport_width_abs" || cur_attr_name == "viewport_width_rel")
                {
                  slider->setRange(0, 100);
                  slider->setFixedWidth(80);
                  line_edit->setMaximumWidth(40);

                  if (multiple_selections.empty())
                    {
                      auto value = static_cast<double>((*current_selection)->getRef()->getAttribute(cur_attr_name));
                      static_cast<QLineEdit *>(line_edit)->setText(QString::number(value, 'f', 2));
                      slider->setValue(value * 100.0);
                    }
                }
              grid_layout->addWidget(line_edit, 0, 0);
              grid_layout->addWidget(slider, 0, 1);
              grid_layout->setContentsMargins(0, 0, 0, 0);
              grid_layout->setSpacing(10);
              widget->setLayout(grid_layout);
              widget->setContentsMargins(0, 0, 0, 0);
              widget->setFixedHeight(30);

              connect(slider, &QSlider::sliderMoved, this, [=] {
                double val = slider->value();
                static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                static_cast<QLineEdit *>(line_edit)->setModified(true);
              });
              connect(slider, &QSlider::sliderPressed, this, [=] {
                double val = slider->value();
                static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                static_cast<QLineEdit *>(line_edit)->setModified(true);
              });
              connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged, this, [=] {
                double val = static_cast<QLineEdit *>(line_edit)->text().toDouble();
                slider->setValue(val * 100.0);
              });
              form->addRow(label, widget);
            }
          else
            {
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
              if (!advanced_editor && !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name))
                form->addRow(label, line_edit);
              else
                was_added = false;
#else
              form->addRow(label, line_edit);
              if (!advanced_editor)
                form->setRowVisible(form->rowCount() - 1,
                                    !isAdvancedAttribute((*current_selection)->getRef(), cur_attr_name));
              if (!form->isRowVisible(form->rowCount() - 1)) was_added = false;
#endif
            }
        }

      if (was_added)
        {
          labels << text_label;
          fields << line_edit;
        }
      else
        {
          text_label.clear();
          label->clear();
          line_edit->close();
        }
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
              bool was_added = true, not_in = false;
              auto attr_name = static_cast<std::string>(child->getAttribute("name"));
              if (attr_name == "text_angle") continue;
              for (const auto &selection : multiple_selections)
                {
                  std::shared_ptr<GRM::Element> tmp;
                  auto selection_name = selection->localName();
                  auto selections2 = schema_tree->querySelectorsAll("[name=" + selection_name + "]");
                  for (const auto &s : selections2)
                    {
                      if (s->localName() == "xs:element") tmp = s->children()[0];
                    }
                  if (tmp->querySelectors("[name=\"" + attr_name + "\"]") == nullptr) not_in = true;
                }
              if (!multiple_selections.empty() && not_in) continue;
              if (!(*current_selection)->getRef()->hasAttribute(attr_name))
                {
                  /* attributes of an element which aren't already in the tree getting added with red text color
                   */
                  if ((*current_selection)->getRef()->localName() == "layout_grid" &&
                      (*current_selection)->getRef()->parentElement()->localName() != "layout_grid")
                    {
                      // special case for flat layout_grid cause other attributes than these below doesn't affect
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
                      line_edit->setToolTip(tooltip_string);

                      if (attr_name == "scientific_format")
                        {
                          line_edit->setFixedWidth(120);
#if QT_VERSION >= 0x060000
                          connect(static_cast<QComboBox *>(line_edit), &QComboBox::currentIndexChanged, this,
                                  [=]() { openTextPreview(); });
#endif
                        }
                    }
                  else if (check_box_attr.contains(attr_name.c_str()))
                    {
                      line_edit = new QCheckBox(this);
                      line_edit->setToolTip(tooltip_string);
                      static_cast<QCheckBox *>(line_edit)->setChecked(
                          static_cast<int>((*current_selection)->getRef()->getAttribute(attr_name)) == 1);
                    }
                  else if (color_ind_attr.contains(attr_name.c_str()))
                    {
                      line_edit = new QPushButton("", this);
                      line_edit->setToolTip(tooltip_string);
                      line_edit->setObjectName(attr_name.c_str());
                      QObject::connect(line_edit, SIGNAL(clicked()), this, SLOT(colorIndexSlot()));
                      QObject::connect(static_cast<QPushButton *>(line_edit), &QPushButton::clicked, [=]() {
                        auto new_index = static_cast<int>((*current_selection)->getRef()->getAttribute(attr_name));
                        static_cast<QPushButton *>(line_edit)->setText(std::to_string(new_index).c_str());
                        QImage new_image(1, 1, QImage::Format_RGB32);
                        QRgb new_value;
                        int err;
                        double new_r, new_g, new_b;

                        gks_inq_color_rep(-1, new_index, -1, &err, &new_r, &new_g, &new_b);
                        new_value = qRgb(255 * new_r, 255 * new_g, 255 * new_b);
                        new_image.setPixel(0, 0, new_value);

                        auto new_color_pic = QPixmap::fromImage(new_image);
                        new_color_pic = new_color_pic.scaled(20, 20);
                        static_cast<QPushButton *>(line_edit)->setIcon(QIcon(new_color_pic));
                      });
                    }
                  else if (color_rgb_attr.contains(attr_name.c_str()))
                    {
                      line_edit = new QPushButton(this);
                      line_edit->setToolTip(tooltip_string);
                      line_edit->setObjectName(attr_name.c_str());
                      QObject::connect(line_edit, SIGNAL(clicked()), this, SLOT(colorRGBSlot()));
                      QObject::connect(static_cast<QPushButton *>(line_edit), &QPushButton::clicked, [=]() {
                        QImage new_image(1, 1, QImage::Format_RGB32);
                        QRgb new_value;
                        if ((*current_selection)->getRef()->hasAttribute(attr_name))
                          {
                            double ref_r = 0, ref_g = 0, ref_b = 0;
                            std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();
                            auto context_ref =
                                static_cast<std::string>((*current_selection)->getRef()->getAttribute(attr_name));
                            auto rgb_vec = GRM::get<std::vector<double>>((*context)[context_ref]);
                            ref_r = rgb_vec.at(0);
                            ref_g = rgb_vec.at(1);
                            ref_b = rgb_vec.at(2);
                            new_value = qRgb(255 * ref_r, 255 * ref_g, 255 * ref_b);
                            new_image.setPixel(0, 0, new_value);

                            static_cast<QPushButton *>(line_edit)->setText(context_ref.c_str());
                            auto new_color_pic = QPixmap::fromImage(new_image);
                            new_color_pic = new_color_pic.scaled(20, 20);
                            static_cast<QPushButton *>(line_edit)->setIcon(QIcon(new_color_pic));
                          }
                      });
                    }
                  else if (attr_name == "char_up_x")
                    {
                      line_edit = new QLineEdit(this);
                      line_edit->setToolTip("The rotation of the text defined by an angle. Default: 90 degrees");

                      static_cast<QLineEdit *>(line_edit)->setPlaceholderText(QString::number(90, 'f', 2));
                      line_edit->setMaximumWidth(50);
                    }
                  else if (attr_name == "char_up_y")
                    {
                      continue;
                    }
                  else
                    {
                      line_edit = new QLineEdit(this);
                      line_edit->setToolTip(tooltip_string);
                      static_cast<QLineEdit *>(line_edit)->setText("");
                    }
                  text_label =
                      QString("<span style='color:#ff0000;'>%1</span>").arg(attrNameToLabel(attr_name).c_str());
                  auto label = new QLabel(text_label, this);
                  label->setFixedWidth(LABEL_WIDTH);
                  label->setWordWrap(true);
                  label->setToolTip(tooltip_string);

                  if (attr_name == "char_up_x")
                    {
                      auto widget = new QWidget(this);
                      auto grid_layout = new QGridLayout();

                      text_label = QString("<span style='color:#ff0000;'>%1</span>").arg("text angle");
                      tooltip_string = QString("The rotation of the text defined by an angle. Default: 90 degrees");

                      auto dial = new QDial(this);
                      dial->setRange(0, 360);
                      dial->setWrapping(true);
                      dial->setInvertedAppearance(true);
                      dial->setValue(90 + DIAL_OFFSET);
                      dial->setFixedSize(40, 40);
                      dial->setNotchesVisible(true);
                      dial->setNotchTarget(15);
                      dial->setPageStep(15);

                      grid_layout->addWidget(line_edit, 0, 1);
                      grid_layout->addWidget(dial, 0, 0);
                      grid_layout->setContentsMargins(0, 0, 0, 0);
                      grid_layout->setSpacing(10);
                      widget->setLayout(grid_layout);
                      widget->setContentsMargins(0, 0, 0, 0);

                      connect(dial, &QSlider::valueChanged, this, [=] {
                        double val = dial->value();
                        val = qRound((val - 0) / static_cast<double>(179 - 0) * (270 - 90) + 270) % 360;
                        val = int((static_cast<int>(val) % 15 >= 7) ? (val / 15 + 1) : (val / 15)) * 15;
                        if (!dial_text_changed)
                          {
                            static_cast<QLineEdit *>(line_edit)->setText(QString::number(val - DIAL_OFFSET, 'f', 2));
                            static_cast<QLineEdit *>(line_edit)->setModified(true);
                          }
                        dial_text_changed = false;
                      });
                      connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged, this, [=] {
                        double val = static_cast<QLineEdit *>(line_edit)->text().toDouble();
                        dial_text_changed = true;
                        dial->setValue(val + DIAL_OFFSET);
                      });
                      form->addRow(text_label, widget);
                      label->clear();
                    }
                  else if (highlight_location && attr_name == "location")
                    {
                      text_label = QString("<span style='color:#0000ff;'>%1</span>").arg(attr_name.c_str());
                      label = new QLabel(text_label, this);
                      label->setToolTip(tooltip_string);
                      form->addRow(label, line_edit);
                    }
                  else if (attr_name == "x_lim_min" || attr_name == "x_lim_max" || attr_name == "y_lim_min" ||
                           attr_name == "y_lim_max" || attr_name == "z_lim_min" || attr_name == "z_lim_max" ||
                           attr_name == "c_lim_min" || attr_name == "c_lim_max" || attr_name == "adjust_x_lim" ||
                           attr_name == "adjust_y_lim" || attr_name == "adjust_z_lim" || attr_name == "r_lim_min" ||
                           attr_name == "r_lim_max" || attr_name == "theta_lim_min" || attr_name == "theta_lim_max")
                    {
                      if (!lim_modification_added &&
                          (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name)))
                        {
                          form->addRow(lim_modification);
                          lim_modification_added = true;
                        }

                      if (lim_modification_added)
                        {
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
                      if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        was_added = false;
                    }
                  else if (attr_name == "x_flip" || attr_name == "y_flip" || attr_name == "z_flip" ||
                           attr_name == "theta_flip")
                    {
                      if (!flip_modification_added &&
                          (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name)))
                        {
                          form->addRow(flip_modification);
                          flip_modification_added = true;
                        }

                      if (flip_modification_added)
                        {
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
                      if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        was_added = false;
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

                      if (space_modification_added)
                        {
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
                      if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        was_added = false;
                    }
                  else if (attr_name == "window_x_min" || attr_name == "window_x_max" || attr_name == "window_y_min" ||
                           attr_name == "window_y_max" || attr_name == "window_z_min" || attr_name == "window_z_max")
                    {
                      if (!window_added && (*current_selection)->getRef()->localName() != "overlay_element" &&
                          (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name)))
                        {
                          form->addRow(window);
                          window_added = true;
                        }

                      if (window_added)
                        {
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
                      if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        was_added = false;
                    }
                  else if (attr_name == "x_log" || attr_name == "y_log" || attr_name == "z_log" ||
                           attr_name == "r_log" || attr_name == "theta_log")
                    {
                      if (!log_modification_added)
                        {
                          form->addRow(log_modification);
                          log_modification_added = true;
                        }

                      if (log_modification_added)
                        {
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
                      if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        was_added = false;
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
                      if (!advanced_editor)
                        {
                          was_added = false;
                          if (ws_modification_added) ws_modification_form->addRow(label, line_edit);
                        }
                    }
                  else if (attr_name == "x_tick" || attr_name == "y_tick" || attr_name == "tick" ||
                           attr_name == "tick_orientation" || attr_name == "tick_size" || attr_name == "num_ticks" ||
                           attr_name == "scale" ||
                           (attr_name == "x_major" || attr_name == "y_major" || attr_name == "major_count") &&
                               (*current_selection)->getRef()->localName() == "axis")
                    {
                      text_label = QString(attrNameToLabel(attr_name).c_str());
                      if (!tick_modification_added &&
                          (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name)))
                        {
                          form->addRow(tick_modification);
                          tick_modification_added = true;
                        }

                      if (tick_modification_added)
                        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                          tick_modification_form->addRow(label, line_edit);
                          if (!advanced_editor)
                            tick_modification_form->setRowVisible(
                                tick_modification_form->rowCount() - 1,
                                !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#else
                          if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                            tick_modification_form->addRow(label, line_edit);
#endif
                        }
                      if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        was_added = false;
                    }
                  else if ((*current_selection)->getRef()->localName() == "axis" &&
                           (attr_name == "label_orientation" || attr_name == "label_pos" ||
                            attr_name == "num_tick_labels" || attr_name == "scientific_format"))
                    {
                      text_label = QString(attrNameToLabel(attr_name).c_str());
                      if (!tick_label_modification_added &&
                          (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name)))
                        {
                          form->addRow(tick_label_modification);
                          tick_label_modification_added = true;
                        }

                      if (attr_name == "scientific_format")
                        {
                          line_edit->setFixedWidth(120);
#if QT_VERSION >= 0x060000
                          connect(static_cast<QComboBox *>(line_edit), &QComboBox::currentIndexChanged, this,
                                  [=]() { openTextPreview(); });
#endif
                        }

                      if (tick_label_modification_added)
                        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                          tick_label_modification_form->addRow(label, line_edit);
                          if (!advanced_editor)
                            tick_label_modification_form->setRowVisible(
                                tick_label_modification_form->rowCount() - 1,
                                !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#else
                          if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                            tick_label_modification_form->addRow(label, line_edit);
#endif
                        }
                      if (!advanced_editor && isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                        was_added = false;
                    }
                  else if (attr_name == "abs_upwards_e" || attr_name == "abs_downwards_e" ||
                           attr_name == "uniform_abs_downwards_e" || attr_name == "uniform_abs_upwards_e")
                    {
                      text_label = QString(attrNameToLabel(attr_name).c_str());
                      if (!absolute_errors_added)
                        {
                          form->addRow(absolute_errors);
                          absolute_errors_added = true;
                        }

                      if (absolute_errors_added) absolute_errors_form->addRow(label, line_edit);
                    }
                  else if (attr_name == "rel_upwards_e" || attr_name == "rel_downwards_e" ||
                           attr_name == "uniform_rel_downwards_e" || attr_name == "uniform_rel_upwards_e")
                    {
                      text_label = QString(attrNameToLabel(attr_name).c_str());
                      if (!relative_errors_added)
                        {
                          form->addRow(relative_errors);
                          relative_errors_added = true;
                        }

                      if (relative_errors_added) relative_errors_form->addRow(label, line_edit);
                    }
                  else
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
                      if (advanced_editor || !isAdvancedAttribute((*current_selection)->getRef(), attr_name))
#endif
                    {
                      if (std::find(context_attributes.begin(), context_attributes.end(), attr_name) !=
                              context_attributes.end() &&
                          type_name == "xs:string" && attr_name != "color_rgb")
                        {
                          auto widget = new QWidget(this);
                          auto grid_layout = new QGridLayout();
                          auto button = new QPushButton(this);
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
                          grid_layout->addWidget(line_edit, 0, 0);
                          button->setFixedWidth(30);
                          grid_layout->addWidget(button, 0, 1);
                          grid_layout->setContentsMargins(0, 0, 0, 0);
                          grid_layout->setSpacing(0);
                          widget->setLayout(grid_layout);
                          widget->setContentsMargins(0, 0, 0, 0);
                          widget->setFixedHeight(30);
                          form->addRow(label, widget);
                        }
                      else if (attr_name == "text" || attr_name == "x_label_3d" || attr_name == "y_label_3d" ||
                               attr_name == "z_label_3d" || attr_name == "tick_label")
                        {
#if QT_VERSION >= 0x060000
                          connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged, this,
                                  [=]() { openTextPreview(); });
#endif
                          form->addRow(label, line_edit);
                        }
                      else if (attr_name == "ambient" || attr_name == "diffuse" || attr_name == "specular" ||
                               attr_name == "specular_power")
                        {
                          text_label = QString(attrNameToLabel(attr_name).c_str());
                          if (!light_added)
                            {
                              form->addRow(light);
                              light_added = true;
                            }

                          if (light_added && slider_attr.contains(attr_name.c_str()))
                            {
                              auto widget = new QWidget(this);
                              auto grid_layout = new QGridLayout();
                              auto slider = new QSlider(this);
                              slider->setOrientation(Qt::Horizontal);
                              slider->setRange(0, 100);
                              slider->setFixedWidth(80);
                              if (multiple_selections.empty())
                                {
                                  auto value =
                                      static_cast<double>((*current_selection)->getRef()->getAttribute(attr_name));
                                  static_cast<QLineEdit *>(line_edit)->setText(QString::number(value, 'f', 2));
                                  slider->setValue(value * 100.0);
                                }
                              line_edit->setMaximumWidth(40);
                              grid_layout->addWidget(line_edit, 0, 0);
                              grid_layout->addWidget(slider, 0, 1);
                              grid_layout->setContentsMargins(0, 0, 0, 0);
                              grid_layout->setSpacing(10);
                              widget->setLayout(grid_layout);
                              widget->setContentsMargins(0, 0, 0, 0);
                              widget->setFixedHeight(30);

                              connect(slider, &QSlider::sliderMoved, this, [=] {
                                double val = slider->value();
                                static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                                static_cast<QLineEdit *>(line_edit)->setModified(true);
                              });
                              connect(slider, &QSlider::sliderPressed, this, [=] {
                                double val = slider->value();
                                static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                                static_cast<QLineEdit *>(line_edit)->setModified(true);
                              });
                              connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged, this, [=] {
                                double val = static_cast<QLineEdit *>(line_edit)->text().toDouble();
                                slider->setValue(val * 100.0);
                              });
                              light_form->addRow(label, widget);
                            }
                          else
                            {
                              light_form->addRow(label, line_edit);
                            }
                        }
                      else if (slider_attr.contains(attr_name.c_str()))
                        {
                          auto widget = new QWidget(this);
                          auto grid_layout = new QGridLayout();
                          auto slider = new QSlider(this);
                          slider->setOrientation(Qt::Horizontal);
                          if (attr_name == "transparency" || attr_name == "height_abs" || attr_name == "width_abs" ||
                              attr_name == "viewport_height_abs" || attr_name == "viewport_height_rel" ||
                              attr_name == "viewport_width_abs" || attr_name == "viewport_width_rel")
                            {
                              slider->setRange(0, 100);
                              slider->setFixedWidth(80);
                              line_edit->setMaximumWidth(40);
                            }
                          grid_layout->addWidget(line_edit, 0, 0);
                          grid_layout->addWidget(slider, 0, 1);
                          grid_layout->setContentsMargins(0, 0, 0, 0);
                          grid_layout->setSpacing(10);
                          widget->setLayout(grid_layout);
                          widget->setContentsMargins(0, 0, 0, 0);
                          widget->setFixedHeight(30);

                          connect(slider, &QSlider::sliderMoved, this, [=] {
                            double val = slider->value();
                            static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                            static_cast<QLineEdit *>(line_edit)->setModified(true);
                          });
                          connect(slider, &QSlider::sliderPressed, this, [=] {
                            double val = slider->value();
                            static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                            static_cast<QLineEdit *>(line_edit)->setModified(true);
                          });
                          connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged, this, [=] {
                            double val = static_cast<QLineEdit *>(line_edit)->text().toDouble();
                            slider->setValue(val * 100.0);
                          });
                          form->addRow(label, widget);
                        }
                      else
                        {
                          form->addRow(label, line_edit);
                        }
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                      if (!advanced_editor)
                        form->setRowVisible(form->rowCount() - 1,
                                            !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
                      if (!form->isRowVisible(form->rowCount() - 1)) was_added = false;
#endif
                    }
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
                  else
                    {
                      was_added = false;
                    }
#endif

                  if (was_added)
                    {
                      labels << text_label;
                      fields << line_edit;
                    }
                  else
                    {
                      text_label.clear();
                      label->clear();
                      line_edit->close();
                    }
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
                      bool was_added = true, not_in = false;
                      if (childchild->localName() == "xs:attribute")
                        {
                          auto attr_name = static_cast<std::string>(childchild->getAttribute("name"));
                          for (const auto &selection : multiple_selections)
                            {
                              std::shared_ptr<GRM::Element> tmp;
                              auto selection_name = selection->localName();
                              auto selections2 = schema_tree->querySelectorsAll("[name=" + selection_name + "]");
                              for (const auto &s : selections2)
                                {
                                  if (s->localName() == "xs:element") tmp = s->children()[0];
                                }
                              if (tmp->querySelectors("[" + attr_name + "]") == nullptr) not_in = true;
                            }

                          if ((*current_selection)->getRef()->localName() == "coordinate_system" ||
                              ((*current_selection)->getRef()->localName() == "layout_grid" &&
                               (*current_selection)->getRef()->parentElement()->localName() != "layout_grid"))
                            {
                              // special case for coordinate_system cause the ndc movements results in a nonsense
                              // plot
                              if (attr_name == "x_max_shift_ndc" || attr_name == "x_min_shift_ndc" ||
                                  attr_name == "y_max_shift_ndc" || attr_name == "y_min_shift_ndc" ||
                                  attr_name == "x_shift_ndc" || attr_name == "y_shift_ndc")
                                continue;
                            }
                          if (!multiple_selections.empty() && not_in) continue;
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
                                  line_edit->setToolTip(tooltip_string);

                                  if (attr_name == "scientific_format")
                                    {
                                      line_edit->setFixedWidth(120);
#if QT_VERSION >= 0x060000
                                      connect(static_cast<QComboBox *>(line_edit), &QComboBox::currentIndexChanged,
                                              this, [=]() { openTextPreview(); });
#endif
                                    }
                                }
                              else if (check_box_attr.contains(attr_name.c_str()))
                                {
                                  line_edit = new QCheckBox(this);
                                  line_edit->setToolTip(tooltip_string);
                                  static_cast<QCheckBox *>(line_edit)->setChecked(
                                      static_cast<int>((*current_selection)->getRef()->getAttribute(attr_name)) == 1);
                                }
                              else if (color_ind_attr.contains(attr_name.c_str()))
                                {
                                  line_edit = new QPushButton("", this);
                                  line_edit->setToolTip(tooltip_string);
                                  line_edit->setObjectName(attr_name.c_str());
                                  QObject::connect(line_edit, SIGNAL(clicked()), this, SLOT(colorIndexSlot()));
                                  QObject::connect(static_cast<QPushButton *>(line_edit), &QPushButton::clicked, [=]() {
                                    auto new_index =
                                        static_cast<int>((*current_selection)->getRef()->getAttribute(attr_name));
                                    static_cast<QPushButton *>(line_edit)->setText(std::to_string(new_index).c_str());
                                    QImage new_image(1, 1, QImage::Format_RGB32);
                                    QRgb new_value;
                                    int err;
                                    double new_r, new_g, new_b;

                                    gks_inq_color_rep(-1, new_index, -1, &err, &new_r, &new_g, &new_b);
                                    new_value = qRgb(255 * new_r, 255 * new_g, 255 * new_b);
                                    new_image.setPixel(0, 0, new_value);

                                    auto new_color_pic = QPixmap::fromImage(new_image);
                                    new_color_pic = new_color_pic.scaled(20, 20);
                                    static_cast<QPushButton *>(line_edit)->setIcon(QIcon(new_color_pic));
                                  });
                                }
                              else if (color_rgb_attr.contains(attr_name.c_str()))
                                {
                                  line_edit = new QPushButton(this);
                                  line_edit->setToolTip(tooltip_string);
                                  line_edit->setObjectName(attr_name.c_str());
                                  QObject::connect(line_edit, SIGNAL(clicked()), this, SLOT(colorRGBSlot()));
                                  QObject::connect(static_cast<QPushButton *>(line_edit), &QPushButton::clicked, [=]() {
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

                                        static_cast<QPushButton *>(line_edit)->setText(context_ref.c_str());
                                        auto new_color_pic = QPixmap::fromImage(new_image);
                                        new_color_pic = new_color_pic.scaled(20, 20);
                                        static_cast<QPushButton *>(line_edit)->setIcon(QIcon(new_color_pic));
                                      }
                                  });
                                }
                              else
                                {
                                  line_edit = new QLineEdit(this);
                                  line_edit->setToolTip(tooltip_string);
                                  static_cast<QLineEdit *>(line_edit)->setText("");
                                }
                              text_label = QString("<span style='color:#ff0000;'>%1</span>")
                                               .arg(attrNameToLabel(attr_name).c_str());
                              auto label = new QLabel(text_label, this);
                              label->setFixedWidth(LABEL_WIDTH);
                              label->setWordWrap(true);
                              label->setToolTip(tooltip_string);
                              if (attr_name == "line_color_ind" || attr_name == "line_color_rgb" ||
                                  attr_name == "line_spec" || attr_name == "line_type" || attr_name == "line_width")
                                {
                                  if (!line_modification_added)
                                    {
                                      form->addRow(line_modification);
                                      line_modification_added = true;
                                    }
                                  if (line_modification_added) line_modification_form->addRow(label, line_edit);
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

                                  if (marker_modification_added) marker_modification_form->addRow(label, line_edit);
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

#if QT_VERSION >= 0x060000
                                  if (attr_name == "font_precision")
                                    connect(static_cast<QComboBox *>(line_edit), &QComboBox::currentIndexChanged, this,
                                            [=]() { openTextPreview(); });
#endif

                                  if (text_modification_added) text_modification_form->addRow(label, line_edit);
                                }
                              else if (attr_name == "fill_color_ind" || attr_name == "fill_color_rgb" ||
                                       attr_name == "fill_int_style" || attr_name == "fill_style")
                                {
                                  if (!fill_modification_added)
                                    {
                                      form->addRow(fill_modification);
                                      fill_modification_added = true;
                                    }

                                  if (fill_modification_added) fill_modification_form->addRow(label, line_edit);
                                }
                              else if (attr_name == "viewport_x_min" || attr_name == "viewport_x_max" ||
                                       attr_name == "viewport_y_min" || attr_name == "viewport_y_max")
                                {
                                  if (!viewport_added && advanced_editor)
                                    {
                                      form->addRow(viewport);
                                      viewport_added = true;
                                    }
                                  if (!advanced_editor)
                                    {
                                      was_added = false;
                                      if (viewport_added) viewport_form->addRow(label, line_edit);
                                    }
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

                                  auto widget = new QWidget(this);
                                  auto grid_layout = new QGridLayout();
                                  auto slider = new QSlider(this);
                                  slider->setOrientation(Qt::Horizontal);
                                  slider->setRange(0, 100);
                                  slider->setFixedWidth(80);
                                  line_edit->setMaximumWidth(40);
                                  grid_layout->addWidget(line_edit, 0, 0);
                                  grid_layout->addWidget(slider, 0, 1);
                                  grid_layout->setContentsMargins(0, 0, 0, 0);
                                  grid_layout->setSpacing(10);
                                  widget->setLayout(grid_layout);
                                  widget->setContentsMargins(0, 0, 0, 0);
                                  widget->setFixedHeight(30);

                                  connect(slider, &QSlider::sliderMoved, this, [=] {
                                    double val = slider->value();
                                    static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                                    static_cast<QLineEdit *>(line_edit)->setModified(true);
                                  });
                                  connect(slider, &QSlider::sliderPressed, this, [=] {
                                    double val = slider->value();
                                    static_cast<QLineEdit *>(line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                                    static_cast<QLineEdit *>(line_edit)->setModified(true);
                                  });
                                  connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged, this, [=] {
                                    double val = static_cast<QLineEdit *>(line_edit)->text().toDouble();
                                    slider->setValue(val * 100.0);
                                  });
                                  if (!advanced_editor)
                                    {
                                      was_added = false;
                                      if (viewport_normalized_added) viewport_normalized_form->addRow(label, widget);
                                    }
                                }
                              else if (attr_name == "x_range_min" || attr_name == "x_range_max" ||
                                       attr_name == "y_range_min" || attr_name == "y_range_max" ||
                                       attr_name == "z_range_min" || attr_name == "z_range_max" ||
                                       attr_name == "c_range_min" || attr_name == "c_range_max" ||
                                       attr_name == "r_range_min" || attr_name == "r_range_max" ||
                                       attr_name == "theta_range_min" || attr_name == "theta_range_max")
                                {
                                  if (advanced_editor && !range_modification_added)
                                    {
                                      form->addRow(range_modification);
                                      range_modification_added = true;
                                    }

                                  if (range_modification_added)
                                    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                                      range_modification_form->addRow(label, line_edit);
                                      if (!advanced_editor)
                                        range_modification_form->setRowVisible(range_modification_form->rowCount() - 1,
                                                                               false);
#else
                                      if (advanced_editor) range_modification_form->addRow(label, line_edit);
#endif
                                    }
                                  if (!advanced_editor) was_added = false;
                                }
                              else if (attr_name == "x_max_shift_ndc" || attr_name == "x_min_shift_ndc" ||
                                       attr_name == "x_shift_ndc" || attr_name == "x_max_shift_wc" ||
                                       attr_name == "x_min_shift_wc" || attr_name == "x_shift_wc" ||
                                       attr_name == "y_max_shift_ndc" || attr_name == "y_min_shift_ndc" ||
                                       attr_name == "y_shift_ndc" || attr_name == "y_max_shift_wc" ||
                                       attr_name == "y_min_shift_wc" || attr_name == "y_shift_wc")
                                {
                                  bool is_non_advanced_element =
                                      advanced_editor ||
                                      (attr_name != "x_max_shift_wc" && attr_name != "x_min_shift_wc" &&
                                       attr_name != "x_shift_wc" && attr_name != "y_max_shift_wc" &&
                                       attr_name != "y_min_shift_wc" && attr_name != "y_shift_wc") &&
                                          ((*current_selection)->getRef()->hasAttribute("viewport_x_min") ||
                                           (*current_selection)->getRef()->localName() == "text");
                                  if (!element_movement_modification_added &&
                                      (advanced_editor ||
                                       !isAdvancedAttribute((*current_selection)->getRef(), attr_name)))
                                    {
                                      if (is_non_advanced_element)
                                        {
                                          form->addRow(element_movement_modification);
                                          element_movement_modification_added = true;
                                        }
                                    }

                                  if (element_movement_modification_added)
                                    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                                      if (is_non_advanced_element)
                                        {
                                          if (slider_attr.contains(attr_name.c_str()))
                                            {
                                              auto widget = new QWidget(this);
                                              auto grid_layout = new QGridLayout();
                                              auto slider = new QSlider(this);
                                              slider->setOrientation(Qt::Horizontal);
                                              slider->setRange(0, 100);
                                              slider->setFixedWidth(80);
                                              line_edit->setMaximumWidth(40);
                                              grid_layout->addWidget(line_edit, 0, 0);
                                              grid_layout->addWidget(slider, 0, 1);
                                              grid_layout->setContentsMargins(0, 0, 0, 0);
                                              grid_layout->setSpacing(10);
                                              widget->setLayout(grid_layout);
                                              widget->setContentsMargins(0, 0, 0, 0);
                                              widget->setFixedHeight(30);

                                              connect(slider, &QSlider::sliderMoved, this, [=] {
                                                double val = slider->value();
                                                static_cast<QLineEdit *>(line_edit)->setText(
                                                    QString::number(val / 100.0, 'f', 2));
                                                static_cast<QLineEdit *>(line_edit)->setModified(true);
                                              });
                                              connect(slider, &QSlider::sliderPressed, this, [=] {
                                                double val = slider->value();
                                                static_cast<QLineEdit *>(line_edit)->setText(
                                                    QString::number(val / 100.0, 'f', 2));
                                                static_cast<QLineEdit *>(line_edit)->setModified(true);
                                              });
                                              connect(static_cast<QLineEdit *>(line_edit), &QLineEdit::textChanged,
                                                      this, [=] {
                                                        double val =
                                                            static_cast<QLineEdit *>(line_edit)->text().toDouble();
                                                        slider->setValue(val * 100.0);
                                                      });
                                              element_movement_modification_form->addRow(label, widget);
                                            }
                                          else
                                            {
                                              element_movement_modification_form->addRow(label, line_edit);
                                            }
                                        }
                                      if (!advanced_editor && is_non_advanced_element)
                                        element_movement_modification_form->setRowVisible(
                                            element_movement_modification_form->rowCount() - 1,
                                            !isAdvancedAttribute((*current_selection)->getRef(), attr_name));
#else
                                      if (is_non_advanced_element)
                                        {
                                          if (slider_attr.contains(attr_name.c_str()))
                                            {
                                              auto widget = new QWidget(this);
                                              auto grid_layout = new QGridLayout();
                                              auto slider = new QSlider(this);
                                              slider->setOrientation(Qt::Horizontal);
                                              slider->setRange(0, 100);
                                              slider->setFixedWidth(80);
                                              line_edit->setMaximumWidth(40);
                                              grid_layout->addWidget(line_edit, 0, 0);
                                              grid_layout->addWidget(slider, 0, 1);
                                              grid_layout->setContentsMargins(0, 0, 0, 0);
                                              grid_layout->setSpacing(10);
                                              widget->setLayout(grid_layout);
                                              widget->setContentsMargins(0, 0, 0, 0);
                                              widget->setFixedHeight(30);

                                              connect(slider, &QSlider::sliderMoved, this, [=] {
                                                double val = slider->value();
                                                ((QLineEdit *)line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                                                ((QLineEdit *)line_edit)->setModified(true);
                                              });
                                              connect(slider, &QSlider::sliderPressed, this, [=] {
                                                double val = slider->value();
                                                ((QLineEdit *)line_edit)->setText(QString::number(val / 100.0, 'f', 2));
                                                ((QLineEdit *)line_edit)->setModified(true);
                                              });
                                              connect(((QLineEdit *)line_edit), &QLineEdit::textChanged, this, [=] {
                                                double val = ((QLineEdit *)line_edit)->text().toDouble();
                                                slider->setValue(val * 100.0);
                                              });
                                              element_movement_modification_form->addRow(label, widget);
                                            }
                                          else
                                            {
                                              element_movement_modification_form->addRow(label, line_edit);
                                            }
                                        }
#endif
                                    }
                                  if ((!advanced_editor &&
                                       isAdvancedAttribute((*current_selection)->getRef(), attr_name)) ||
                                      !is_non_advanced_element)
                                    was_added = false;
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
                                  if (!advanced_editor &&
                                      isAdvancedAttribute((*current_selection)->getRef(), attr_name))
                                    was_added = false;
                                }

                              if (was_added)
                                {
                                  labels << text_label;
                                  fields << line_edit;
                                }
                              else
                                {
                                  text_label.clear();
                                  label->clear();
                                  line_edit->close();
                                }
                            }
                        }
                    }
                }
              else if (advanced_editor)
                {
                  /* special case for colorrep cause there are way to many attributes inside the attributegroup
                   */
                  line_edit = new QLineEdit(this);
                  static_cast<QLineEdit *>(line_edit)->setText("");
                  text_label = QString("<span style='color:#ff0000;'>%1</span>").arg("Colorrep-index");
                  form->addRow(text_label, line_edit);

                  attr_type.emplace("Colorrep-index", "xs:string");
                  labels << text_label;
                  fields << line_edit;

                  line_edit = new QLineEdit(this);
                  static_cast<QLineEdit *>(line_edit)->setText("");
                  text_label = QString("<span style='color:#ff0000;'>%1</span>").arg("Colorrep-value");
                  form->addRow(text_label, line_edit);

                  attr_type.emplace("Colorrep-value", "xs:string");
                  labels << text_label;
                  fields << line_edit;
                }
            }
        }
    }

  if (line_modification_added)
    {
      line_modification_layout->addLayout(line_modification_form);
      line_modification->setContentLayout(*line_modification_layout);
    }
  else
    {
      clearLayout(line_modification_form);
      clearLayout(line_modification_layout);
      line_modification->close();
    }
  if (marker_modification_added)
    {
      marker_modification_layout->addLayout(marker_modification_form);
      marker_modification->setContentLayout(*marker_modification_layout);
    }
  else
    {
      clearLayout(marker_modification_form);
      clearLayout(marker_modification_layout);
      marker_modification->close();
    }
  if (text_modification_added)
    {
      text_modification_layout->addLayout(text_modification_form);
      text_modification->setContentLayout(*text_modification_layout);
    }
  else
    {
      clearLayout(text_modification_form);
      clearLayout(text_modification_layout);
      text_modification->close();
    }
  if (fill_modification_added)
    {
      fill_modification_layout->addLayout(fill_modification_form);
      fill_modification->setContentLayout(*fill_modification_layout);
    }
  else
    {
      clearLayout(fill_modification_form);
      clearLayout(fill_modification_layout);
      fill_modification->close();
    }
  if (viewport_added)
    {
      viewport_layout->addLayout(viewport_form);
      viewport->setContentLayout(*viewport_layout);
    }
  else
    {
      clearLayout(viewport_form);
      clearLayout(viewport_layout);
      viewport->close();
    }
  if (viewport_normalized_added)
    {
      viewport_normalized_layout->addLayout(viewport_normalized_form);
      viewport_normalized->setContentLayout(*viewport_normalized_layout);
    }
  else
    {
      clearLayout(viewport_normalized_form);
      clearLayout(viewport_normalized_layout);
      viewport_normalized->close();
    }
  if (window_added)
    {
      window_layout->addLayout(window_form);
      window->setContentLayout(*window_layout);
    }
  else
    {
      clearLayout(window_form);
      clearLayout(window_layout);
      window->close();
    }
  if (range_modification_added)
    {
      range_modification_layout->addLayout(range_modification_form);
      range_modification->setContentLayout(*range_modification_layout);
    }
  else
    {
      clearLayout(range_modification_form);
      clearLayout(range_modification_layout);
      range_modification->close();
    }
  if (log_modification_added)
    {
      log_modification_layout->addLayout(log_modification_form);
      log_modification->setContentLayout(*log_modification_layout);
    }
  else
    {
      clearLayout(log_modification_form);
      clearLayout(log_modification_layout);
      log_modification->close();
    }
  if (flip_modification_added)
    {
      flip_modification_layout->addLayout(flip_modification_form);
      flip_modification->setContentLayout(*flip_modification_layout);
    }
  else
    {
      clearLayout(flip_modification_form);
      clearLayout(flip_modification_layout);
      flip_modification->close();
    }
  if (lim_modification_added)
    {
      lim_modification_layout->addLayout(lim_modification_form);
      lim_modification->setContentLayout(*lim_modification_layout);
    }
  else
    {
      clearLayout(lim_modification_form);
      clearLayout(lim_modification_layout);
      lim_modification->close();
    }
  if (element_movement_modification_added)
    {
      element_movement_modification_layout->addLayout(element_movement_modification_form);
      element_movement_modification->setContentLayout(*element_movement_modification_layout);
    }
  else
    {
      clearLayout(element_movement_modification_form);
      clearLayout(element_movement_modification_layout);
      element_movement_modification->close();
    }
  if (space_modification_added)
    {
      space_modification_layout->addLayout(space_modification_form);
      space_modification->setContentLayout(*space_modification_layout);
    }
  else
    {
      clearLayout(space_modification_form);
      clearLayout(space_modification_layout);
      space_modification->close();
    }
  if (ws_modification_added)
    {
      ws_modification_layout->addLayout(ws_modification_form);
      ws_modification->setContentLayout(*ws_modification_layout);
    }
  else
    {
      clearLayout(ws_modification_form);
      clearLayout(ws_modification_layout);
      ws_modification->close();
    }
  if (tick_modification_added)
    {
      tick_modification_layout->addLayout(tick_modification_form);
      tick_modification->setContentLayout(*tick_modification_layout);
    }
  else
    {
      clearLayout(tick_modification_form);
      clearLayout(tick_modification_layout);
      tick_modification->close();
    }
  if (tick_label_modification_added)
    {
      tick_label_modification_layout->addLayout(tick_label_modification_form);
      tick_label_modification->setContentLayout(*tick_label_modification_layout);
    }
  else
    {
      clearLayout(tick_label_modification_form);
      clearLayout(tick_label_modification_layout);
      tick_label_modification->close();
    }
  if (inherit_from_plot_added)
    {
      inherit_from_plot_modification_layout->addLayout(inherit_from_plot_modification_form);
      inherit_from_plot_modification->setContentLayout(*inherit_from_plot_modification_layout, true);
    }
  else
    {
      clearLayout(inherit_from_plot_modification_form);
      clearLayout(inherit_from_plot_modification_layout);
      inherit_from_plot_modification->close();
    }
  if (absolute_errors_added)
    {
      absolute_errors_layout->addLayout(absolute_errors_form);
      absolute_errors->setContentLayout(*absolute_errors_layout);
    }
  else
    {
      clearLayout(absolute_errors_form);
      clearLayout(absolute_errors_layout);
      absolute_errors->close();
    }
  if (relative_errors_added)
    {
      relative_errors_layout->addLayout(relative_errors_form);
      relative_errors->setContentLayout(*relative_errors_layout);
    }
  else
    {
      clearLayout(relative_errors_form);
      clearLayout(relative_errors_layout);
      relative_errors->close();
    }
  if (light_added)
    {
      light_layout->addLayout(light_form);
      light->setContentLayout(*light_layout);
    }
  else
    {
      clearLayout(light_form);
      clearLayout(light_layout);
      light->close();
    }

  QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
  QObject::connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));

  auto scroll_area_content = new QWidget;
  scroll_area_content->setLayout(form);

  auto scroll_area = new QScrollArea;
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll_area->setWidgetResizable(true);
  scroll_area->setWidget(scroll_area_content);

  auto group_box_layout = new QVBoxLayout;
  group_box_layout->addWidget(scroll_area);
  group_box_layout->addWidget(button_box);
  group_box_layout->setContentsMargins(2, 2, 2, 2);
  this->setLayout(group_box_layout);
}

void EditElementWidget::reject()
{
  grplot_widget->setTreeUpdate(false);
  grplot_widget->getHideTextPreviewAct()->trigger();
  fields.clear();
  labels.clear();
  attr_type.clear();
  this->multiple_selections.clear();
  this->close();
}

bool EditElementWidget::setAttributesDuringAccept(std::shared_ptr<GRM::Element> current_selection)
{
  auto current_selection_saved = current_selection;
  bool highlight_location = false;
  for (int i = 0; i < labels.count(); i++)
    {
      current_selection = current_selection_saved;
      auto &field = *fields[i]; // because typeid(*fields[i]) is bad :(
      if ((util::startsWith(labels[i].toStdString(), "<span style='color:#ff0000;'>") ||
           util::startsWith(labels[i].toStdString(), "<span style='color:#0000ff;'>")) &&
          util::endsWith(labels[i].toStdString(), "</span>"))
        {
          if (util::startsWith(labels[i].toStdString(), "<span style='color:#0000ff;'>")) highlight_location = true;
          labels[i].remove(0, 29);
          labels[i].remove(labels[i].size() - 7, 7);
        }
      auto attr_name = labelToAttrName(labels[i].toStdString());
      if ((current_selection->localName() == "axis" || current_selection->localName() == "coordinate_system") &&
          (attr_name == "x_log" || attr_name == "y_log" || attr_name == "z_log" || attr_name == "r_log" ||
           attr_name == "adjust_x_lim" || attr_name == "adjust_y_lim" || attr_name == "adjust_z_lim" ||
           attr_name == "x_lim_min" || attr_name == "x_lim_max" || attr_name == "y_lim_min" ||
           attr_name == "y_lim_max" || attr_name == "z_lim_min" || attr_name == "z_lim_max" ||
           attr_name == "r_lim_min" || attr_name == "r_lim_max" || attr_name == "theta_lim_min" ||
           attr_name == "theta_lim_max" || attr_name == "x_flip" || attr_name == "y_flip" || attr_name == "z_flip" ||
           attr_name == "r_flip" || attr_name == "theta_flip"))
        {
          if (current_selection->localName() == "axis")
            {
              bool pass = false;
              auto location = static_cast<std::string>(current_selection->getAttribute("location"));
              if (location != "x" && location != "y")
                {
                  if (attr_name == "x_lim_min" || attr_name == "x_lim_max" || attr_name == "y_lim_min" ||
                      attr_name == "y_lim_max" || attr_name == "adjust_x_lim" || attr_name == "adjust_y_lim")
                    pass = true;
                }
              if (!pass)
                {
                  if (current_selection->parentElement()->localName() == "coordinate_system")
                    current_selection = current_selection->parentElement()->parentElement()->parentElement();
                  else
                    current_selection = current_selection->parentElement()->parentElement()->parentElement();
                }
            }
          else
            {
              current_selection = current_selection->parentElement()->parentElement();
            }
        }
      if (typeid(field) == typeid(QLineEdit))
        {
          if (static_cast<QLineEdit *>(fields[i])->isModified())
            {
              if (labels[i].toStdString() == "text angle")
                {
                  bool update;
                  auto render = grm_get_render();
                  auto val = static_cast<QLineEdit *>(fields[i])->text().toDouble();
                  render->getAutoUpdate(&update);
                  render->setAutoUpdate(false);
                  current_selection->setAttribute("char_up_x", std::cos(val * M_PI / 180.0));
                  render->setAutoUpdate(update);
                  current_selection->setAttribute("char_up_y", std::sin(val * M_PI / 180.0));
                }
              else
                {
                  auto name = static_cast<std::string>(current_selection->getAttribute("name"));
                  if (static_cast<QLineEdit *>(fields[i])->text().toStdString().empty() && attr_name != "tick_label" &&
                      attr_name != "text")
                    {
                      /* remove attributes from tree when the value got removed */
                      current_selection->removeAttribute(attr_name);
                      if (current_selection->hasAttribute("_" + attr_name + "_set_by_user"))
                        current_selection->removeAttribute("_" + attr_name + "_set_by_user");
                    }
                  else
                    {
                      if (attr_name == "text")
                        {
                          const auto value = static_cast<QLineEdit *>(fields[i])->text().toStdString();
                          if ((attr_type[attr_name] == "xs:string" || attr_type[attr_name] == "strint") &&
                              !util::isDigits(value))
                            {
                              if (current_selection->parentElement()->localName() == "text_region")
                                {
                                  current_selection->parentElement()->parentElement()->setAttribute("text_content",
                                                                                                    value);
                                }
                              else if (name == "xlabel" || name == "ylabel")
                                {
                                  current_selection->parentElement()
                                      ->parentElement()
                                      ->querySelectors(name)
                                      ->setAttribute(name, value);
                                }
                            }
                          else if (attr_type[attr_name] == "xs:double" && util::isNumber(value))
                            {
                              current_selection->parentElement()->setAttribute(attr_name, std::stod(value));
                            }
                          else if ((attr_type[attr_name] == "xs:integer" || attr_type[attr_name] == "strint") &&
                                   util::isDigits(value))
                            {
                              current_selection->parentElement()->setAttribute(attr_name, std::stoi(value));
                            }
                        }
                      if (attr_name == "Colorrep-index")
                        {
                          /* special case for colorrep attribute */
                          current_selection->setAttribute(
                              "colorrep." + static_cast<QLineEdit *>(fields[i])->text().toStdString(),
                              static_cast<QLineEdit *>(fields[i + 1])->text().toStdString());
                        }
                      else if (attr_name != "Colorrep-value")
                        {
                          const auto value = static_cast<QLineEdit *>(fields[i])->text().toStdString();
                          if ((attr_type[attr_name] == "xs:string" || attr_type[attr_name] == "strint") &&
                              !util::isDigits(value))
                            {
                              current_selection->setAttribute(attr_name, value);
                            }
                          else if (attr_type[attr_name] == "xs:string" &&
                                   (attr_name == "arc_label" || attr_name == "angle_label" || attr_name == "x_label" ||
                                    attr_name == "y_label" || attr_name == "z_label" || attr_name == "tick_label" ||
                                    attr_name == "text" || attr_name == "x_label_3d" || attr_name == "y_label_3d" ||
                                    attr_name == "z_label_3d"))
                            {
                              current_selection->setAttribute(attr_name, value);
                            }
                          else if (attr_type[attr_name] == "xs:double" && util::isNumber(value))
                            {
                              current_selection->setAttribute(attr_name, std::stod(value));
                            }
                          else if ((attr_type[attr_name] == "xs:integer" || attr_type[attr_name] == "strint") &&
                                   util::isDigits(value))
                            {
                              current_selection->setAttribute(attr_name, std::stoi(value));
                            }
                          else
                            {
                              fprintf(stderr, "Invalid value %s for attribute %s with type %s\n", value.c_str(),
                                      attr_name.c_str(), attr_type[attr_name].c_str());
                            }
                        }
                    }
                }
            }
        }
      else if (typeid(field) == typeid(QComboBox))
        {
          int index = static_cast<QComboBox *>(fields[i])->currentIndex();
          if (static_cast<QComboBox *>(fields[i])->itemText(index).toStdString().empty())
            {
              /* remove attributes from tree when the value got removed */
              current_selection->removeAttribute(attr_name);
              if (current_selection->hasAttribute("_" + attr_name + "_set_by_user"))
                current_selection->removeAttribute("_" + attr_name + "_set_by_user");
            }
          else
            {
              if (attr_name == "location" && current_selection->localName() == "axis")
                current_selection->setAttribute("_ignore_next_tick_orientation", true);
              const auto value = static_cast<QComboBox *>(fields[i])->itemText(index).toStdString();
              grplot_widget->attributeSetForComboBox(attr_type[attr_name], current_selection, value, attr_name);
            }

          if (attr_name == "colormap")
            {
              const auto value = static_cast<QComboBox *>(fields[i])->itemText(index).toStdString();
              auto colormap = QPixmap((":/preview_images/colormaps/" + value + ".png").c_str());
              colormap = colormap.scaled(20, 20);
              grplot_widget->getColormapAct()->setIcon(colormap);
            }
        }
      else if (typeid(field) == typeid(QCheckBox))
        {
          current_selection->setAttribute(attr_name, static_cast<QCheckBox *>(fields[i])->isChecked());
        }
      else if (typeid(field) == typeid(QDial))
        {
          auto val = static_cast<QDial *>(fields[i])->value();
          current_selection->setAttribute(attr_name, val);
        }
    }
  return highlight_location;
}

void EditElementWidget::accept()
{
  bool highlight_location = false;
  if (multiple_selections.empty())
    {
      auto current_selection = grplot_widget->getCurrentSelection();
      grplot_widget->createHistoryElement();
      highlight_location = setAttributesDuringAccept((*current_selection)->getRef());
    }
  else
    {
      for (const auto &selection : multiple_selections)
        {
          grplot_widget->createHistoryElement();
          highlight_location = setAttributesDuringAccept(selection);
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
  grplot_widget->editElementAccepted(highlight_location);
  fields.clear();
  labels.clear();
  attr_type.clear();
  this->multiple_selections.clear();
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
  openTextPreview();
}

void EditElementWidget::colorRGBSlot()
{
  auto sender_ref = sender();
  std::string attribute_name = sender_ref->objectName().toStdString();
  auto current_selection = grplot_widget->getCurrentSelection();
  grplot_widget->colorRGBPopUp(attribute_name, (*current_selection)->getRef());
}

bool EditElementWidget::isAdvancedAttribute(const std::shared_ptr<GRM::Element> &element, std::string attr_name,
                                            bool inherit_elem)
{
  // hide all set and non set attributes which allow a graphical modification of the figure if the advanced editor isn't
  // turned on
  auto elem_name = element->localName();
  static std::unordered_map<std::string, std::vector<std::string>> element_to_advanced_attributes{
      {std::string("figure"),
       std::vector<std::string>{
           "active",
       }},
      {std::string("layout_grid"),
       std::vector<std::string>{
           "aspect_ratio",
           "fit_parents_height",
           "fit_parents_width",
           "start_col",
           "start_row",
           "stop_col",
           "stop_row",
           "viewport_height_abs",
           "viewport_height_rel",
           "viewport_width_abs",
           "viewport_width_rel",
       }},
      {std::string("layout_grid_element"),
       std::vector<std::string>{
           "aspect_ratio",
           "fit_parents_height",
           "fit_parents_width",
           "viewport_height_abs",
           "viewport_height_rel",
           "viewport_width_abs",
           "viewport_width_rel",
       }},
      {std::string("draw_graphics"),
       std::vector<std::string>{
           "name",
       }},
      {std::string("plot"),
       std::vector<std::string>{
           "c_lim_max",
           "c_lim_min",
           "location",
           "max_y_length",
           "plot_group",
           "scale",
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
           "location",
           "tick",
       }},
      {std::string("side_region"),
       std::vector<std::string>{
           "marginal_heatmap_side_plot",
           "width",
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
           "char_height",
           "plot_type", // Todo: readd plot_type if it can be used to transform a 2d plot in a 3d plot f.e.
       }},
      {std::string("grid_3d"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("axis"),
       std::vector<std::string>{
           "axis_type",
           "max_value",
           "min_value",
           "name",
           "x_max_shift_ndc",
           "x_min_shift_ndc",
           "x_shift_ndc",
           "y_max_shift_ndc",
           "y_min_shift_ndc",
           "y_shift_ndc",
           "z_index",
       }},
      {std::string("tick_group"),
       std::vector<std::string>{
           "x_max_shift_ndc",
           "x_min_shift_ndc",
           "x_shift_ndc",
           "y_max_shift_ndc",
           "y_min_shift_ndc",
           "y_shift_ndc",
           "z_index",
       }},
      {std::string("tick"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("grid_line"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("axes_3d"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("text"),
       std::vector<std::string>{
           "height",
           "name",
           "set_text_color_for_background",
           "world_coordinates",
           "width",
           "x_max_shift_ndc",
           "x_min_shift_ndc",
           "y_max_shift_ndc",
           "y_min_shift_ndc",
           "z_index",
       }},
      {std::string("titles_3d"),
       std::vector<std::string>{
           "z_index",
       }},
      {std::string("legend"),
       std::vector<std::string>{
           "scale",
           "select_specific_xform",
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
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("arc_grid_line"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("theta_axes"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("angle_line"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "text_x0",
           "text_y0",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("error_bars"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("polyline_3d"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
           "z",
       }},
      {std::string("polyline"),
       std::vector<std::string>{
           "x",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("polymarker"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "name",
           "x",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
           "z_index",
       }},
      {std::string("polymarker_3d"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y",
           "y_max_shift_wc",
           "y_min_shift_wc",
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
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y",
           "y_max",
           "y_min",
           "y_max_shift_wc",
           "y_min_shift_wc",
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
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y",
           "y_max",
           "y_min",
           "y_max_shift_wc",
           "y_min_shift_wc",
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
           "name",
           "num_col",
           "num_row",
           "select_specific_xform",
           "start_col",
           "start_row",
           "x_dim",
           "x_max",
           "x_min",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_dim",
           "y_max",
           "y_min",
           "y_max_shift_wc",
           "y_min_shift_wc",
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
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y",
           "y_dim",
           "y_max_shift_wc",
           "y_min_shift_wc",
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
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
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
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
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
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max",
           "y_min",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("draw_arc"),
       std::vector<std::string>{"disable_x_trans", "disable_y_trans", "end_angle", "movable", "start_angle", "x_max",
                                "x_min", "x_max_shift_wc", "x_min_shift_wc", "x_shift_wc", "y_max", "y_min",
                                "y_max_shift_wc", "y_min_shift_wc", "y_shift_wc", "z_index"}},
      {std::string("fill_arc"),
       std::vector<std::string>{"disable_x_trans", "disable_y_trans", "end_angle", "movable", "start_angle", "x_max",
                                "x_min", "x_max_shift_wc", "x_min_shift_wc", "x_shift_wc", "y_max", "y_min",
                                "y_max_shift_wc", "y_min_shift_wc", "y_shift_wc", "z_index"}},
      {std::string("fill_area"), std::vector<std::string>{"disable_x_trans", "disable_y_trans", "movable", "name", "x",
                                                          "x_max_shift_wc", "x_min_shift_wc", "x_shift_wc", "y",
                                                          "y_max_shift_wc", "y_min_shift_wc", "y_shift_wc", "z_index"}},
      {std::string("series_barplot"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "label",
           "max_y_length",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
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
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
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
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_heatmap"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_hexbin"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_histogram"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_range_max",
           "y_range_min",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_imshow"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_isosurface"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_line"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_nonuniform_heatmap"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_nonuniform_polar_heatmap"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_pie"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_range_max",
           "y_range_min",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_line3"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_polar_heatmap"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_polar_histogram"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_polar_line"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_polar_scatter"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_quiver"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_scatter"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_scatter3"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_shade"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
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
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_shade"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_stem"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_surface"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_tricontour"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_trisurface"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_volume"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("series_wireframe"),
       std::vector<std::string>{
           "c_range_max",
           "c_range_min",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "ref_x_axis_location",
           "ref_y_axis_location",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("error_bar"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "downwards_e",
           "movable",
           "upwards_e",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("bar"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "x1",
           "x2",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
           "y1",
           "y2",
       }},
      {std::string("integral"),
       std::vector<std::string>{
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("integral_group"),
       std::vector<std::string>{
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("polar_bar"),
       std::vector<std::string>{
           "bin_edges",
           "bin_nr",
           "bin_width",
           "bin_widths",
           "count",
           "disable_x_trans",
           "disable_y_trans",
           "movable",
           "norm",
           "theta_flip",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("pie_segment"),
       std::vector<std::string>{
           "disable_x_trans",
           "disable_y_trans",
           "end_angle",
           "movable",
           "start_angle",
           "x_max_shift_wc",
           "x_min_shift_wc",
           "x_shift_wc",
           "y_max_shift_wc",
           "y_min_shift_wc",
           "y_shift_wc",
       }},
      {std::string("overlay"), std::vector<std::string>{}},
      {std::string("overlay_element"), std::vector<std::string>{"element_type", "data", "window_x_min", "window_x_max",
                                                                "window_y_min", "window_y_max"}},
  };

  if (element_to_advanced_attributes.count(elem_name))
    if (std::find(element_to_advanced_attributes[elem_name].begin(), element_to_advanced_attributes[elem_name].end(),
                  attr_name) != element_to_advanced_attributes[elem_name].end())
      return true;
  // hide x and y for polar_types - theta and r for 2d_types - z for non 3d_types
  if (element->localName() == "plot")
    {
      if (auto coordinate_system = element->querySelectors("coordinate_system"); coordinate_system != nullptr)
        {
          if (auto plot_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
              plot_type == "2d")
            {
              if (attr_name == "adjust_z_lim" || attr_name == "r_lim_max" || attr_name == "r_lim_min" ||
                  attr_name == "r_log" || attr_name == "theta_flip" || attr_name == "theta_lim_max" ||
                  attr_name == "theta_lim_min" || attr_name == "z_log" || attr_name == "z_flip" ||
                  attr_name == "z_lim_max" || attr_name == "z_lim_min" || attr_name == "polar_with_pan" ||
                  attr_name == "keep_radii_axes")
                return true;
            }
          else if (plot_type == "3d")
            {
              if (attr_name == "r_lim_max" || attr_name == "r_lim_min" || attr_name == "r_log" ||
                  attr_name == "theta_flip" || attr_name == "theta_lim_max" || attr_name == "theta_lim_min" ||
                  attr_name == "polar_with_pan" || attr_name == "keep_radii_axes" ||
                  attr_name == "only_square_aspect_ratio")
                return true;
            }
          else if (plot_type == "polar")
            {
              if (attr_name == "adjust_x_lim" || attr_name == "adjust_y_lim" || attr_name == "adjust_z_lim" ||
                  attr_name == "c_lim_max" || attr_name == "c_lim_min" || attr_name == "x_lim_max" ||
                  attr_name == "x_lim_min" || attr_name == "x_log" || attr_name == "y_log" ||
                  attr_name == "y_lim_max" || attr_name == "y_lim_min" || attr_name == "z_flip" ||
                  attr_name == "z_lim_max" || attr_name == "z_lim_min" || attr_name == "z_log" ||
                  attr_name == "only_square_aspect_ratio")
                return true;
            }
        }
    }
  else if (element->localName() == "central_region")
    {
      if (auto coordinate_system = element->querySelectors("coordinate_system"); coordinate_system != nullptr)
        {
          if (auto plot_type = static_cast<std::string>(coordinate_system->getAttribute("plot_type"));
              plot_type == "2d")
            {
              if (attr_name == "r_min" || attr_name == "r_max" || attr_name == "space_3d_camera_distance" ||
                  attr_name == "space_3d_fov" || attr_name == "space_3d_theta" || attr_name == "space_3d_phi" ||
                  attr_name == "window_z_max" || attr_name == "window_z_min" || attr_name == "clip_region" ||
                  attr_name == "select_specific_xform")
                return true;
            }
          else if (plot_type == "3d")
            {
              if (attr_name == "r_min" || attr_name == "r_max" || attr_name == "space_rotation" ||
                  attr_name == "space_tilt" || attr_name == "space_z_max" || attr_name == "space_z_min" ||
                  attr_name == "clip_region" || attr_name == "select_specific_xform")
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
          if (auto plot_type = static_cast<std::string>(element->getAttribute("plot_type")); plot_type == "2d")
            {
              if (attr_name == "theta_flip" || attr_name == "x_grid" || attr_name == "y_grid" ||
                  attr_name == "z_grid" || attr_name == "z_label" || attr_name == "angle_ticks" ||
                  attr_name == "x_label" || attr_name == "y_label" || attr_name == "z_log" || attr_name == "r_log" ||
                  attr_name == "adjust_z_lim" || attr_name == "z_lim_min" || attr_name == "z_lim_max" ||
                  attr_name == "z_flip" || attr_name == "theta_lim_min" || attr_name == "theta_lim_max" ||
                  attr_name == "r_lim_min" || attr_name == "r_lim_max")
                return true;
            }
          else if (plot_type == "3d")
            {
              if (attr_name == "theta_flip" || attr_name == "angle_ticks" || attr_name == "r_log" ||
                  attr_name == "theta_lim_min" || attr_name == "theta_lim_max" || attr_name == "r_lim_min" ||
                  attr_name == "r_lim_max")
                return true;
            }
          else if (plot_type == "polar")
            {
              if (attr_name == "x_grid" || attr_name == "x_label" || attr_name == "y_grid" || attr_name == "y_label" ||
                  attr_name == "y_line" || attr_name == "z_grid" || attr_name == "z_label" || attr_name == "x_label" ||
                  attr_name == "y_label" || attr_name == "x_log" || attr_name == "y_log" || attr_name == "z_log" ||
                  attr_name == "adjust_x_lim" || attr_name == "adjust_y_lim" || attr_name == "adjust_z_lim" ||
                  attr_name == "x_lim_min" || attr_name == "x_lim_max" || attr_name == "y_lim_min" ||
                  attr_name == "y_lim_max" || attr_name == "z_lim_min" || attr_name == "z_lim_max")
                return true;
            }
        }
    }
  else if (element->localName() == "axis")
    {
      auto axis_type = static_cast<std::string>(element->getAttribute("axis_type"));
      if (axis_type == "x")
        {
          if (attr_name == "y_tick" || attr_name == "y_major" || attr_name == "y_log" || attr_name == "adjust_y_lim" ||
              attr_name == "y_lim_min" || attr_name == "y_lim_max")
            return true;
        }
      else if (axis_type == "y")
        {
          if (attr_name == "x_tick" || attr_name == "x_major" || attr_name == "x_log" || attr_name == "adjust_x_lim" ||
              attr_name == "x_lim_min" || attr_name == "x_lim_max")
            return true;
        }
      if (attr_name == "z_log" || attr_name == "r_log" || attr_name == "x_flip" || attr_name == "y_flip" ||
          attr_name == "z_flip" || attr_name == "theta_flip" || attr_name == "theta_lim_min" ||
          attr_name == "theta_lim_max" || attr_name == "r_lim_min" || attr_name == "r_lim_max" ||
          attr_name == "adjust_z_lim" || attr_name == "z_lim_min" || attr_name == "z_lim_max")
        return true;
      if (auto location = static_cast<std::string>(element->getAttribute("location"));
          location == "x" || location == "y")
        {
          if (attr_name == "window_x_min" || attr_name == "window_x_max" || attr_name == "window_y_min" ||
              attr_name == "window_y_max")
            return true;
          if ((attr_name == "x_lim_min" || attr_name == "x_lim_max" || attr_name == "adjust_x_lim" ||
               attr_name == "y_lim_min" || attr_name == "y_lim_max" || attr_name == "adjust_y_lim") &&
              !inherit_elem)
            return true;
        }
      else
        {
          if (inherit_elem)
            {
              if (axis_type == "x" &&
                  (attr_name == "x_lim_min" || attr_name == "x_lim_max" || attr_name == "adjust_x_lim"))
                return true;
              if (axis_type == "y" &&
                  (attr_name == "y_lim_min" || attr_name == "y_lim_max" || attr_name == "adjust_y_lim"))
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

void EditElementWidget::openTextPreview()
{
  if (auto current_selection = grplot_widget->getCurrentSelection(); current_selection != nullptr)
    {
      std::string text;
      int text_color = 1, scientific_format = 1, font_precision = 3;

      for (const auto &attr : {"text", "x_label_3d", "y_label_3d", "z_label_3d", "tick_label"})
        {
          if ((*current_selection)->getRef()->hasAttribute(attr))
            {
              text = static_cast<std::string>((*current_selection)->getRef()->getAttribute(attr));
              break;
            }
        }
      if ((*current_selection)->getRef()->hasAttribute("text_color_ind"))
        text_color = static_cast<int>((*current_selection)->getRef()->getAttribute("text_color_ind"));

      if ((*current_selection)->getRef()->hasAttribute("scientific_format"))
        scientific_format = static_cast<int>((*current_selection)->getRef()->getAttribute("scientific_format"));

      if ((*current_selection)->getRef()->hasAttribute("font_precision"))
        font_precision = static_cast<int>((*current_selection)->getRef()->getAttribute("font_precision"));

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
          auto attr_name = labelToAttrName(labels[i].toStdString());

          if (typeid(field) == typeid(QLineEdit) && static_cast<QLineEdit *>(fields[i])->isModified())
            {
              if (attr_name == "tick_label" || attr_name == "text")
                text = static_cast<QLineEdit *>(fields[i])->text().toStdString();
              else if (attr_name == "text_color_ind")
                text_color = static_cast<QLineEdit *>(fields[i])->text().toInt();
            }
          else if (typeid(field) == typeid(QComboBox))
            {
              int index = static_cast<QComboBox *>(fields[i])->currentIndex();
              if (attr_name == "scientific_format")
                {
                  if (static_cast<QComboBox *>(fields[i])->itemText(index).toStdString().empty())
                    scientific_format = 1;
                  else
                    scientific_format = GRM::scientificFormatStringToInt(
                        static_cast<QComboBox *>(fields[i])->itemText(index).toStdString());
                }
            }
        }

      auto bbox_x_min = static_cast<int>((*current_selection)->getRef()->getAttribute("_bbox_x_min"));
      auto bbox_x_max = static_cast<int>((*current_selection)->getRef()->getAttribute("_bbox_x_max"));
      auto bbox_y_min = static_cast<int>((*current_selection)->getRef()->getAttribute("_bbox_y_min"));
      auto bbox_y_max = static_cast<int>((*current_selection)->getRef()->getAttribute("_bbox_y_max"));
      auto width = bbox_x_max - bbox_x_min, height = bbox_y_max - bbox_y_min;

      if ((*current_selection)->getRef()->hasAttribute("char_up_x") ||
          (*current_selection)->getRef()->hasAttribute("char_up_y"))
        {
          auto char_up_x = static_cast<int>((*current_selection)->getRef()->getAttribute("char_up_x"));
          auto char_up_y = static_cast<int>((*current_selection)->getRef()->getAttribute("char_up_y"));

          if (abs(char_up_x) > abs(char_up_y))
            {
              auto tmp = width;
              width = height;
              height = tmp;
            }
        }
      grplot_widget->setUpPreviewTextWidget(text, scientific_format, text_color, font_precision, width, height);
    }
}
