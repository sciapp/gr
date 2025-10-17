#ifndef COLORPICKERRGB_HXX
#define COLORPICKERRGB_HXX

#include <QSlider>
#include <QLineEdit>
#include <QDialog>
#include "grm.h"
class ColorPickerRGB;
#include "../GRPlotWidget.hxx"

class ColorPickerRGB : public QDialog
{
  Q_OBJECT
public:
  explicit ColorPickerRGB(GRPlotWidget *widget, QDialog *parent = nullptr);
  void start(std::string attribute_name, const std::shared_ptr<GRM::Element> &element);

private slots:
  void redSliderChangeSlot();
  void greenSliderChangeSlot();
  void blueSliderChangeSlot();
  void redLineEditChangeSlot();
  void greenLineEditChangeSlot();
  void blueLineEditChangeSlot();
  void reject();
  void accept();

private:
  GRPlotWidget *grplot_widget;
  QSlider *red_slider, *green_slider, *blue_slider;
  QLabel *red_label, *green_label, *blue_label;
  QLineEdit *red_value, *green_value, *blue_value;
  QLabel *result;
};
#endif // COLORPICKERRGB_HXX
