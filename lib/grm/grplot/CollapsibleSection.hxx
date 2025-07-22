#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>

class CollapsibleSection : public QWidget
{
  Q_OBJECT
private:
  QGridLayout main_layout;
  QToolButton toggle_button;
  QFrame header_line;
  QParallelAnimationGroup toggle_animation;
  QScrollArea content_area;
  int animation_duration{100};

public:
  explicit CollapsibleSection(const QString &title = "", const int animation_duration = 100, QWidget *parent = nullptr);
  ~CollapsibleSection() override;
  void setContentLayout(QLayout &content_layout);
};