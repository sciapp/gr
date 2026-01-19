#include <QPropertyAnimation>
#include "CollapsibleSection.hxx"

/*
 * Code inspired from Stackoverflow user https://stackoverflow.com/users/887074/erotemic
 * Full code under:
 * https://stackoverflow.com/questions/32476006/how-to-make-an-expandable-collapsable-section-widget-in-qt/37119983#37119983
 */

CollapsibleSection::CollapsibleSection(const QString &title, const int animation_duration, QWidget *parent)
    : QWidget(parent), animation_duration(animation_duration)
{
  int row = 0;

  toggle_button.setStyleSheet("QToolButton { border: none; }");
  toggle_button.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  toggle_button.setArrowType(Qt::ArrowType::RightArrow);
  toggle_button.setText(title);
  toggle_button.setCheckable(true);
  toggle_button.setChecked(false);

  header_line.setFrameShape(QFrame::HLine);
  header_line.setFrameShadow(QFrame::Sunken);
  header_line.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  content_area.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  // so all labels are displayed completely
  // start out collapsed
  content_area.setMaximumHeight(0);
  content_area.setMinimumHeight(0);
  // let the entire widget grow and shrink with its content
  toggle_animation.addAnimation(new QPropertyAnimation(this, "minimumHeight"));
  toggle_animation.addAnimation(new QPropertyAnimation(this, "maximumHeight"));
  toggle_animation.addAnimation(new QPropertyAnimation(&content_area, "maximumHeight"));
  // don't waste space
  main_layout.setVerticalSpacing(0);
  main_layout.setContentsMargins(0, 0, 0, 0);

  main_layout.addWidget(&toggle_button, row, 0, 1, 1, Qt::AlignLeft);
  main_layout.addWidget(&header_line, row++, 2, 1, 1);
  main_layout.addWidget(&content_area, row, 0, 1, 3);
  setLayout(&main_layout);
  QObject::connect(&toggle_button, &QToolButton::clicked, [this](const bool checked) {
    toggle_button.setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
    toggle_animation.setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    toggle_animation.start();
  });
}

CollapsibleSection::~CollapsibleSection() = default;

void CollapsibleSection::setContentLayout(QLayout &content_layout, bool clicked)
{
  delete content_area.layout();
  content_area.setLayout(&content_layout);
  const auto collapsed_height = sizeHint().height() - content_area.maximumHeight();
  auto content_height = content_layout.sizeHint().height();
  for (int i = 0; i < toggle_animation.animationCount() - 1; ++i)
    {
      auto collapsible_section_animation = static_cast<QPropertyAnimation *>(toggle_animation.animationAt(i));
      collapsible_section_animation->setDuration(animation_duration);
      collapsible_section_animation->setStartValue(collapsed_height);
      collapsible_section_animation->setEndValue(collapsed_height + content_height);
    }
  auto content_animation =
      static_cast<QPropertyAnimation *>(toggle_animation.animationAt(toggle_animation.animationCount() - 1));
  content_animation->setDuration(animation_duration);
  content_animation->setStartValue(0);
  content_animation->setEndValue(content_height);
  if (clicked) toggle_button.clicked(true);
}
