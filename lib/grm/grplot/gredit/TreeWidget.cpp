#include <QTreeWidgetItem>
#include <queue>
#include <QHeaderView>
#include "TreeWidget.h"
#include "CustomTreeWidgetItem.h"


TreeWidget::TreeWidget(QWidget *parent) : QTreeWidget(parent)
{
  this->setWindowTitle("DOM-Tree");
  QStringList labels;
  labels << tr("Object") << tr("Value");

  this->setHeaderLabels(labels);
  this->setColumnCount(2);
  this->header()->setSectionResizeMode(QHeaderView::Stretch);
  //    this->setHeaderHidden(true);
  QString style = "QTreeWidget::item:!selected "
                  "{ "
                  "border: 1px solid gray; "
                  "border-left: none; "
                  "border-top: none; "
                  "border-bottom: 1px solid gray;"
                  "}"
                  "QTreeWidget::item:selected {}";
  this->setStyleSheet(style);
}

void TreeWidget::updateData(std::shared_ptr<GRM::Element> ref)
{
  this->clear();
  plotTree = new QTreeWidgetItem(this);
  plotTree->setText(0, tr("Plot"));
  plotTree->setExpanded(true);

  for (const auto &cur_attr : ref->getAttributeNames())
    {
      QTreeWidgetItem *item = new CustomTreeWidgetItem(plotTree, ref);
      item->setText(0, tr(cur_attr.c_str()));
      item->setText(1, tr(static_cast<std::string>(ref->getAttribute(cur_attr)).c_str()));
      item->setExpanded(true);
    }
  for (const auto &cur_elem : ref->children())
    {
      updateDataRecursion(cur_elem, plotTree);
    }

  connect(this->model(), &QAbstractItemModel::dataChanged,
          [](const QModelIndex &index, const QModelIndex &, const QVector<int> &roles) {
            // TODO: Get current element and change value to ref
            // This will be the return-channel from the TreeWidget to the DOM-Object
            //


            //            printf("(%d %d)", index.column(), index.row());

            //            for (auto i : roles)
            //              {
            //                printf("%d ", i);
            //              }
            //            printf("\n");
          });
}

void TreeWidget::updateDataRecursion(std::shared_ptr<GRM::Element> ref, QTreeWidgetItem *parent)
{
  bool skip = false;
  QTreeWidgetItem *item = new QTreeWidgetItem(parent);
  item->setText(0, tr(ref->localName().c_str()));
  std::string blacklist[] = {"name", "_bbox_xmin", "_bbox_xmax", "_bbox_ymin", "_bbox_ymax", "_bbox_id"};
  item->setText(1, tr(static_cast<std::string>(ref->getAttribute(blacklist[0])).c_str()));
  for (const auto &cur_attr : ref->getAttributeNames())
    {
      skip = false;
      for (auto const &n : blacklist)
        {
          if (cur_attr == n)
            {
              skip = true;
            }
        }
      if (skip) continue;
      QTreeWidgetItem *attributes = new CustomTreeWidgetItem(item, ref);
      attributes->setText(0, tr(cur_attr.c_str()));
      attributes->setText(1, tr(static_cast<std::string>(ref->getAttribute(cur_attr)).c_str()));
      //        attributes->setExpanded(true);
    }
  item->setExpanded(true);
  for (const auto &cur_elem : ref->children())
    {
      updateDataRecursion(cur_elem, item);
    }
}
