#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QTableWidget>
#include <utility>

class TableWidget;
#include "../grplot_widget.hxx"

class TableWidget : public QTableWidget
{
  Q_OBJECT
public:
  explicit TableWidget(GRPlotWidget *widget, QWidget *parent = nullptr);
  std::map<std::string, std::list<std::string>> extractContextNames(const std::shared_ptr<GRM::Context> context);
  void updateData(std::shared_ptr<GRM::Context> context);
  std::vector<std::string> getContextNames();

private slots:
  void applyTableChanges(int row, int column);
  void showUsagesOfContext(int row, int column);

private:
  GRPlotWidget *grplot_widget;
  std::shared_ptr<GRM::Context> context;
  std::vector<std::string> context_names;
  std::vector<std::string> context_attributes;
  std::vector<Bounding_object> referenced_attributes;
  int col_num;
};


#endif // TABLEWIDGET_H
