#ifndef COLORPICKERMIXED_HXX
#define COLORPICKERMIXED_HXX

#include <QSlider>
#include <QLineEdit>
#include <QDialog>
#include "grm.h"
class ColorPickerMixed;
#include "../GRPlotWidget.hxx"

class ColorPickerMixed : public QDialog
{
  Q_OBJECT
public:
  explicit ColorPickerMixed(GRPlotWidget *widget, QDialog *parent = nullptr);
  void start(const std::shared_ptr<GRM::Element> &element, const std::string &attribute_name, const int index);

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
  QLineEdit *red_value, *green_value, *blue_value, *index_value;
  QLabel *result;
  QList<QSlider *> sliders;
  QSlider *last_used_slider;
  bool triggered_by_index = false, triggered_by_text = false;

  void colorIndexHelper(const std::shared_ptr<GRM::Element> &plot_elem, int current_index, QGridLayout *grid_layout,
                        int max_index, int index_name_start);
  void resetLastSlider();
};
#endif // COLORPICKERMIXED_HXX
