#ifndef COLORPICKERMIXED_HXX
#define COLORPICKERMIXED_HXX

#include <QSlider>
#include <QLineEdit>
#include <QDialog>
class ColorPickerMixed;
#include "../GRPlotWidget.hxx"
#include "GRColorSlider.hxx"
#include "GRRGBPicker.hxx"

class ColorPickerMixed : public QDialog
{
  Q_OBJECT
public:
  explicit ColorPickerMixed(GRPlotWidget *widget, QDialog *parent = nullptr);
  void start(const std::shared_ptr<GRM::Element> &element, const std::string &attribute_name, const int index);

private slots:
  void reject();
  void accept();

private:
  GRPlotWidget *grplot_widget_;
  QList<GRColorSlider *> sliders_;
  GRRGBPicker *rgb_picker_;
  std::weak_ptr<GRM::Element> elem_;
  std::string attr_name_;
};

#endif // COLORPICKERMIXED_HXX
