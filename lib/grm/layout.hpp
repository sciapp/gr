#ifndef LAYOUT_HPP_INCLUDED
#define LAYOUT_HPP_INCLUDED

#include <vector>
#include <unordered_map>

#include "args.h"

class Slice
{
public:
  Slice(int rowStart, int rowStop, int colStart, int colStop);
  int rowStart;
  int rowStop;
  int colStart;
  int colStop;
  Slice *copy();
};

class GridElement
{
public:
  GridElement();
  virtual void finalizeSubplot();
  virtual bool isGrid();
  void setSubplot(double x1, double x2, double y1, double y2);
  void setAbsHeight(double height);
  void setAbsHeightPxl(int height);
  void setRelativeHeight(double height);
  void setAbsWidth(double width);
  void setAbsWidthPxl(int width);
  void setRelativeWidth(double width);
  void setAspectRatio(double ar);
  void setFitParentsHeight(bool fitParentsHeight);
  void setFitParentsWidth(bool fitParentsWidth);
  double *getSubplot();

  double *subplot;

  double absHeight = -1;
  double absWidth = -1;
  int absHeightPxl = -1;
  int absWidthPxl = -1;
  int fitParentsHeight = 0;
  int fitParentsWidth = 1;
  double relativeHeight = -1;
  double relativeWidth = -1;
  double aspectRatio;

  int widthSet = 0;
  int heightSet = 0;
  int arSet = 0;
  int subplotSet = 0;

  int finalized = 0;

  grm_args_t *subplot_args = nullptr;
};

class Grid : public GridElement
{
public:
  Grid(int nrows, int ncols);
  ~Grid();
  void setElement(int row, int col, GridElement *element);
  void setElement(int row, int col, grm_args_t *args);
  void setElement(Slice *slice, GridElement *element);
  void setElement(Slice *slice, grm_args_t *args);
  void ensureCellIsGrid(int row, int col);
  void ensureCellsAreGrid(Slice *slice);
  GridElement *getElement(int row, int col) const;
  void printGrid() const;
  virtual void finalizeSubplot();
  bool isGrid() override;
  void trim();
  int getColSpan(GridElement *element);
  int getRowSpan(GridElement *element);

private:
  std::vector<std::vector<GridElement *>> rows;
  std::unordered_map<GridElement *, Slice *> elementToPosition;
  int nrows;
  int ncols;
  void upsize(int nrows, int ncols);
};

#endif /* ifndef LAYOUT_HPP_INCLUDED */
