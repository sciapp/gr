#include "SelectionListWidget.hxx"

SelectionListWidget::SelectionListWidget(GRPlotWidget *widget, QWidget *parent) : QListWidget(parent)
{
  grplot_widget = widget;
}

void SelectionListWidget::updateSelectionList(std::vector<std::shared_ptr<GRM::Element>> multiple_selections)
{
  this->clear();
  for (const auto &selection : multiple_selections)
    {
      auto item = new CustomQListWidgetItem(QString(selection->localName().c_str()), selection, this);
      item->setCheckState(Qt::Checked);
      this->addItem(item);
    }
}
