#ifndef GR_RGB_PICKER_HXX_INCLUDED
#define GR_RGB_PICKER_HXX_INCLUDED

#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QWidget>

static constexpr double TOLERATED_COLOR_DISTANCE_DEFAULT = 1.0 / 255.0;

class GRRGBPicker : public QWidget
{
  Q_OBJECT
public:
  explicit GRRGBPicker(QWidget *parent = nullptr, double tolerated_color_distance = TOLERATED_COLOR_DISTANCE_DEFAULT);
  explicit GRRGBPicker(int gr_color, QWidget *parent = nullptr,
                       double tolerated_color_distance = TOLERATED_COLOR_DISTANCE_DEFAULT);
  explicit GRRGBPicker(const QColor &color, QWidget *parent = nullptr,
                       double tolerated_color_distance = TOLERATED_COLOR_DISTANCE_DEFAULT);

  int grColor() const;
  QColor color() const;

signals:
  void colorChanged(int gr_color, const QColor &color);

public slots:
  bool setColor(int gr_color);
  bool setColor(const QColor &color);

private:
  void initUI(int gr_color = -1, const QColor &color = QColor());
  bool setColorHelper(int gr_color, const QColor &color);

  QLineEdit *red_value_, *green_value_, *blue_value_, *gr_color_index_value_;
  QSlider *red_slider_, *green_slider_, *blue_slider_;
  QLabel *mixed_color_;

  int gr_color_ = -1;
  QColor color_;
  double tolerated_color_distance_;
};

#endif /* ifndef GR_RGB_PICKER_HXX_INCLUDED */
