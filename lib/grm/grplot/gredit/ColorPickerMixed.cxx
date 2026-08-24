#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when using `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#include "ColorPickerMixed.hxx"

#include <gks.h>
#include <gr.h>

#include <QDialog>
#include <QLabel>

static std::weak_ptr<GRM::Element> elem;
static std::string attr_name;

void ColorPickerMixed::resetLastSlider()
{
  if (last_used_slider != nullptr)
    {
      std::string pattern = "background: #000000";
      auto style_sheet = last_used_slider->styleSheet().toStdString();
      auto pos = style_sheet.find(pattern);
      if (pos != std::string::npos)
        {
          style_sheet.replace(pos, pattern.length(), "background: transparent");
          last_used_slider->setStyleSheet(style_sheet.c_str());
        }
      last_used_slider = nullptr;
    }
}

void ColorPickerMixed::colorIndexHelper(const std::shared_ptr<GRM::Element> &plot_elem, int current_index,
                                        QGridLayout *grid_layout, int max_index, int index_name_start)
{
  int errind;
  double r, g, b;
  QImage image(max_index, 1, QImage::Format_RGB32);
  int index_min = index_name_start, index_max = index_name_start + max_index - 1;

  if (plot_elem->hasAttribute("colormap"))
    {
      auto colormap = static_cast<int>(plot_elem->getAttribute("colormap"));
      gr_setcolormap(colormap);
    }
  for (int index = 0; index < max_index; index++)
    {
      gks_inq_color_rep(-1, index_name_start, -1, &errind, &r, &g, &b);
      QRgb value = qRgb(255 * r, 255 * g, 255 * b);
      image.setPixel(index, 0, value);
      index_name_start += 1;
    }

  auto label_pix = new QLabel();
  auto color_pic = QPixmap::fromImage(image);
  color_pic = color_pic.scaled(540, 20);
  label_pix->setPixmap(color_pic);

  auto slider = new QSlider();
  slider->setRange(index_min, index_max);
  slider->setOrientation(Qt::Horizontal);
  gks_inq_color_rep(-1, index_min, -1, &errind, &r, &g, &b);
  slider->setStyleSheet(("QSlider::groove:horizontal {background: transparent;border: none;} "
                         "QSlider::handle:horizontal {background: transparent;width:4px;margin:-5px " +
                         std::to_string(540 / max_index / 2 - 2) + "px;}")
                            .c_str());
  QObject::connect(slider, &QSlider::valueChanged, [=]() {
    double r2, g2, b2;
    int errind2;
    slider->setStyleSheet(("QSlider::groove:horizontal {background: transparent;border: none;} "
                           "QSlider::handle:horizontal {background: #000000;width:4px;margin:-5px " +
                           std::to_string(540 / max_index / 2 - 2) + "px;}")
                              .c_str());

    if (last_used_slider != slider) resetLastSlider();
    last_used_slider = slider;

    gks_inq_color_rep(-1, last_used_slider->value(), -1, &errind2, &r2, &g2, &b2);
    result->setStyleSheet(("QLabel{background-color:rgb(" + std::to_string(255 * r2) + "," + std::to_string(255 * g2) +
                           "," + std::to_string(255 * b2) + ");border:1px solid black;}")
                              .c_str());
    triggered_by_index = true;
    index_value->setText(std::to_string(last_used_slider->value()).c_str());
    if (!triggered_by_text)
      {
        red_slider->setValue(255 * r2);
        green_slider->setValue(255 * g2);
        blue_slider->setValue(255 * b2);
      }
    triggered_by_index = false;
  });
  if (current_index >= index_min && current_index <= index_max) slider->setValue(current_index);
  sliders.push_back(slider);

  grid_layout->addWidget(label_pix, 0, 0);
  grid_layout->addWidget(slider, 0, 0);
}

ColorPickerMixed::ColorPickerMixed(GRPlotWidget *widget, QDialog *parent) : QDialog(parent)
{
  grplot_widget = widget;
}

void ColorPickerMixed::start(const std::shared_ptr<GRM::Element> &element, const std::string &attribute_name,
                             const int current_index)
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr && layout_grid->querySelectorsAll("layout_grid_element").size() > 1)
                               ? layout_grid->querySelectors("[_selected_for_menu]")
                               : global_root->querySelectors("figure[active=1]");
  if (figure_elem == nullptr) return;
  const auto plot_elems = figure_elem->querySelectorsAll("plot");
  std::shared_ptr<GRM::Element> plot_elem;

  if (plot_elems.size() > 1)
    plot_elem = figure_elem->querySelectors("plot[_selected_for_menu=\"1\"]");
  else
    plot_elem = plot_elems[0];

  this->setWindowTitle(attribute_name.c_str());
  this->setFixedSize(605, 600);

  index_value = new QLineEdit(this);
  index_value->setText("(Index)");
  index_value->setAlignment(Qt::AlignCenter);
  QObject::connect(index_value, &QLineEdit::textChanged, [=]() {
    double r2, g2, b2;
    int errind2;
    if (!triggered_by_index) resetLastSlider();
    if (util::isNumber(index_value->text().toStdString()) && index_value->text() != nullptr)
      {
        auto num = std::stoi(index_value->text().toStdString());
        num = std::min(std::max(num, 0), 1256);
        if (num >= 80 && num <= 168)
          num = std::min(num, 80);
        else if (num > 168 && num <= 255)
          num = std::max(num, 256);
        index_value->setText(std::to_string(num).c_str());

        gks_inq_color_rep(-1, num, -1, &errind2, &r2, &g2, &b2);
        result->setStyleSheet(("QLabel{background-color:rgb(" + std::to_string(255 * r2) + "," +
                               std::to_string(255 * g2) + "," + std::to_string(255 * b2) + ");border:1px solid black;}")
                                  .c_str());
        triggered_by_index = true;
        if (!triggered_by_text)
          {
            red_slider->setValue(255 * r2);
            green_slider->setValue(255 * g2);
            blue_slider->setValue(255 * b2);
          }
        for (const auto slider : sliders)
          {
            if (num >= slider->minimum() && num <= slider->maximum())
              {
                slider->setValue(num);
                break;
              }
          }
        triggered_by_index = false;
      }
  });

  auto grid_layout_ansi = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_ansi, 8, 0);
  auto grid_layout_reduced_cmap = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_reduced_cmap, 80 - 8, 8);
  auto grid_layout_rainbow = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_rainbow, 588 - 257, 257);
  auto grid_layout_gray = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_gray, 644 - 588, 588);
  auto grid_layout_blue = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_blue, 700 - 644, 644);
  auto grid_layout_magenta = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_magenta, 756 - 700, 700);
  auto grid_layout_red = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_red, 812 - 756, 756);
  auto grid_layout_yellow = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_yellow, 868 - 812, 812);
  auto grid_layout_green = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_green, 924 - 868, 868);
  auto grid_layout_cian = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_cian, 980 - 924, 924);
  auto grid_layout_high_diff_colors = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_high_diff_colors, 1000 - 980, 980);
  auto grid_layout_cmap = new QGridLayout;
  colorIndexHelper(plot_elem, current_index, grid_layout_cmap, 1255 - 1000, 1000);

  auto form = new QFormLayout();

  form->addRow(grid_layout_high_diff_colors);
  form->addRow(grid_layout_ansi);
  form->addRow(grid_layout_cmap);
  form->addRow(grid_layout_rainbow);
  form->addRow(grid_layout_gray);
  form->addRow(grid_layout_blue);
  form->addRow(grid_layout_magenta);
  form->addRow(grid_layout_red);
  form->addRow(grid_layout_yellow);
  form->addRow(grid_layout_green);
  form->addRow(grid_layout_cian);
  form->addRow(grid_layout_reduced_cmap);

  int ref_r = 0, ref_g = 0, ref_b = 0;
  std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();
  if (element->hasAttribute(attribute_name) && element->getAttribute(attribute_name).isString())
    {
      auto context_ref = static_cast<std::string>(element->getAttribute(attribute_name));
      auto rgb_vec = GRM::get<std::vector<double>>((*context)[context_ref]);
      ref_r = rgb_vec.at(0) * 255;
      ref_g = rgb_vec.at(1) * 255;
      ref_b = rgb_vec.at(2) * 255;
    }
  else if (element->hasAttribute(attribute_name) && element->getAttribute(attribute_name).isInt())
    {
      double r2, g2, b2;
      int errind2;

      gks_inq_color_rep(-1, last_used_slider->value(), -1, &errind2, &r2, &g2, &b2);
      ref_r = r2 * 255;
      ref_g = g2 * 255;
      ref_b = b2 * 255;
    }

  red_slider = new QSlider(this);
  red_slider->setRange(0, 255);
  red_slider->setOrientation(Qt::Horizontal);
  red_slider->setValue(ref_r);
  red_value = new QLineEdit(this);
  red_value->setText(std::to_string(red_slider->value()).c_str());
  red_value->setAlignment(Qt::AlignCenter);
  red_value->setStyleSheet("QLineEdit {color:red}");

  green_slider = new QSlider(this);
  green_slider->setRange(0, 255);
  green_slider->setOrientation(Qt::Horizontal);
  green_slider->setValue(ref_g);
  green_value = new QLineEdit(this);
  green_value->setText(std::to_string(green_slider->value()).c_str());
  green_value->setAlignment(Qt::AlignCenter);
  green_value->setStyleSheet("QLineEdit {color:green}");

  blue_slider = new QSlider(this);
  blue_slider->setRange(0, 255);
  blue_slider->setOrientation(Qt::Horizontal);
  blue_slider->setValue(ref_b);
  blue_value = new QLineEdit(this);
  blue_value->setText(std::to_string(blue_slider->value()).c_str());
  blue_value->setAlignment(Qt::AlignCenter);
  blue_value->setStyleSheet("QLineEdit {color:blue}");

  result = new QLabel(this);
  result->setStyleSheet(("QLabel{background-color:rgb(" + std::to_string(red_slider->value()) + "," +
                         std::to_string(green_slider->value()) + "," + std::to_string(blue_slider->value()) +
                         ");border:1px solid black;}")
                            .c_str());

  QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
  QObject::connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));

  auto grid_layout = new QGridLayout;
  auto scroll_area_content = new QWidget;
  scroll_area_content->setLayout(form);

  grid_layout->addWidget(scroll_area_content, 0, 0, 1, 6);
  grid_layout->addWidget(red_value, 2, 0, 1, 2);
  grid_layout->addWidget(green_value, 2, 2, 1, 2);
  grid_layout->addWidget(blue_value, 2, 4, 1, 2);
  grid_layout->addWidget(red_slider, 3, 0, 1, 2);
  grid_layout->addWidget(green_slider, 3, 2, 1, 2);
  grid_layout->addWidget(blue_slider, 3, 4, 1, 2);
  grid_layout->addWidget(result, 4, 0, 2, 4);
  grid_layout->addWidget(index_value, 4, 4, 2, 2);
  grid_layout->addWidget(button_box, 6, 0, 1, 6);

  if (this->layout() != nullptr)
    {
      QLayoutItem *item;
      while ((item = this->layout()->takeAt(0)) != nullptr)
        {
          delete item->widget();
          delete item;
        }
      delete this->layout();
    }
  this->setLayout(grid_layout);

  connect(red_slider, SIGNAL(valueChanged(int)), this, SLOT(redSliderChangeSlot()));
  connect(red_value, SIGNAL(textChanged(QString)), this, SLOT(redLineEditChangeSlot()));
  connect(green_slider, SIGNAL(valueChanged(int)), this, SLOT(greenSliderChangeSlot()));
  connect(green_value, SIGNAL(textChanged(QString)), this, SLOT(greenLineEditChangeSlot()));
  connect(blue_slider, SIGNAL(valueChanged(int)), this, SLOT(blueSliderChangeSlot()));
  connect(blue_value, SIGNAL(textChanged(QString)), this, SLOT(blueLineEditChangeSlot()));

  attr_name = attribute_name;
  elem = element;
}

void ColorPickerMixed::redSliderChangeSlot()
{
  red_value->setText(std::to_string(red_slider->value()).c_str());
  result->setStyleSheet("QLabel{background-color:rgb(" + red_value->text() + "," + green_value->text() + "," +
                        blue_value->text() + ");border:1px solid black;}");

  if (!triggered_by_index)
    {
      resetLastSlider();
      auto ind = gr_rgbcolorexists(red_slider->value() / 255.0, green_slider->value() / 255.0,
                                   blue_slider->value() / 255.0, 80, 256, 1);
      if (ind != -1)
        index_value->setText(std::to_string(ind).c_str());
      else
        index_value->setText("(Index)");
    }
}

void ColorPickerMixed::greenSliderChangeSlot()
{
  green_value->setText(std::to_string(green_slider->value()).c_str());
  result->setStyleSheet("QLabel{background-color:rgb(" + red_value->text() + "," + green_value->text() + "," +
                        blue_value->text() + ");border:1px solid black;}");

  if (!triggered_by_index)
    {
      resetLastSlider();
      auto ind = gr_rgbcolorexists(red_slider->value() / 255.0, green_slider->value() / 255.0,
                                   blue_slider->value() / 255.0, 80, 256, 1);
      if (ind != -1)
        index_value->setText(std::to_string(ind).c_str());
      else
        index_value->setText("(Index)");
    }
}

void ColorPickerMixed::blueSliderChangeSlot()
{
  blue_value->setText(std::to_string(blue_slider->value()).c_str());
  result->setStyleSheet("QLabel{background-color:rgb(" + red_value->text() + "," + green_value->text() + "," +
                        blue_value->text() + ");border:1px solid black;}");

  if (!triggered_by_index)
    {
      resetLastSlider();
      auto ind = gr_rgbcolorexists(red_slider->value() / 255.0, green_slider->value() / 255.0,
                                   blue_slider->value() / 255.0, 80, 256, 1);
      if (ind != -1)
        index_value->setText(std::to_string(ind).c_str());
      else
        index_value->setText("(Index)");
    }
}

void ColorPickerMixed::redLineEditChangeSlot()
{
  triggered_by_text = true;
  if (!red_value->text().isEmpty()) red_slider->setValue(std::stoi(red_value->text().toStdString()));
  if (green_value->text().isEmpty()) green_slider->setValue(0);
  if (blue_value->text().isEmpty()) blue_slider->setValue(0);
  triggered_by_text = false;
}

void ColorPickerMixed::greenLineEditChangeSlot()
{
  triggered_by_text = true;
  if (!green_value->text().isEmpty()) green_slider->setValue(std::stoi(green_value->text().toStdString()));
  if (red_value->text().isEmpty()) red_slider->setValue(0);
  if (blue_value->text().isEmpty()) blue_slider->setValue(0);
  triggered_by_text = false;
}

void ColorPickerMixed::blueLineEditChangeSlot()
{
  triggered_by_text = true;
  if (!blue_value->text().isEmpty()) blue_slider->setValue(std::stoi(blue_value->text().toStdString()));
  if (red_value->text().isEmpty()) red_slider->setValue(0);
  if (green_value->text().isEmpty()) green_slider->setValue(0);
  triggered_by_text = false;
}

void ColorPickerMixed::reject()
{
  this->done(QDialog::Rejected);
}

void ColorPickerMixed::accept()
{
  std::vector<std::string> values;
  std::vector<double> data_vec;
  std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();
  bool editor_enabled = false;
  grm_get_render()->getEnableEditor(&editor_enabled);

  if (editor_enabled) grplot_widget->createHistoryElement();

  if (auto elem_locked = elem.lock(); elem_locked != nullptr)
    {
      auto global_root = grm_get_document_root();
      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = "_rgb" + std::to_string(id);
      bool rgb_changed = true;

      if (index_value->text() == "(Index)")
        {
          auto rgb_key = static_cast<std::string>(elem_locked->getAttribute(attr_name));
          if (rgb_key != "" && !util::isNumber(rgb_key) && elem_locked->getAttribute(attr_name).isString())
            {
              auto rgb_vec = GRM::get<std::vector<double>>((*context)[rgb_key]);
              if (std::to_string(int(rgb_vec[0] * 255)) == red_value->text().toStdString() &&
                  std::to_string(int(rgb_vec[1] * 255)) == green_value->text().toStdString() &&
                  std::to_string(int(rgb_vec[2] * 255)) == blue_value->text().toStdString())
                rgb_changed = false;
            }
        }
      if (elem_locked->localName() == "sphere" || elem_locked->localName() == "spin" ||
          elem_locked->localName() == "series_isosurface" || elem_locked->localName() == "unit_cell")
        rgb_changed = true;

      if ((red_value->text() != "0" || green_value->text() != "0" || blue_value->text() != "0") &&
          (rgb_changed || last_used_slider == nullptr))
        {
          double new_rgb_value[3] = {std::stod(red_value->text().toStdString()) / 255.0,
                                     std::stod(green_value->text().toStdString()) / 255.0,
                                     std::stod(blue_value->text().toStdString()) / 255.0};
          std::vector<double> new_rgb_value_vec(new_rgb_value, new_rgb_value + 3);
          (*context)[attr_name + str] = new_rgb_value_vec;
          elem_locked->setAttribute(attr_name, attr_name + str);
        }
      else if (last_used_slider != nullptr)
        {
          if (last_used_slider->styleSheet().toStdString().find("background: #000000") != std::string::npos)
            elem_locked->setAttribute(attr_name, last_used_slider->value());
        }
      else if (index_value->text() != "(Index)" && util::isNumber(index_value->text().toStdString()) &&
               index_value->text() != nullptr)
        {
          auto num = std::stoi(index_value->text().toStdString());
          if ((num < 80 || num > 256) && num > 0 && num < 1256) elem_locked->setAttribute(attr_name, num);
        }
    }
  attr_name = "";
  sliders.clear();
  last_used_slider = nullptr;
  this->done(QDialog::Accepted);
}
