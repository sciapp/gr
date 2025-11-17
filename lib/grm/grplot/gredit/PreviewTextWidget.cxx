#include "PreviewTextWidget.hxx"
#include "gr.h"
#include "grm.h"

PreviewTextWidget::PreviewTextWidget(QWidget *parent) : GRWidget(parent) {}

void PreviewTextWidget::initialize(std::string text, int scientific_format, int text_color, int font_precision,
                                   int width, int height)
{
  this->text = text;
  this->scientific_format = scientific_format;
  this->text_color = text_color;
  this->width_px = width;
  this->height_px = height;
  this->font_precision = font_precision;

  this->setFixedWidth(width + (width < 100 ? 20 : 0));
  this->setFixedHeight(height + (height < 100 ? 20 : 0));
}

void PreviewTextWidget::draw()
{
  double x = 0.0, y = 0.01;
  double vp[4] = {0.0, 1.0, 0.0, 1.0};
  double ws_window[4] = {0.0, 1.0, 0.0, 1.0};
  double dpm[2];
  double display_metric_width, display_metric_height;
  int display_pixel_width, display_pixel_height;

  gr_setwindow(vp[0], vp[1], vp[2], vp[3]);

  auto aspect_ratio = double(this->width_px) / double(this->height_px);
  if (aspect_ratio > 1)
    {
      vp[2] /= aspect_ratio;
      vp[3] /= aspect_ratio;
    }
  else
    {
      vp[0] *= aspect_ratio;
      vp[1] *= aspect_ratio;
    }

  gr_inqdspsize(&display_metric_width, &display_metric_height, &display_pixel_width, &display_pixel_height);
  dpm[0] = display_pixel_width / display_metric_width;
  dpm[1] = display_pixel_height / display_metric_height;

  gr_setwsviewport(0.0, (this->width_px + (this->width_px < 100 ? 20 : 0)) / dpm[0], 0.0,
                   (this->height_px + (this->height_px < 100 ? 20 : 0)) / dpm[1]);
  gr_setwswindow(ws_window[0], ws_window[1] / (600 / (this->width_px + (this->width_px < 100 ? 5 : 0))), ws_window[2],
                 ws_window[3] / (450 / (this->height_px + (this->height_px < 100 ? 5 : 0))));
  gr_setviewport(vp[0], vp[1], vp[2], vp[3]);

  gr_settextencoding(301);
  gr_settextfontprec(232, font_precision);
  gr_setscientificformat(scientific_format);
  gr_settextcolorind(this->text_color);

  const char *text_c = this->text.c_str();
  if (this->scientific_format == 2)
    {
      gr_textext(x, y, (char *)text_c);
    }
  else if (this->scientific_format == 3)
    {
      gr_mathtex(x, y, (char *)text_c);
    }
  else
    {
      gr_text(x, y, (char *)text_c);
    }
}