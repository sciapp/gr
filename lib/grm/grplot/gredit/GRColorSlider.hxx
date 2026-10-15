#ifndef GR_COLOR_SLIDER_HXX_INCLUDED
#define GR_COLOR_SLIDER_HXX_INCLUDED

#include <QImage>
#include <QWidget>

class GRColorSlider : public QWidget
{
  Q_OBJECT
public:
  explicit GRColorSlider(QWidget *parent = nullptr);
  GRColorSlider(unsigned int gr_color_range_min, unsigned int gr_color_range_max, QWidget *parent = nullptr);
  explicit GRColorSlider(const std::pair<unsigned int, unsigned int> &gr_color_range, QWidget *parent = nullptr);

  void updateColors();

  int grColor() const;
  QColor color() const;

signals:
  void grColorChanged(int gr_color);

public slots:
  bool setGRColor(int gr_color);

protected:
  void mouseMoveEvent(QMouseEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void paintEvent(QPaintEvent *) override;
  QSize sizeHint() const override;

private:
  bool isSelectedColorDark();
  void updateQImageColors();
  void updateColorSelectionFromMouse(int mouse_x);

  std::pair<unsigned int, unsigned int> gr_color_range_;
  int selected_gr_color_ = -1;
  QImage gr_colors_;
  bool mouse_left_pressed_ = false;

  static constexpr double GROOVE_PADDING = 0.2;   // relative to the total height of the widget
  static constexpr unsigned int HANDLE_WIDTH = 3; // in logical pixels
};

#endif /* ifndef GR_COLOR_SLIDER_HXX_INCLUDED */
