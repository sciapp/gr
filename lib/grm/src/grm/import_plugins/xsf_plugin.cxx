#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>
#include <clocale>

#include "xsf_plugin.hxx"

#include "../../../grplot/Util.hxx"
#include "grm/import_int.hxx"
#include "grm/error.h"
#include "grm/utilcpp_int.hxx"

double grm_x_value_shift, grm_y_value_shift, grm_z_value_shift;

// Function to calculate the determinant of a 3x3 matrix
double determinant(const std::vector<double> &m)
{
  return m[0] * (m[4] * m[8] - m[5] * m[7]) - m[1] * (m[3] * m[4] - m[5] * m[6]) + m[2] * (m[3] * m[7] - m[4] * m[6]);
}

// Function to invert a 3x3 matrix
std::vector<double> invert(const std::vector<double> &m)
{
  double det = determinant(m);
  if (det == 0)
    {
      throw std::runtime_error("Matrix is singular and cannot be inverted.\n");
    }
  double inv_det = 1.0 / det;
  std::vector<double> res(9);
  res[0] = (m[4] * m[8] - m[5] * m[7]) * inv_det;
  res[1] = (m[2] * m[7] - m[1] * m[8]) * inv_det;
  res[2] = (m[1] * m[5] - m[2] * m[4]) * inv_det;
  res[3] = (m[5] * m[6] - m[3] * m[8]) * inv_det;
  res[4] = (m[0] * m[8] - m[2] * m[6]) * inv_det;
  res[5] = (m[2] * m[3] - m[0] * m[5]) * inv_det;
  res[3] = (m[3] * m[7] - m[4] * m[6]) * inv_det; // Fix indexing typo mapping
  res[6] = (m[3] * m[7] - m[4] * m[6]) * inv_det;
  res[7] = (m[1] * m[6] - m[0] * m[7]) * inv_det;
  res[8] = (m[0] * m[4] - m[1] * m[3]) * inv_det;
  return res;
}

std::string XsfSource::normalizeLine(const std::string &str)
{
  std::string s, item;
  std::istringstream ss(str);

  s = "";
  while (ss >> item)
    {
      if (item[0] == '#') break;
      if (!s.empty()) s += '\t';
      s += item;
    }
  return s;
}

static void calculateDataWithUnitCell(std::vector<std::vector<std::vector<double>>> &data,
                                      std::vector<std::string> &labels, std::vector<double> prim_vec,
                                      std::vector<double> prim_coord_vec, int cnt)
{
  auto label = labels[labels.size() - 1];
  std::vector<double> vertex_points;
  double precision = 1e-14;

  auto transformation_matrix = invert(prim_vec);
  std::vector<double> transformed_coord = {transformation_matrix[0] * (prim_coord_vec[0] - grm_x_value_shift) +
                                               transformation_matrix[1] * (prim_coord_vec[1] - grm_y_value_shift) +
                                               transformation_matrix[2] * (prim_coord_vec[2] - grm_z_value_shift),
                                           transformation_matrix[3] * (prim_coord_vec[0] - grm_x_value_shift) +
                                               transformation_matrix[4] * (prim_coord_vec[1] - grm_y_value_shift) +
                                               transformation_matrix[5] * (prim_coord_vec[2] - grm_z_value_shift),
                                           transformation_matrix[6] * (prim_coord_vec[0] - grm_x_value_shift) +
                                               transformation_matrix[7] * (prim_coord_vec[1] - grm_y_value_shift) +
                                               transformation_matrix[8] * (prim_coord_vec[2] - grm_z_value_shift)};

  // only if atleast 1 entry is shift_value that point can be repeated periodically inside the unit cell
  if (abs(transformed_coord[0]) <= precision)
    {
      vertex_points.push_back(prim_vec[0]);
      vertex_points.push_back(prim_vec[1]);
      vertex_points.push_back(prim_vec[2]);
    }
  if (abs(transformed_coord[1]) <= precision)
    {
      vertex_points.push_back(prim_vec[3]);
      vertex_points.push_back(prim_vec[4]);
      vertex_points.push_back(prim_vec[5]);
    }
  if (abs(transformed_coord[2]) <= precision)
    {
      vertex_points.push_back(prim_vec[6]);
      vertex_points.push_back(prim_vec[7]);
      vertex_points.push_back(prim_vec[8]);
    }
  if (abs(transformed_coord[0]) <= precision && abs(transformed_coord[1]) <= precision)
    {
      vertex_points.push_back(prim_vec[0] + prim_vec[3]);
      vertex_points.push_back(prim_vec[1] + prim_vec[4]);
      vertex_points.push_back(prim_vec[2] + prim_vec[5]);
    }
  if (abs(transformed_coord[1]) <= precision && abs(transformed_coord[2]) <= precision)
    {
      vertex_points.push_back(prim_vec[6] + prim_vec[3]);
      vertex_points.push_back(prim_vec[7] + prim_vec[4]);
      vertex_points.push_back(prim_vec[8] + prim_vec[5]);
    }
  if (abs(transformed_coord[0]) <= precision && abs(transformed_coord[2]) <= precision)
    {
      vertex_points.push_back(prim_vec[6] + prim_vec[0]);
      vertex_points.push_back(prim_vec[7] + prim_vec[1]);
      vertex_points.push_back(prim_vec[8] + prim_vec[2]);
    }
  if (abs(transformed_coord[0]) <= precision && abs(transformed_coord[1]) <= precision &&
      abs(transformed_coord[2]) <= precision)
    {
      // all 7 points need to be added
      vertex_points.push_back(prim_vec[6] + prim_vec[0] + prim_vec[3]);
      vertex_points.push_back(prim_vec[7] + prim_vec[1] + prim_vec[4]);
      vertex_points.push_back(prim_vec[8] + prim_vec[2] + prim_vec[5]);
    }

  // back direction
  if (abs(transformed_coord[0] - 1) <= precision)
    {
      vertex_points.push_back(-prim_vec[0]);
      vertex_points.push_back(-prim_vec[1]);
      vertex_points.push_back(-prim_vec[2]);
    }
  if (abs(transformed_coord[1] - 1) <= precision)
    {
      vertex_points.push_back(-prim_vec[3]);
      vertex_points.push_back(-prim_vec[4]);
      vertex_points.push_back(-prim_vec[5]);
    }
  if (abs(transformed_coord[2] - 1) <= precision)
    {
      vertex_points.push_back(-prim_vec[6]);
      vertex_points.push_back(-prim_vec[7]);
      vertex_points.push_back(-prim_vec[8]);
    }
  if (abs(transformed_coord[0] - 1) <= precision && abs(transformed_coord[1] - 1) <= precision)
    {
      vertex_points.push_back(-prim_vec[0] - prim_vec[3]);
      vertex_points.push_back(-prim_vec[1] - prim_vec[4]);
      vertex_points.push_back(-prim_vec[2] - prim_vec[5]);
    }
  if (abs(transformed_coord[1] - 1) <= precision && abs(transformed_coord[2] - 1) <= precision)
    {
      vertex_points.push_back(-prim_vec[6] - prim_vec[3]);
      vertex_points.push_back(-prim_vec[7] - prim_vec[4]);
      vertex_points.push_back(-prim_vec[8] - prim_vec[5]);
    }
  if (abs(transformed_coord[0] - 1) <= precision && abs(transformed_coord[2] - 1) <= precision)
    {
      vertex_points.push_back(-prim_vec[6] - prim_vec[0]);
      vertex_points.push_back(-prim_vec[7] - prim_vec[1]);
      vertex_points.push_back(-prim_vec[8] - prim_vec[2]);
    }
  if (abs(transformed_coord[0] - 1) <= precision && abs(transformed_coord[1] - 1) <= precision &&
      abs(transformed_coord[2] - 1) <= precision)
    {
      // all 7 points need to be added
      vertex_points.push_back(-prim_vec[6] - prim_vec[0] - prim_vec[3]);
      vertex_points.push_back(-prim_vec[7] - prim_vec[1] - prim_vec[4]);
      vertex_points.push_back(-prim_vec[8] - prim_vec[2] - prim_vec[5]);
    }

  for (int point = 0; point < vertex_points.size() / 3; point++)
    {
      data[0][0].push_back(data[0][0][cnt] + vertex_points[point * 3]);
      data[0][1].push_back(data[0][1][cnt] + vertex_points[point * 3 + 1]);
      data[0][2].push_back(data[0][2][cnt] + vertex_points[point * 3 + 2]);
      labels.push_back(label);
    }
}

grm_error_t XsfSource::readDataFile(const std::string &path, std::vector<std::vector<std::vector<double>>> &data,
                                    std::vector<int> &x_data, std::vector<int> &y_data, std::vector<int> &error_data,
                                    std::vector<std::string> &labels, grm_args_t *args, const char *colms,
                                    const char *x_colms, const char *y_colms, const char *e_colms, PlotRange *ranges,
                                    grm_special_axis_series_t *special_axis_series, InputFlags &input_flags,
                                    std::vector<int> &timestamps, double **special_data_grid)
{
  std::string line;
  std::string token;
  std::ifstream file_path(path);
  std::istream &cin_path = std::cin;
  grm_error_t error = GRM_ERROR_NONE;
  bool prim_vec_line = false, data_grid = false, first_line = true, crystal_data = false;
  int prim_coord_line = 0, data_grid_plane = 0, data_grid_x_dim, data_grid_y_dim;
  std::vector<double> prim_vec, data_grid_vec;
  int cnt = 0, data_grid_cnt = 0;
  double *contour_plot_data;

  // Save locale setting
  const std::string old_locale = std::setlocale(LC_NUMERIC, nullptr);
  std::setlocale(LC_NUMERIC, "C");

  std::istream &file = (path == "-") ? cin_path : file_path;
  /* read the lines from the file */
  while (getline(file, line))
    {
      std::istringstream iss(line, std::istringstream::in);

      if (first_line)
        {
          first_line = false;
          if (startsWith(line, " CRYSTAL")) crystal_data = true;
        }
      else if (startsWith(line, " PRIMVEC"))
        {
          prim_vec_line = true;
          continue;
        }
      else if (startsWith(line, " PRIMCOORD"))
        {
          prim_vec_line = false;
          prim_coord_line = 1;
          continue;
        }
      else if (startsWith(line, " BEGIN_DATAGRID_2D_A"))
        {
          data_grid_plane = 1;
          continue;
        }
      else if (startsWith(line, " END_DATAGRID_2D"))
        {
          *special_data_grid = contour_plot_data;
          int crystal_radius = 4;
          if (crystal_data) grm_args_push(args, "radius_kind", "i", crystal_radius);
          break;
        }

      if (prim_vec_line)
        {
          // push the lines in this vec, but the data is column ordered - respect that in the later calculation
          auto normalized_line = normalizeLine(line);
          std::istringstream line_ss(normalized_line);
          for (int col = 0; std::getline(line_ss, token, '\t') && token.length(); col++)
            {
              prim_vec.push_back(stod(token));
            }
        }
      else if (prim_coord_line == 1)
        {
          prim_coord_line = 2;
          data.emplace_back(std::vector<std::vector<double>>());
          data[0].emplace_back(std::vector<double>()); // x
          data[0].emplace_back(std::vector<double>()); // y
          data[0].emplace_back(std::vector<double>()); // z

          grm_x_value_shift = -(prim_vec[6] + prim_vec[0] + prim_vec[3]) / 2.0;
          grm_y_value_shift = -(prim_vec[7] + prim_vec[1] + prim_vec[4]) / 2.0;
          grm_z_value_shift = -(prim_vec[8] + prim_vec[2] + prim_vec[5]) / 2.0;

          grm_args_push(args, "cell", "nD", prim_vec.size(), prim_vec.data());
        }
      else if (prim_coord_line == 2)
        {
          std::vector<double> prim_coord_vec;
          int col = 0;
          auto normalized_line = normalizeLine(line);
          std::istringstream line_ss(normalized_line);
          for (col = 0; std::getline(line_ss, token, '\t') && token.length(); col++)
            {
              if (col == 0)
                labels.push_back(token); // element number or symbol
              else if (col == 1)
                prim_coord_vec.push_back(stod(token) + grm_x_value_shift); // Needed internally
              else if (col == 2)
                prim_coord_vec.push_back(stod(token) + grm_y_value_shift); // Needed internally
              else if (col == 3)
                prim_coord_vec.push_back(stod(token) + grm_z_value_shift); // Needed internally
            }
          if (col != 0)
            {
              data[0][0].push_back(prim_coord_vec[0]);
              data[0][1].push_back(prim_coord_vec[1]);
              data[0][2].push_back(prim_coord_vec[2]);

              if (crystal_data) calculateDataWithUnitCell(data, labels, prim_vec, prim_coord_vec, cnt);
              cnt += 1;
            }
          else
            {
              prim_coord_line = 0;
            }
        }
      else if (data_grid_plane == 1)
        {
          auto normalized_line = normalizeLine(line);
          std::istringstream line_ss(normalized_line);
          for (int col = 0; std::getline(line_ss, token, '\t') && token.length(); col++)
            {
              if (col == 0)
                data_grid_x_dim = stoi(token);
              else
                data_grid_y_dim = stoi(token);
            }
          // not completely clean but timestamp is never set during molecule so it can be reused to transfer the shape
          // of the contour plot
          timestamps.push_back(data_grid_x_dim);
          timestamps.push_back(data_grid_y_dim);
          contour_plot_data = static_cast<double *>(malloc(data_grid_x_dim * data_grid_y_dim * sizeof(double)));
          data_grid_plane = 2;
        }
      else if (data_grid_plane == 2)
        {
          auto normalized_line = normalizeLine(line);
          std::istringstream line_ss(normalized_line);

          // first row is the start point, the other 2 define the plane
          for (int col = 0; std::getline(line_ss, token, '\t') && token.length(); col++)
            {
              data_grid_vec.push_back(stod(token));
            }

          if (data_grid_vec.size() >= 9)
            {
              data_grid_plane = 0;
              data_grid = true;
            }
        }
      else if (data_grid)
        {
          // save info for contour_plot
          auto y_idx = static_cast<int>(floor(data_grid_cnt / data_grid_y_dim));
          contour_plot_data[data_grid_cnt % data_grid_x_dim + y_idx * data_grid_x_dim] = stod(line);
          data_grid_cnt += 1;
        }
    }

  // Restore locale setting
  std::setlocale(LC_NUMERIC, old_locale.c_str());
  input_flags.xyz_molecule_file = true;
  return error;
}

DataSource *XsfPlugin::getDataSourceFromFile(const std::string &path) const
{
  return new XsfSource;
}

const Plugin::ApiVersion XsfPlugin::apiVersion() const
{
  return ApiVersion{1, 0};
}

#ifdef __EMSCRIPTEN__
extern "C" void *XsfAllocPlugin()
#else
extern "C" void *allocPlugin()
#endif
{
  return new XsfPlugin;
}
#ifdef __EMSCRIPTEN__
extern "C" void XsfDeallocPlugin(void *p)
#else
extern "C" void deallocPlugin(void *p)
#endif
{
  auto d = static_cast<Plugin *>(p);
  delete d;
}
