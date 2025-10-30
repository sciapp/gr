#ifndef SELECTIONLISTWIDGET_HXX
#define SELECTIONLISTWIDGET_HXX

#include <QListWidget>
class SelectionListWidget;
#include "../grplotWidget.hxx"
#include "CustomQListWidgetItem.hxx"


class SelectionListWidget : public QListWidget
{
  Q_OBJECT
public:
  explicit SelectionListWidget(GRPlotWidget *widget, QWidget *parent = nullptr);
  void updateSelectionList(std::vector<std::shared_ptr<GRM::Element>> multiple_selections);

private:
  GRPlotWidget *grplot_widget;
};


#endif // SELECTIONLISTWIDGET_HXX
