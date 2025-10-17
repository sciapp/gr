#ifndef LAYOUT_HXX_INCLUDED
#define LAYOUT_HXX_INCLUDED

#include <vector>
#include <unordered_map>
#include <string>
#include <grm/dom_render/graphics_tree/element.hxx>
#include <grm/util.h>

#include "args.h"
#include "error.h"

namespace GRM
{

class GRM_EXPORT Slice
{
public:
  Slice(int row_start, int row_stop, int col_start, int col_stop);
  Slice *copy();
  bool isPositive();
  bool isForward();

  int row_start;
  int row_stop;
  int col_start;
  int col_stop;
  friend class Grid;
};

class GRM_EXPORT GridElement
{
public:
  GridElement();
  GridElement(double abs_height, double abs_width, int fit_parents_height, int fit_parents_width,
              double relative_height, double relative_width, double aspect_ratio);
  virtual ~GridElement();
  virtual void finalizePlot();
  virtual bool isGrid();
  void setPlot(double x1, double x2, double y1, double y2);
  void setAbsHeight(double height);
  void setRelativeHeight(double height);
  void setAbsWidth(double width);
  void setRelativeWidth(double width);
  void setAspectRatio(double ar);
  void setFitParentsHeight(bool fit_parents_height);
  void setFitParentsWidth(bool fit_parents_width);
  double *getPlot();
  grm_args_t *plot_args = nullptr;

  double *plot;

  double abs_height = -1;
  double abs_width = -1;
  int fit_parents_height = 0;
  int fit_parents_width = 1;
  double relative_height = -1;
  double relative_width = -1;
  double aspect_ratio = -1;

  int width_set = 0;
  int height_set = 0;
  int ar_set = 0;
  int plot_set = 0;

  int finalized = 0;


  friend class Grid;
  std::shared_ptr<GRM::Element> element_in_dom = nullptr;
};

class GRM_EXPORT Grid : public GridElement
{

public:
  Grid(int n_rows, int n_cols);
  Grid(int n_rows, int n_cols, double abs_height, double abs_width, int fit_parents_height, int fit_parents_width,
       double relative_height, double relative_width, double aspect_ratio);
  ~Grid();
  void setElement(int row, int col, GridElement *element);
  void setElement(int row, int col, grm_args_t *args);
  void setElement(Slice *slice, GridElement *element);
  void setElement(Slice *slice, grm_args_t *args);
  void ensureCellIsGrid(int row, int col);
  void ensureCellsAreGrid(Slice *slice);
  GridElement *getElement(int row, int col) const;
  void printGrid() const;
  virtual void finalizePlot() override;
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
  std::unordered_map<GridElement *, Slice *> element_to_position;
  int n_rows;
  int n_cols;
  void upsize(int nrows, int ncols);
};

} // namespace GRM

#endif /* ifndef LAYOUT_HPP_INCLUDED */
