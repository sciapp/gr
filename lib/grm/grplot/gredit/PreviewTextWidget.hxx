#ifndef PREVIEWTEXTWIDGET_HXX
#define PREVIEWTEXTWIDGET_HXX

#include "qtgr/grwidget.h"

class PreviewTextWidget : public GRWidget
{
public:
  PreviewTextWidget(QWidget *parent = nullptr);
  void initialize(std::string text, int scientific_format, int text_color, int width, int height);

protected:
  virtual void draw();

private:
  std::string text;
  int scientific_format, text_color;
  int width_px, height_px;
};

#endif // PREVIEWTEXTWIDGET_HXX
