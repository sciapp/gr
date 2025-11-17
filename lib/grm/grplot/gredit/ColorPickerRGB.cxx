#include "ColorPickerRGB.hxx"

#include <QDialog>
#include <QLabel>

static std::weak_ptr<GRM::Element> elem;
static std::string attr_name;

ColorPickerRGB::ColorPickerRGB(GRPlotWidget *widget, QDialog *parent) : QDialog(parent)
{
  grplot_widget = widget;
}

void ColorPickerRGB::start(std::string attribute_name, const std::shared_ptr<GRM::Element> &element)
{
  const auto global_root = grm_get_document_root();
  const auto layout_grid = global_root->querySelectors("figure[active=1]")->querySelectors("layout_grid");
  const auto figure_elem = (layout_grid != nullptr) ? layout_grid->querySelectors("[_selected_for_menu]")
                                                    : global_root->querySelectors("figure[active=1]");
  const auto plot_elem = figure_elem->querySelectors("plot");

  this->setWindowTitle(attribute_name.c_str());
  this->setFixedSize(400, 230);
  auto grid_layout = new QGridLayout;

  int ref_r = 0, ref_g = 0, ref_b = 0;
  std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();
  if (element->hasAttribute(attribute_name))
    {
      auto context_ref = static_cast<std::string>(element->getAttribute(attribute_name));
      auto rgb_vec = GRM::get<std::vector<double>>((*context)[context_ref]);
      ref_r = rgb_vec.at(0) * 255;
      ref_g = rgb_vec.at(1) * 255;
      ref_b = rgb_vec.at(2) * 255;
    }

  red_slider = new QSlider(this);
  red_slider->setRange(0, 255);
  red_slider->setOrientation(Qt::Horizontal);
  red_slider->setValue(ref_r);
  red_label = new QLabel("Red");
  red_label->setStyleSheet("QLabel{color:red;}");
  red_value = new QLineEdit(this);
  red_value->setText(std::to_string(red_slider->value()).c_str());
  red_value->setAlignment(Qt::AlignCenter);

  green_slider = new QSlider(this);
  green_slider->setRange(0, 255);
  green_slider->setOrientation(Qt::Horizontal);
  green_slider->setValue(ref_g);
  green_label = new QLabel("Green");
  green_label->setStyleSheet("QLabel{color:green;}");
  green_value = new QLineEdit(this);
  green_value->setText(std::to_string(green_slider->value()).c_str());
  green_value->setAlignment(Qt::AlignCenter);

  blue_slider = new QSlider(this);
  blue_slider->setRange(0, 255);
  blue_slider->setOrientation(Qt::Horizontal);
  blue_slider->setValue(ref_b);
  blue_label = new QLabel("Blue");
  blue_label->setStyleSheet("QLabel{color:blue;}");
  blue_value = new QLineEdit(this);
  blue_value->setText(std::to_string(blue_slider->value()).c_str());
  blue_value->setAlignment(Qt::AlignCenter);

  result = new QLabel(this);
  result->setStyleSheet(("QLabel{background-color:rgb(" + std::to_string(red_slider->value()) + "," +
                         std::to_string(green_slider->value()) + "," + std::to_string(blue_slider->value()) +
                         ");border:1px solid black;}")
                            .c_str());

  QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
  QObject::connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));

  grid_layout->addWidget(red_label, 0, 0, Qt::AlignCenter);
  grid_layout->addWidget(green_label, 0, 1, Qt::AlignCenter);
  grid_layout->addWidget(blue_label, 0, 2, Qt::AlignCenter);
  grid_layout->addWidget(red_value, 1, 0);
  grid_layout->addWidget(green_value, 1, 1, Qt::AlignCenter);
  grid_layout->addWidget(blue_value, 1, 2, Qt::AlignCenter);
  grid_layout->addWidget(red_slider, 2, 0);
  grid_layout->addWidget(green_slider, 2, 1);
  grid_layout->addWidget(blue_slider, 2, 2);
  grid_layout->addWidget(result, 3, 0, 2, 3);
  grid_layout->addWidget(button_box, 5, 0, 1, 3);

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
  connect(red_value, SIGNAL(returnPressed()), this, SLOT(redLineEditChangeSlot()));
  connect(green_slider, SIGNAL(valueChanged(int)), this, SLOT(greenSliderChangeSlot()));
  connect(green_value, SIGNAL(returnPressed()), this, SLOT(greenLineEditChangeSlot()));
  connect(blue_slider, SIGNAL(valueChanged(int)), this, SLOT(blueSliderChangeSlot()));
  connect(blue_value, SIGNAL(returnPressed()), this, SLOT(blueLineEditChangeSlot()));

  attr_name = attribute_name;
  elem = element;
}

void ColorPickerRGB::redSliderChangeSlot()
{
  red_value->setText(std::to_string(red_slider->value()).c_str());
  result->setStyleSheet("QLabel{background-color:rgb(" + red_value->text() + "," + green_value->text() + "," +
                        blue_value->text() + ");border:1px solid black;}");
}

void ColorPickerRGB::greenSliderChangeSlot()
{
  green_value->setText(std::to_string(green_slider->value()).c_str());
  result->setStyleSheet("QLabel{background-color:rgb(" + red_value->text() + "," + green_value->text() + "," +
                        blue_value->text() + ");border:1px solid black;}");
}

void ColorPickerRGB::blueSliderChangeSlot()
{
  blue_value->setText(std::to_string(blue_slider->value()).c_str());
  result->setStyleSheet("QLabel{background-color:rgb(" + red_value->text() + "," + green_value->text() + "," +
                        blue_value->text() + ");border:1px solid black;}");
}

void ColorPickerRGB::redLineEditChangeSlot()
{
  red_slider->setValue(std::stoi(red_value->text().toStdString()));
}

void ColorPickerRGB::greenLineEditChangeSlot()
{
  green_slider->setValue(std::stoi(green_value->text().toStdString()));
}

void ColorPickerRGB::blueLineEditChangeSlot()
{
  blue_slider->setValue(std::stoi(blue_value->text().toStdString()));
}

void ColorPickerRGB::reject()
{
  this->done(QDialog::Rejected);
}

void ColorPickerRGB::accept()
{
  grplot_widget->createHistoryElement();
  if (auto elem_locked = elem.lock(); elem_locked != nullptr)
    {
      auto global_root = grm_get_document_root();
      auto id = static_cast<int>(global_root->getAttribute("_id"));
      auto str = std::to_string(id);
      std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();

      double new_rgb_value[3] = {std::stod(red_value->text().toStdString()) / 255.0,
                                 std::stod(green_value->text().toStdString()) / 255.0,
                                 std::stod(blue_value->text().toStdString()) / 255.0};
      std::vector<double> new_rgb_value_vec(new_rgb_value, new_rgb_value + 3);
      (*context)[attr_name + str] = new_rgb_value_vec;
      elem_locked->setAttribute(attr_name, attr_name + str);
    }
  attr_name = "";
  this->done(QDialog::Accepted);
}
