#include "GRRGBPicker.hxx"
#include <QGridLayout>
#include <QSignalBlocker>
#include <gks.h>
#include <gr.h>

GRRGBPicker::GRRGBPicker(QWidget *parent, double tolerated_color_distance)
    : QWidget(parent), tolerated_color_distance_(tolerated_color_distance)
{
  initUI();
}
GRRGBPicker::GRRGBPicker(int gr_color, QWidget *parent, double tolerated_color_distance)
    : QWidget(parent), tolerated_color_distance_(tolerated_color_distance)
{
  initUI(gr_color);
}
GRRGBPicker::GRRGBPicker(const QColor &color, QWidget *parent, double tolerated_color_distance)
    : QWidget(parent), tolerated_color_distance_(tolerated_color_distance)
{
  initUI(-1, color);
}

int GRRGBPicker::grColor() const
{
  return gr_color_;
}

QColor GRRGBPicker::color() const
{
  return color_;
}

bool GRRGBPicker::setColor(int gr_color)
{
  return setColorHelper(gr_color, QColor());
}

bool GRRGBPicker::setColor(const QColor &color)
{
  return setColorHelper(-1, color);
}

bool GRRGBPicker::setColorHelper(int gr_color, const QColor &color)
{
  if ((gr_color < 0 || gr_color > 1255) && !color.isValid()) return false;

  // Block signals sent by these widgets avoid circular updates during this function
  std::list<QSignalBlocker> blockers;
  for (auto widget : std::initializer_list<QWidget *>{
           red_slider_,
           green_slider_,
           blue_slider_,
           red_value_,
           green_value_,
           blue_value_,
           gr_color_index_value_,
       })
    {
      blockers.emplace_back(widget);
    }

  if (gr_color >= 0)
    {
      // Find RGB color for this GR color index
      gr_color_ = gr_color;
      double r, g, b;
      int error_indicator;
      gks_inq_color_rep(-1, gr_color_, -1, &error_indicator, &r, &g, &b);
      color_ = QColor::fromRgbF(r, g, b);
    }
  else
    {
      // Try to find a near GR color index for this RGB color
      color_ = color;
      gr_color_ = gr_inqnearestcolorfromrgb(color.redF(), color.greenF(), color.blueF(), 80, 256,
                                            tolerated_color_distance_, NULL);
    }
  red_slider_->setValue(color_.red());
  green_slider_->setValue(color_.green());
  blue_slider_->setValue(color_.blue());
  red_value_->setText(QString::number(color_.red()));
  green_value_->setText(QString::number(color_.green()));
  blue_value_->setText(QString::number(color_.blue()));
  mixed_color_->setStyleSheet(QString("background-color: rgb(%1, %2, %3); border:1px solid black;")
                                  .arg(color_.red())
                                  .arg(color_.green())
                                  .arg(color_.blue()));
  gr_color_index_value_->setText(gr_color_ < 0 ? "(Index)" : QString::number(gr_color_));

  emit colorChanged(gr_color_, color_);

  return true;
}

void GRRGBPicker::initUI(int gr_color, const QColor &color)
{
  red_slider_ = new QSlider(this);
  green_slider_ = new QSlider(this);
  blue_slider_ = new QSlider(this);

  red_value_ = new QLineEdit(this);
  green_value_ = new QLineEdit(this);
  blue_value_ = new QLineEdit(this);

  mixed_color_ = new QLabel(this);
  gr_color_index_value_ = new QLineEdit(this);

  for (auto [slider, text_edit, color_str] : {
           std::tuple{red_slider_, red_value_, "red"},
           std::tuple{green_slider_, green_value_, "green"},
           std::tuple{blue_slider_, blue_value_, "blue"},
       })
    {
      slider->setRange(0, 255);
      slider->setOrientation(Qt::Horizontal);
      text_edit->setAlignment(Qt::AlignCenter);
      text_edit->setStyleSheet(QString("color: %1;").arg(color_str));
    }

  setColorHelper(gr_color, color);

  // Connect sliders to `setColorHelper` which consistently sets all other widgets and emits `grColorChanged`
  for (auto slider : {red_slider_, green_slider_, blue_slider_})
    {
      connect(slider, &QSlider::valueChanged, [this](int value) {
        setColorHelper(-1, QColor(red_slider_->value(), green_slider_->value(), blue_slider_->value()));
      });
    }
  // Connect text edits to sliders which then will call `setColorHelper`
  for (auto [text_edit, slider] : {
           std::tuple{red_value_, red_slider_},
           std::tuple{green_value_, green_slider_},
           std::tuple{blue_value_, blue_slider_},
       })
    {
      connect(text_edit, &QLineEdit::textChanged, [slider = slider](const QString &value) {
        // Change slider value on valid text and let it emit a valueChanged signal which will process the new value
        bool ok;
        auto value_int = value.toInt(&ok);
        if (!ok || value_int < 0 || value_int > 255) return;
        slider->setValue(value_int);
      });
    }
  // Connect the textedit for the GR color index also to `setColorHelper`
  connect(gr_color_index_value_, &QLineEdit::textChanged, [this](const QString &value) {
    bool ok;
    auto value_int = value.toInt(&ok);
    if (!ok || (value_int >= 80 && value_int <= 256)) return;
    setColorHelper(value_int, QColor());
  });

  auto layout = new QGridLayout();
  layout->addWidget(red_value_, 0, 0, 1, 1);
  layout->addWidget(green_value_, 0, 1, 1, 1);
  layout->addWidget(blue_value_, 0, 2, 1, 1);
  layout->addWidget(red_slider_, 1, 0, 1, 1);
  layout->addWidget(green_slider_, 1, 1, 1, 1);
  layout->addWidget(blue_slider_, 1, 2, 1, 1);
  layout->addWidget(mixed_color_, 2, 0, 1, 2);
  layout->addWidget(gr_color_index_value_, 2, 2, 1, 1);
  setLayout(layout);
}
