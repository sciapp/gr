#ifndef LAYOUT_HPP_INCLUDED
#define LAYOUT_HPP_INCLUDED

#include <vector>
#include <unordered_map>

class Slice
{
public:
  Slice(int rowStart, int rowStop, int colStart, int colStop);
  int rowStart_;
  int rowStop_;
  int colStart_;
  int colStop_;
};

class GridElement
{
public:
  GridElement();
  virtual void finalizeSubplot();
  double *subplot;
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
};

class Grid : public GridElement
{
public:
  Grid(int nrows, int ncols);
  ~Grid();
  void setElement(int row, int col, GridElement *element);
  GridElement *getElement(int row, int col) const;
  void printGrid() const;
  virtual void finalizeSubplot();
  void upsize(int nrows, int ncols);

private:
  std::vector<std::vector<GridElement *>> rows;
  std::unordered_map<GridElement *, Slice *> elementToPosition;
  int nrows;
  int ncols;
};

#endif /* ifndef LAYOUT_HPP_INCLUDED */
