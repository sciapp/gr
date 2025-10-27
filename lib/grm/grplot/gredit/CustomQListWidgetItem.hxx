#ifndef CUSTOMQLISTWIDGETITEM_HXX
#define CUSTOMQLISTWIDGETITEM_HXX
#include "SelectionListWidget.hxx"


#include <QListWidgetItem>
#include <QListWidget>
class CustomTreeWidgetItem;
#include "../GRPlotWidget.hxx"


class CustomQListWidgetItem : public QListWidgetItem
{
public:
  explicit CustomQListWidgetItem(const QString &text, const std::shared_ptr<GRM::Element> &selection,
                                 QListWidget *parent);

  std::shared_ptr<GRM::Element> getElementRef();

private:
  std::shared_ptr<GRM::Element> element_ref;
};


#endif // CUSTOMQLISTWIDGETITEM_HXX
