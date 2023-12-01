#ifndef LAYOUT_HXX_INCLUDED
#define LAYOUT_HXX_INCLUDED

#include <vector>
#include <unordered_map>
#include <string>
#include <grm/dom_render/graphics_tree/Element.hxx>
#include <grm/util.h>

#include "args.h"
#include "error.h"

namespace grm
{

class EXPORT Slice
{
public:
  Slice(int rowStart, int rowStop, int colStart, int colStop);
  Slice *copy();
  bool isPositive();
  bool isForward();

  int rowStart;
  int rowStop;
  int colStart;
  int colStop;
  friend class Grid;
};

class EXPORT GridElement
{
public:
  GridElement();
  GridElement(double absHeight, double absWidth, int absHeightPxl, int absWidthPxl, int fitParentsHeight,
              int fitParentsWidth, double relativeHeight, double relativeWidth, double aspectRatio);
  virtual ~GridElement();
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
  grm_args_t *subplot_args = nullptr;

  double *subplot;

  double absHeight = -1;
  double absWidth = -1;
  int absHeightPxl = -1;
  int absWidthPxl = -1;
  int fitParentsHeight = 0;
  int fitParentsWidth = 1;
  double relativeHeight = -1;
  double relativeWidth = -1;
  double aspectRatio = -1;

  int widthSet = 0;
  int heightSet = 0;
  int arSet = 0;
  int subplotSet = 0;

  int finalized = 0;


  friend class Grid;
  std::shared_ptr<GRM::Element> elementInDOM = nullptr;
};

class EXPORT Grid : public GridElement
{

public:
  Grid(int nrows, int ncols);
  Grid(int nrows, int ncols, double absHeight, double absWidth, int absHeightPxl, int absWidthPxl, int fitParentsHeight,
       int fitParentsWidth, double relativeHeight, double relativeWidth, double aspectRatio);
  ~Grid();
  void setElement(int row, int col, GridElement *element);
  void setElement(int row, int col, grm_args_t *args);
  void setElement(Slice *slice, GridElement *element);
  void setElement(Slice *slice, grm_args_t *args);
  void ensureCellIsGrid(int row, int col);
  void ensureCellsAreGrid(Slice *slice);
  GridElement *getElement(int row, int col) const;
  void printGrid() const;
  virtual void finalizeSubplot() override;
  bool isGrid() override;
  void trim();
  int getColSpan(GridElement *element);
  int getRowSpan(GridElement *element);
  int getNRows() const;
  int getNCols() const;
  bool isRowsEmpty() const;
  const std::unordered_map<GridElement *, Slice *> &getElementToPosition();
  const std::vector<std::vector<GridElement *>> &getRows();

private:
  std::vector<std::vector<GridElement *>> rows;
  std::unordered_map<GridElement *, Slice *> elementToPosition;
  int nrows;
  int ncols;
  void upsize(int nrows, int ncols);
};

} // namespace grm

#endif /* ifndef LAYOUT_HPP_INCLUDED */
