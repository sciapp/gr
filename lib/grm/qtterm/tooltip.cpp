#include "tooltip.h"

#define style \
  "\
    .gr-label {\n\
        color: #26aae1;\n\
        font-size: %dpx;\n\
        line-height: 0.8;\n\
    }\n\
    .gr-value {\n\
        color: #3c3c3c;\n\
        font-size: %dpx;\n\
        line-height: 0.8;\n\
    }"

#define tooltipTemplate \
  "\
    <span class=\"gr-label\">%s</span><br>\n\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span><br>\n\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span>"

#define tooltipTemplate_nolabel \
  "\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span><br>\n\
    <span class=\"gr-label\">%s: </span>\n\
    <span class=\"gr-value\">%.14g</span>"


Tooltip::Tooltip(QWidget *parent) : QWidget(parent), offset_y(0)
{
  setAttribute(Qt::WA_TransparentForMouseEvents);
  data = nullptr;
  hide();
}

void Tooltip::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event)
  int painter_width = 1;
  if (data && data->x_px >= 0 && data->y_px >= 0)
    {
      QPainter painter;
      painter.begin(this);
      QColor background(128, 128, 128, 128);

      if (data->x_px > parentWidget()->width() / 2)
        {
          QPointF points[4] = {QPointF(width(), offset_y + height() / 2.),
                               QPointF(width() - 5 - painter_width, offset_y + height() / 2. + 5),
                               QPointF(width() - 5 - painter_width, offset_y + height() / 2. - 5),
                               QPointF(width(), offset_y + height() / 2.)};
          painter.save();
          painter.setBrush(QBrush(background, Qt::SolidPattern));
          painter.setPen(Qt::NoPen);
          painter.drawPolygon(points, 4);
          painter.restore();
          background.setRgb(224, 224, 224, 128);
          painter.setPen(QPen(background, painter_width));
          painter.fillRect(painter_width, painter_width, (int)labelText.size().width() - painter_width * 2,
                           (int)labelText.size().height() - painter_width * 2, QBrush(background, Qt::SolidPattern));
          painter.drawRect(painter_width, painter_width, (int)labelText.size().width() - painter_width * 2,
                           (int)labelText.size().height() - painter_width * 2);
          labelText.drawContents(&painter);
        }
      else
        {
          QPointF points[4] = {
              QPointF(0, offset_y + height() / 2.), QPointF(5 + painter_width, offset_y + height() / 2. + 5),
              QPointF(5 + painter_width, offset_y + height() / 2. - 5), QPointF(0, offset_y + height() / 2.)};
          painter.save();
          painter.setBrush(QBrush(background, Qt::SolidPattern));
          painter.setPen(Qt::NoPen);
          painter.drawPolygon(points, 4);
          painter.restore();
          background.setRgb(224, 224, 224, 128);
          painter.setPen(QPen(background, painter_width));
          painter.fillRect(painter_width + 5, painter_width, (int)labelText.size().width() - painter_width * 2,
                           (int)labelText.size().height() - painter_width * 2, QBrush(background, Qt::SolidPattern));
          painter.drawRect(painter_width + 5, painter_width, (int)labelText.size().width() - painter_width * 2,
                           (int)labelText.size().height() - painter_width * 2);
          painter.translate(5, 0);
          labelText.drawContents(&painter);
        }
      painter.end();
    }
}

void Tooltip::move_to_new_data()
{
  QPoint center_point;
  char c_info[BUFSIZ];
  char c_style[BUFSIZ];
  double fontfactor;

  offset_y = 0;

  fontfactor = (qSqrt(parentWidget()->width()) + qSqrt(parentWidget()->height())) / 45;
  std::snprintf(c_style, BUFSIZ, style, (int)(11 * fontfactor), (int)(11 * fontfactor));
  std::string style_str(c_style);
  labelText.setDefaultStyleSheet(style_str.c_str());
  if (strcmp(data->label, "") == 0)
    {
      std::snprintf(c_info, BUFSIZ, tooltipTemplate_nolabel, data->xlabel, qRound(data->x * 100) / 100., data->ylabel,
                    qRound(data->y * 100) / 100.);
    }
  else
    {
      std::snprintf(c_info, BUFSIZ, tooltipTemplate, data->label, data->xlabel, qRound(data->x * 100) / 100.,
                    data->ylabel, qRound(data->y * 100) / 100.);
    }
  std::string info_str(c_info);
  labelText.setHtml(info_str.c_str());

  resize((int)(labelText.size().width() + 5), (int)labelText.size().height());

  center_point = QPoint(data->x_px, data->y_px - height() / 2);

  if (center_point.y() < 0)
    { // top of the window
      center_point.setY(0);
      offset_y = data->y_px - height() / 2;
    }
  if (center_point.y() + height() > parentWidget()->height())
    { // bottom of the window
      center_point.setY(parentWidget()->height() - this->height());
      offset_y = data->y_px - parentWidget()->height() + height() / 2;
    }
  if (data->x_px > parentWidget()->width() / 2)
    { // right half of the window
      center_point.setX(center_point.x() - width());
    }
  move(center_point);
}

void Tooltip::updateData(grm_tooltip_info_t *newData)
{
  if (data == nullptr || current_data_is_different_from(*newData))
    {
      data = newData;
    }
  if (data && data->x_px >= 0 && data->y_px >= 0)
    {
      move_to_new_data();
      if (isHidden())
        {
          show();
        }
      else
        {
          update();
        }
    }
  else
    {
      hide();
    }
}

bool Tooltip::current_data_is_different_from(const grm_tooltip_info_t &new_data)
{
  // Comparator that irgnores x and y values of grm_tooltip_info_t
  return !(data->x_px == new_data.x_px && data->y_px == new_data.y_px && data->label == new_data.label &&
           data->xlabel == new_data.xlabel && data->ylabel == new_data.ylabel);
}

Tooltip::~Tooltip() = default;
