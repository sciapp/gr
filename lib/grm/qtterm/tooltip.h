#ifndef QTTERM_TOOLTIP_H
#define QTTERM_TOOLTIP_H


#include <QWidget>
#include <QTextDocument>
#include <QPainter>
#include <QtMath>
#include "grm.h"

class Tooltip : public QWidget
{
  Q_OBJECT
public:
  explicit Tooltip(QWidget *parent = nullptr);

  ~Tooltip() override;

  void paintEvent(QPaintEvent *event) override;

  void updateData(grm_tooltip_info_t *);

private:
  grm_tooltip_info_t *data;
  QTextDocument labelText;
  int offset_y; // Used to push the tooltip into the viewport

  bool current_data_is_different_from(const grm_tooltip_info_t &new_data);

  void move_to_new_data();
};

#endif // QTTERM_TOOLTIP_H
