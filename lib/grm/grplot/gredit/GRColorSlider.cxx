#include "GRColorSlider.hxx"
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include <gks.h>

/*!
 * \brief Check if a color is light or dark by calculating its brightness.
 *
 * \param[in] r The red intensity of the color (0.0 to 1.0).
 * \param[in] g The green intensity of the color (0.0 to 1.0).
 * \param[in] b The blue intensity of the color (0.0 to 1.0).
 * \return whether the color is dark or not.
 */
static bool isDark(double r, double g, double b)
{
  return 0.299 * r + 0.587 * g + 0.114 * b < 0.25;
}
static bool isDark(const QColor &color)
{
  return isDark(color.redF(), color.greenF(), color.blueF());
}

GRColorSlider::GRColorSlider(QWidget *parent) : GRColorSlider(1000, 1255, parent) {}
GRColorSlider::GRColorSlider(unsigned int gr_color_range_min, unsigned int gr_color_range_max, QWidget *parent)
    : GRColorSlider({gr_color_range_min, gr_color_range_max}, parent)
{
}
GRColorSlider::GRColorSlider(const std::pair<unsigned int, unsigned int> &gr_color_range, QWidget *parent)
    : QWidget(parent), gr_color_range_(gr_color_range)
{
}

void GRColorSlider::updateColors()
{
  gr_colors_ = QImage();
  update();
}

int GRColorSlider::grColor() const
{
  return selected_gr_color_;
}

QColor GRColorSlider::color() const
{
  if (selected_gr_color_ < 0) return QColor();
  return QColor::fromRgb(gr_colors_.pixelColor(selected_gr_color_ - gr_color_range_.first, 0).red(),
                         gr_colors_.pixelColor(selected_gr_color_ - gr_color_range_.first, 0).green(),
                         gr_colors_.pixelColor(selected_gr_color_ - gr_color_range_.first, 0).blue());
}

bool GRColorSlider::setGRColor(int gr_color)
{
  bool valid_input_arg = (gr_color >= gr_color_range_.first && gr_color <= gr_color_range_.second) || gr_color < 0;
  auto old_gr_color = selected_gr_color_;
  selected_gr_color_ = valid_input_arg && gr_color >= 0 ? gr_color : -1;
  if (selected_gr_color_ != old_gr_color)
    {
      emit grColorChanged(selected_gr_color_);
      update();
    }
  return valid_input_arg;
}

void GRColorSlider::mouseMoveEvent(QMouseEvent *event)
{
  if (!mouse_left_pressed_) return;
  updateColorSelectionFromMouse(event->pos().x());
}

void GRColorSlider::mousePressEvent(QMouseEvent *event)
{
  if (event->button() != Qt::LeftButton) return;
  mouse_left_pressed_ = true;
  updateColorSelectionFromMouse(event->pos().x());
}

void GRColorSlider::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() != Qt::LeftButton) return;
  mouse_left_pressed_ = false;
}

void GRColorSlider::paintEvent(QPaintEvent *)
{
  if (gr_colors_.isNull())
    {
      updateQImageColors();
    }

  QPainter painter(this);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
  const auto groove_padding_absolute = static_cast<int>(height() * GROOVE_PADDING);
  painter.drawImage(QRect{0, groove_padding_absolute, width(), height() - 2 * groove_padding_absolute}, gr_colors_);

  if (selected_gr_color_ < 0) return;

  int handle_pos_x = std::round((selected_gr_color_ - gr_color_range_.first + 0.5) * width() /
                                (gr_color_range_.second - gr_color_range_.first + 1));
  QPen pen{isSelectedColorDark() ? QColor::fromRgb(255, 255, 255) : QColor::fromRgb(0, 0, 0)};
  pen.setWidth(HANDLE_WIDTH);
  painter.setPen(pen);
  painter.drawLine(QPoint{handle_pos_x, 0}, QPoint{handle_pos_x, height()});
}

QSize GRColorSlider::sizeHint() const
{
  return QSize(200, 20);
}

bool GRColorSlider::isSelectedColorDark()
{
  return isDark(gr_colors_.pixelColor(selected_gr_color_ - gr_color_range_.first, 0));
}

void GRColorSlider::updateQImageColors()
{
  if (gr_colors_.isNull() ||
      gr_colors_.size() != QSize{static_cast<int>(gr_color_range_.second - gr_color_range_.first), 1})
    {
      gr_colors_ =
          QImage{static_cast<int>(gr_color_range_.second - gr_color_range_.first + 1), 1, QImage::Format_RGB888};
    }
  for (unsigned int x = 0; x < gr_colors_.width(); ++x)
    {
      double r, g, b;
      int error_indicator;
      gks_inq_color_rep(-1, x + gr_color_range_.first, -1, &error_indicator, &r, &g, &b);
      if (error_indicator != GKS_K_NO_ERROR) continue;
      gr_colors_.setPixelColor(x, 0, QColor::fromRgbF(r, g, b));
    }
}

void GRColorSlider::updateColorSelectionFromMouse(int mouse_x)
{
  int gr_color = (static_cast<double>(mouse_x) / width()) * (gr_color_range_.second - gr_color_range_.first + 1) +
                 gr_color_range_.first;
  setGRColor(gr_color);
}
