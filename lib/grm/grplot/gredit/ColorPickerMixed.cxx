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

ColorPickerMixed::ColorPickerMixed(GRPlotWidget *widget, QDialog *parent) : QDialog(parent)
{
  grplot_widget_ = widget;

  this->setFixedSize(605, 600);

  auto gr_color_slider_ansi = new GRColorSlider({0, 7}, this);
  // auto gr_color_slider_reduced_cmap = new GRColorSlider({8, 79}, this);
  auto gr_color_slider_rainbow = new GRColorSlider({257, 587}, this);
  auto gr_color_slider_gray = new GRColorSlider({588, 643}, this);
  auto gr_color_slider_blue = new GRColorSlider({644, 699}, this);
  auto gr_color_slider_magenta = new GRColorSlider({700, 755}, this);
  auto gr_color_slider_red = new GRColorSlider({756, 811}, this);
  auto gr_color_slider_yellow = new GRColorSlider({812, 865}, this);
  auto gr_color_slider_green = new GRColorSlider({868, 923}, this);
  auto gr_color_slider_cyan = new GRColorSlider({924, 979}, this);
  auto gr_color_slider_high_diff_colors = new GRColorSlider({980, 999}, this);
  auto gr_color_slider_cmap = new GRColorSlider({1000, 1255}, this);
  sliders_.push_back(gr_color_slider_high_diff_colors);
  sliders_.push_back(gr_color_slider_ansi);
  sliders_.push_back(gr_color_slider_cmap);
  sliders_.push_back(gr_color_slider_rainbow);
  sliders_.push_back(gr_color_slider_gray);
  sliders_.push_back(gr_color_slider_blue);
  sliders_.push_back(gr_color_slider_magenta);
  sliders_.push_back(gr_color_slider_red);
  sliders_.push_back(gr_color_slider_yellow);
  sliders_.push_back(gr_color_slider_green);
  sliders_.push_back(gr_color_slider_cyan);
  // sliders_.push_back(gr_color_slider_reduced_cmap);

  rgb_picker_ = new GRRGBPicker(this);

  QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);

  // Add widgets to a vertical layout
  auto layout = new QVBoxLayout();
  for (const auto slider : sliders_) layout->addWidget(slider);
  layout->addWidget(rgb_picker_);
  layout->addWidget(button_box);
  this->setLayout(layout);

  // Connect signals
  for (const auto slider : sliders_)
    {
      QObject::connect(slider, &GRColorSlider::grColorChanged, [this, slider](int gr_color) {
        if (gr_color < 0) return;
        rgb_picker_->setColor(gr_color);
        for (const auto other_slider : sliders_)
          {
            if (other_slider == slider) continue;
            other_slider->setGRColor(-1);
          }
      });
      QObject::connect(rgb_picker_, &GRRGBPicker::colorChanged, slider, &GRColorSlider::setGRColor);
    }
  QObject::connect(button_box, &QDialogButtonBox::accepted, this, &ColorPickerMixed::accept);
  QObject::connect(button_box, &QDialogButtonBox::rejected, this, &ColorPickerMixed::reject);
}

void ColorPickerMixed::start(const std::shared_ptr<GRM::Element> &element, const std::string &attribute_name,
                             const int current_index)
{
  attr_name_ = attribute_name;
  elem_ = element;

  this->setWindowTitle(attribute_name.c_str());

  for (auto slider : sliders_)
    {
      slider->updateColors();
    }

  std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();
  if (element->hasAttribute(attribute_name) && element->getAttribute(attribute_name).isString())
    {
      auto context_ref = static_cast<std::string>(element->getAttribute(attribute_name));
      auto rgb_vec = GRM::get<std::vector<double>>((*context)[context_ref]);
      int r = std::round(rgb_vec.at(0) * 255);
      int g = std::round(rgb_vec.at(1) * 255);
      int b = std::round(rgb_vec.at(2) * 255);

      rgb_picker_->setColor(QColor::fromRgb(r, g, b));
    }
  else if (element->hasAttribute(attribute_name) && element->getAttribute(attribute_name).isInt())
    {
      auto gr_color_index = static_cast<int>(element->getAttribute(attribute_name));
      rgb_picker_->setColor(gr_color_index);
    }
  else
    {
      // Fall back to black if no color is set on the element
      rgb_picker_->setColor(1);
    }
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

  if (editor_enabled) grplot_widget_->createHistoryElement();

  if (auto elem_locked = elem_.lock(); elem_locked != nullptr)
    {
      auto global_root = grm_get_document_root();

      if (rgb_picker_->grColor() >= 0)
        {
          auto num = rgb_picker_->grColor();
          if ((num < 80 || num > 256) && num > 0 && num < 1256) elem_locked->setAttribute(attr_name_, num);
        }
      else
        {
          auto id = static_cast<int>(global_root->getAttribute("_id"));
          auto str = "_rgb" + std::to_string(id);
          bool rgb_changed = true;
          auto rgb_key = static_cast<std::string>(elem_locked->getAttribute(attr_name_));

          if (elem_locked->localName() == "sphere" || elem_locked->localName() == "spin" ||
              elem_locked->localName() == "series_isosurface" || elem_locked->localName() == "unit_cell")
            {
              rgb_changed = true;
            }
          else if (rgb_key != "" && !util::isNumber(rgb_key) && elem_locked->getAttribute(attr_name_).isString())
            {
              auto rgb_vec = GRM::get<std::vector<double>>((*context)[rgb_key]);
              rgb_changed = (QColor::fromRgbF(rgb_vec[0], rgb_vec[1], rgb_vec[2]) != rgb_picker_->color());
            }

          if (rgb_changed)
            {
              std::vector<double> new_rgb_value_vec{rgb_picker_->color().redF(), rgb_picker_->color().greenF(),
                                                    rgb_picker_->color().blueF()};
              (*context)[attr_name_ + str] = new_rgb_value_vec;
              elem_locked->setAttribute(attr_name_, attr_name_ + str);
            }
        }
    }
  this->done(QDialog::Accepted);
}
