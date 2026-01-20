#ifndef GR_CREATOR_HXX
#define GR_CREATOR_HXX

#include <optional>

#include "graphics_tree/document.hxx"
#include <grm/dom_render/context.hxx>
#include <grm/dom_render/graphics_tree/element.hxx>
#include <grm/dom_render/graphics_tree/document.hxx>
#include "grm/layout.hxx"


enum class CoordinateSpace
{
  WC,
  NDC
};

namespace GRM
{

class GRM_EXPORT Creator : public Document
{

public:
  /* ------------------------------- create functions ----------------------------------------------------------------*/

  static std::shared_ptr<Creator> createCreator(std::shared_ptr<Context> context);

  std::shared_ptr<Element> createPlot(int plot_id, const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createCentralRegion(const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createPolymarker(const std::string &x_key, std::optional<std::vector<double>> x,
                                            const std::string &y_key, std::optional<std::vector<double>> y,
                                            const std::shared_ptr<Context> &ext_context = nullptr, int marker_type = 0,
                                            double marker_size = 0.0, int marker_color_ind = 0,
                                            const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createPolymarker(double x, double y, int marker_type = 0, double marker_size = 0.0,
                                            int marker_colorind = 0,
                                            const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createPolyline(const std::string &x_key, std::optional<std::vector<double>> x,
                                          const std::string &y_key, std::optional<std::vector<double>> y,
                                          const std::shared_ptr<Context> &ext_context = nullptr, int line_type = 0,
                                          double line_width = 0.0, int line_colorind = 0,
                                          const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createPolyline(double x1, double x2, double y1, double y2, int line_type = 0,
                                          double line_width = 0.0, int line_colorind = 0,
                                          const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createText(double x, double y, const std::string &text,
                                      CoordinateSpace space = CoordinateSpace::NDC,
                                      const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createFillArea(const std::string &x_key, std::optional<std::vector<double>> x,
                                          const std::string &y_key, std::optional<std::vector<double>> y,
                                          const std::shared_ptr<Context> &ext_context = nullptr, int fill_int_style = 0,
                                          int fill_style = 0, int fill_color_ind = -1,
                                          const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createCellArray(double xmin, double xmax, double ymin, double ymax, int dimx, int dimy,
                                           int scol, int srow, int ncol, int nrow, const std::string &color_key,
                                           std::optional<std::vector<int>> color,
                                           const std::shared_ptr<Context> &ext_context = nullptr,
                                           const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createNonUniformPolarCellArray(
      double x_org, double y_org, const std::string &theta_key, std::optional<std::vector<double>> theta,
      const std::string &r_key, std::optional<std::vector<double>> r, int dim_theta, int dim_r, int s_col, int s_row,
      int n_col, int n_row, const std::string &color_key, std::optional<std::vector<int>> color,
      const std::shared_ptr<Context> &ext_context = nullptr, const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createPolarCellArray(double x_org, double y_org, double theta_min, double theta_max,
                                                double r_min, double r_max, int dim_theta, int dim_r, int s_col,
                                                int s_row, int n_col, int n_row, const std::string &color_key,
                                                std::optional<std::vector<int>> color,
                                                const std::shared_ptr<Context> &ext_context = nullptr,
                                                const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createEmptyAxis(const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createAxis(double min_val, double max_val, double tick, double org, double pos,
                                      int major_count, int num_ticks, int num_tick_labels, double tick_size,
                                      int tick_orientation, double label_pos,
                                      const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createTickGroup(int is_major, const std::string &tick_label, double value, double width,
                                           const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createTick(int is_major, double value,
                                      const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createGridLine(int is_major, double value,
                                          const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createLegend(const std::shared_ptr<Element> &ext_element = nullptr,
                                        const std::shared_ptr<Context> &ext_context = nullptr);
  std::shared_ptr<Element> createPieSegment(const double start_angle, const double end_angle, const std::string &text,
                                            const int color_index,
                                            const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createBar(const double x1, const double x2, const double y1, const double y2,
                                     const int bar_color_index, const int edge_color_index,
                                     const std::string &bar_color_rgb = "", const std::string &edge_color_rgb = "",
                                     const double linewidth = -1, const std::string &text = "",
                                     const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createSeries(const std::string &name);
  std::shared_ptr<Element> createDrawImage(double xmin, double ymin, double xmax, double ymax, int width, int height,
                                           const std::string &data_key, std::optional<std::vector<int>> data, int model,
                                           const std::shared_ptr<Context> &ext_context = nullptr,
                                           const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createDrawArc(double xmin, double xmax, double ymin, double ymax, double start_angle,
                                         double end_angle, const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createFillArc(double xmin, double xmax, double ymin, double ymax, double a1, double a2,
                                         int fill_int_style = 0, int fill_style = 0, int fill_color_ind = -1,
                                         const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createDrawRect(double xmin, double xmax, double ymin, double ymax,
                                          const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createFillRect(double xmin, double xmax, double ymin, double ymax, int fill_int_style = 0,
                                          int fill_style = 0, int fill_color_ind = -1,
                                          const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createQuiver(const std::string &x_key, std::optional<std::vector<double>> x,
                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                        const std::string &u_key, std::optional<std::vector<double>> u,
                                        const std::string &v_key, std::optional<std::vector<double>> v, int colored,
                                        const std::shared_ptr<Context> &ext_context = nullptr);
  std::shared_ptr<Element> createHexbin(const std::string &x_key, std::optional<std::vector<double>> x,
                                        const std::string &y_key, std::optional<std::vector<double>> y,
                                        const std::shared_ptr<Context> &ext_context = nullptr);
  std::shared_ptr<Element> createColorbar(unsigned int num_color_values,
                                          const std::shared_ptr<Context> &ext_context = nullptr,
                                          const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createNonUniformCellArray(const std::string &x_key, std::optional<std::vector<double>> x,
                                                     const std::string &y_key, std::optional<std::vector<double>> y,
                                                     int dimx, int dimy, int scol, int srow, int ncol, int nrow,
                                                     const std::string &color_key,
                                                     std::optional<std::vector<int>> color,
                                                     const std::shared_ptr<Context> &ext_context = nullptr,
                                                     const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createGrid3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org,
                                        double z_org, int major_x, int major_y, int major_z,
                                        const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createEmptyGrid3d(bool x_grid, bool y_grid, bool z_grid,
                                             const std::shared_ptr<Element> &ext_element = nullptr);

  std::shared_ptr<Element> createAxes3d(double x_tick, double y_tick, double z_tick, double x_org, double y_org,
                                        double z_org, int major_x, int major_y, int major_z, int tick_orientation,
                                        const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createEmptyAxes3d(int tick_orientation,
                                             const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createPolyline3d(const std::string &x_key, std::optional<std::vector<double>> x,
                                            const std::string &y_key, std::optional<std::vector<double>> y,
                                            const std::string &z_key, std::optional<std::vector<double>> z,
                                            const std::shared_ptr<Context> &ext_context = nullptr,
                                            const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createPolymarker3d(const std::string &x_key, std::optional<std::vector<double>> x,
                                              const std::string &y_key, std::optional<std::vector<double>> y,
                                              const std::string &z_key, std::optional<std::vector<double>> z,
                                              const std::shared_ptr<Context> &ext_context = nullptr,
                                              const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createDrawGraphics(const std::string &data_key, std::optional<std::vector<int>> data,
                                              const std::shared_ptr<Context> &ext_context = nullptr,
                                              const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createTriSurface(const std::string &px_key, std::optional<std::vector<double>> px,
                                            const std::string &py_key, std::optional<std::vector<double>> py,
                                            const std::string &pz_key, std::optional<std::vector<double>> pz,
                                            const std::shared_ptr<Context> &ext_context = nullptr);
  std::shared_ptr<Element> createTitles3d(const std::string &x_label, const std::string &y_label,
                                          const std::string &z_label,
                                          const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createLayoutGrid(const Grid &grid);
  std::shared_ptr<Element> createLayoutGridElement(const GridElement &grid_element, const Slice &slice);
  std::shared_ptr<Element> createPanzoom(double x, double y, double xzoom, double yzoom);
  std::shared_ptr<Element> createPolarBar(double count, int class_nr,
                                          const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createErrorBar(double error_bar_x, double error_bar_y_min, double error_bar_y_max,
                                          int color_error_bar, const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createIntegral(double int_lim_low, double int_lim_high,
                                          const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createSideRegion(const std::string &location,
                                            const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createTextRegion(const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createSidePlotRegion(const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createRadialAxes(const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createThetaAxes(const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createAngleLine(double x, double y, const std::string &angle_label,
                                           const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createArcGridLine(double value, const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createOverlay(const std::shared_ptr<Element> &ext_element = nullptr);
  std::shared_ptr<Element> createOverlayElement(double x, double y, std::string type,
                                                const std::shared_ptr<Element> &ext_element = nullptr);
  void createColormap(int x, int y, int size, std::vector<int> &colormap);

  /* ---------------------------------- get functions ----------------------------------------------------------------*/

  int getAxisId();

private:
  Creator(std::shared_ptr<Context> context);
  std::shared_ptr<Context> context;
};

} // namespace GRM

#endif // GR_CREATOR_HXX
