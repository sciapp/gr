/* ######################### includes ############################################################################### */

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <climits>

#include "args_int.h"
#include "interaction_int.h"
#include "plot_int.h"
#include "utilcpp_int.hxx"
#include "gr.h"
#include "grm/dom_render/render.hxx"
#include <grm/dom_render/graphics_tree/util.hxx>


/* ######################### internal implementation ################################################################ */

/* ========================= static variables ======================================================================= */

/* ------------------------- tooltips ------------------------------------------------------------------------------- */

static tooltip_reflist_t *tooltip_list = nullptr;
static grm_tooltip_info_t *nearest_tooltip = nullptr;

/* ------------------------- movable xform -------------------------------------------------------------------------- */

static std::weak_ptr<GRM::Element> movable_obj_ref;

/* ========================= macros ================================================================================= */

/* ------------------------- math ----------------------------------------------------------------------------------- */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ------------------------- tooltips ------------------------------------------------------------------------------- */

#define MAX_MOUSE_DIST 50


/* ========================= methods ================================================================================ */

/* ------------------------- tooltip list --------------------------------------------------------------------------- */


tooltip_list_t *tooltip_list_new(void)
{
  static const tooltip_list_vtable_t vt = {
      tooltip_list_entry_copy,
      tooltip_list_entry_delete,
  };
  tooltip_list_t *list;

  list = static_cast<tooltip_list_t *>(malloc(sizeof(tooltip_list_t)));
  if (list == nullptr)
    {
      return nullptr;
    }
  list->vt = &vt;
  list->head = nullptr;
  list->tail = nullptr;
  list->size = 0;

  return list;
}

void tooltip_list_delete(tooltip_list_t *list)
{
  tooltip_list_node_t *current_list_node;
  tooltip_list_node_t *next_list_node;

  current_list_node = list->head;
  while (current_list_node != nullptr)
    {
      next_list_node = current_list_node->next;
      list->vt->entry_delete(current_list_node->entry);
      free(current_list_node);
      current_list_node = next_list_node;
    }
  free(list);
}

err_t tooltip_list_push_front(tooltip_list_t *list, tooltip_list_const_entry_t entry)
{
  tooltip_list_node_t *new_list_node;
  err_t error = ERROR_NONE;

  new_list_node = static_cast<tooltip_list_node_t *>(malloc(sizeof(tooltip_list_node_t)));
  error_cleanup_and_set_error_if(new_list_node == nullptr, ERROR_MALLOC);
  error = list->vt->entry_copy(&new_list_node->entry, entry);
  error_cleanup_if_error;
  new_list_node->next = list->head;
  list->head = new_list_node;
  if (list->tail == nullptr)
    {
      list->tail = new_list_node;
    }
  ++(list->size);

  return ERROR_NONE;

error_cleanup:
  free(new_list_node);
  return error;
}

err_t tooltip_list_push_back(tooltip_list_t *list, tooltip_list_const_entry_t entry)
{
  tooltip_list_node_t *new_list_node;
  err_t error = ERROR_NONE;

  new_list_node = static_cast<tooltip_list_node_t *>(malloc(sizeof(tooltip_list_node_t)));
  error_cleanup_and_set_error_if(new_list_node == nullptr, ERROR_MALLOC);
  error = list->vt->entry_copy(&new_list_node->entry, entry);
  error_cleanup_if_error;
  new_list_node->next = nullptr;
  if (list->head == nullptr)
    {
      list->head = new_list_node;
    }
  else
    {
      list->tail->next = new_list_node;
    }
  list->tail = new_list_node;
  ++(list->size);

  return ERROR_NONE;

error_cleanup:
  free(new_list_node);
  return error;
}

tooltip_list_entry_t tooltip_list_pop_front(tooltip_list_t *list)
{
  tooltip_list_node_t *front_node;
  tooltip_list_entry_t front_entry;

  assert(list->head != nullptr);
  front_node = list->head;
  list->head = list->head->next;
  if (list->tail == front_node)
    {
      list->tail = nullptr;
    }
  front_entry = front_node->entry;
  free(front_node);
  --(list->size);

  return front_entry;
}

tooltip_list_entry_t tooltip_list_pop_back(tooltip_list_t *list)
{
  tooltip_list_node_t *last_node;
  tooltip_list_node_t *next_to_last_node = nullptr;
  tooltip_list_entry_t last_entry;

  assert(list->tail != nullptr);
  last_node = list->tail;
  tooltip_list_find_previous_node(list, last_node, &next_to_last_node);
  if (next_to_last_node == nullptr)
    {
      list->head = list->tail = nullptr;
    }
  else
    {
      list->tail = next_to_last_node;
      next_to_last_node->next = nullptr;
    }
  last_entry = last_node->entry;
  free(last_node);
  --(list->size);

  return last_entry;
}

err_t tooltip_list_push(tooltip_list_t *list, tooltip_list_const_entry_t entry)
{
  return tooltip_list_push_front(list, entry);
}

tooltip_list_entry_t tooltip_list_pop(tooltip_list_t *list)
{
  return tooltip_list_pop_front(list);
}

err_t tooltip_list_enqueue(tooltip_list_t *list, tooltip_list_const_entry_t entry)
{
  return tooltip_list_push_back(list, entry);
}

tooltip_list_entry_t tooltip_list_dequeue(tooltip_list_t *list)
{
  return tooltip_list_pop_front(list);
}

int tooltip_list_empty(tooltip_list_t *list)
{
  return list->size == 0;
}

int tooltip_list_find_previous_node(const tooltip_list_t *list, const tooltip_list_node_t *node,
                                    tooltip_list_node_t **previous_node)
{
  tooltip_list_node_t *prev_node;
  tooltip_list_node_t *current_node;

  prev_node = nullptr;
  current_node = list->head;
  while (current_node != nullptr)
    {
      if (current_node == node)
        {
          if (previous_node != nullptr)
            {
              *previous_node = prev_node;
            }
          return 1;
        }
      prev_node = current_node;
      current_node = current_node->next;
    }

  return 0;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~ ref list ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

tooltip_reflist_t *tooltip_reflist_new(void)
{
  static const tooltip_reflist_vtable_t vt = {
      tooltip_reflist_entry_copy,
      tooltip_reflist_entry_delete,
  };
  tooltip_reflist_t *list;

  list = (tooltip_reflist_t *)tooltip_list_new();
  list->vt = &vt;

  return list;
}

void tooltip_reflist_delete(tooltip_reflist_t *list)
{
  tooltip_list_delete((tooltip_list_t *)list);
}

void tooltip_reflist_delete_with_entries(tooltip_reflist_t *list)
{
  tooltip_reflist_node_t *current_reflist_node;
  tooltip_reflist_node_t *next_reflist_node;

  current_reflist_node = list->head;
  while (current_reflist_node != nullptr)
    {
      next_reflist_node = current_reflist_node->next;
      tooltip_list_entry_delete((tooltip_list_entry_t)current_reflist_node->entry);
      free(current_reflist_node);
      current_reflist_node = next_reflist_node;
    }
  free(list);
}

err_t tooltip_reflist_push_front(tooltip_reflist_t *list, tooltip_reflist_entry_t entry)
{
  return tooltip_list_push_front((tooltip_list_t *)list, (tooltip_list_entry_t)entry);
}

err_t tooltip_reflist_push_back(tooltip_reflist_t *list, tooltip_reflist_entry_t entry)
{
  return tooltip_list_push_back((tooltip_list_t *)list, (tooltip_list_entry_t)entry);
}

tooltip_reflist_entry_t tooltip_reflist_pop_front(tooltip_reflist_t *list)
{
  return tooltip_list_pop_front((tooltip_list_t *)list);
}

tooltip_reflist_entry_t tooltip_reflist_pop_back(tooltip_reflist_t *list)
{
  return tooltip_list_pop_back((tooltip_list_t *)list);
}

err_t tooltip_reflist_push(tooltip_reflist_t *list, tooltip_reflist_entry_t entry)
{
  return tooltip_list_push((tooltip_list_t *)list, (tooltip_list_entry_t)entry);
}

tooltip_reflist_entry_t tooltip_reflist_pop(tooltip_reflist_t *list)
{
  return tooltip_list_pop((tooltip_list_t *)list);
}

err_t tooltip_reflist_enqueue(tooltip_reflist_t *list, tooltip_reflist_entry_t entry)
{
  return tooltip_list_enqueue((tooltip_list_t *)list, (tooltip_list_entry_t)entry);
}

tooltip_reflist_entry_t tooltip_reflist_dequeue(tooltip_reflist_t *list)
{
  return tooltip_list_dequeue((tooltip_list_t *)list);
}

int tooltip_reflist_empty(tooltip_reflist_t *list)
{
  return tooltip_list_empty((tooltip_list_t *)list);
}


int tooltip_reflist_find_previous_node(const tooltip_reflist_t *list, const tooltip_reflist_node_t *node,
                                       tooltip_reflist_node_t **previous_node)
{
  return tooltip_list_find_previous_node((tooltip_list_t *)list, (tooltip_list_node_t *)node,
                                         (tooltip_list_node_t **)previous_node);
}

err_t tooltip_reflist_entry_copy(tooltip_reflist_entry_t *copy, tooltip_reflist_entry_t entry)
{
  *copy = entry;
  return ERROR_NONE;
}

err_t tooltip_reflist_entry_delete(tooltip_reflist_entry_t entry UNUSED)
{
  return ERROR_NONE;
}

err_t tooltip_list_entry_copy(tooltip_list_entry_t *copy, tooltip_list_const_entry_t entry)
{
  tooltip_list_entry_t _copy;

  _copy = static_cast<tooltip_list_entry_t>(malloc(sizeof(grm_tooltip_info_t)));
  if (_copy == nullptr)
    {
      return ERROR_MALLOC;
    }

  memcpy(_copy, entry, sizeof(grm_tooltip_info_t));
  *copy = _copy;

  return ERROR_NONE;
}

err_t tooltip_list_entry_delete(tooltip_list_entry_t entry)
{
  free(entry);
  return ERROR_NONE;
}


/* ######################### public implementation ################################################################## */

/* ========================= methods ================================================================================ */

/* ------------------------- user interaction ----------------------------------------------------------------------- */

static void moveTransformationHelper(const std::shared_ptr<GRM::Element> &element, double ndc_x, double ndc_y,
                                     double xshift, double yshift, bool is_movable)
{
  double x_with_shift, y_with_shift, x_ndc, y_ndc;
  double old_x_shift = 0, old_y_shift = 0, wc_x_shift, wc_y_shift;
  int width, height, max_width_height;
  std::string post_fix = "_wc";
  std::vector<std::string> ndc_transformation_elems = {"figure",
                                                       "plot",
                                                       "colorbar",
                                                       "labels_group",
                                                       "titles_3d",
                                                       "text",
                                                       "layout_grid_element",
                                                       "layout_grid",
                                                       "central_region",
                                                       "side_region",
                                                       "marginal_heatmap_plot",
                                                       "legend",
                                                       "axis"};

  GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
  max_width_height = grm_max(width, height);

  if (std::find(ndc_transformation_elems.begin(), ndc_transformation_elems.end(), element->localName()) !=
      ndc_transformation_elems.end())
    post_fix = "_ndc";

  x_with_shift = ndc_x + (double)xshift / max_width_height;
  x_ndc = ndc_x;
  y_with_shift = ndc_y + (double)yshift / max_width_height;
  y_ndc = ndc_y;

  gr_ndctowc(&x_with_shift, &y_with_shift);
  gr_ndctowc(&x_ndc, &y_ndc);

  wc_x_shift = x_with_shift - x_ndc;
  wc_y_shift = y_ndc - y_with_shift;

  if (element->hasAttribute("x_shift" + post_fix))
    old_x_shift = static_cast<double>(element->getAttribute("x_shift" + post_fix));
  if (element->hasAttribute("y_shift" + post_fix))
    old_y_shift = static_cast<double>(element->getAttribute("y_shift" + post_fix));

  if (xshift != 0)
    {
      if (post_fix == "_wc")
        {
          if (is_movable && element->localName() == "polyline" &&
              static_cast<std::string>(element->getAttribute("name")) == "integral_left")
            {
              auto old_int_lim_low = static_cast<double>(element->parentElement()->getAttribute("int_lim_low"));
              element->parentElement()->setAttribute("int_lim_low", old_int_lim_low + wc_x_shift);
            }
          else if (is_movable && element->localName() == "polyline" &&
                   static_cast<std::string>(element->getAttribute("name")) == "integral_right")
            {
              auto old_int_lim_high = static_cast<double>(element->parentElement()->getAttribute("int_lim_high"));
              element->parentElement()->setAttribute("int_lim_high", old_int_lim_high + wc_x_shift);
            }
          else
            {
              element->setAttribute("x_shift" + post_fix, old_x_shift + wc_x_shift);
            }
        }
      else
        {
          element->setAttribute("x_shift" + post_fix, old_x_shift + (double)xshift / max_width_height);
        }
    }
  if (yshift != 0)
    {
      if (post_fix == "_wc")
        {
          element->setAttribute("y_shift" + post_fix, old_y_shift + wc_y_shift);
        }
      else
        {
          element->setAttribute("y_shift" + post_fix, old_y_shift + (double)yshift / max_width_height);
        }
    }
}

int grm_input(const grm_args_t *input_args)
{
  /*
   * reset_ranges:
   * - `x`, `y`: mouse cursor position
   * - `key`: Pressed key (as string)
   * zoom:
   * - `x`, `y`: start point
   * - `angle_delta`: mouse wheel rotation angle in eighths of a degree, can be replaced by factor (double type)
   * - `factor`: zoom factor, can be replaced by angle_delta (double type)
   * box zoom:
   * - `x1`, `y1`, `x2`, `y2`: coordinates of a box selection, (x1, y1) is the fixed corner
   * - `keep_aspect_ratio`: if set to `1`, the aspect ratio of the gr window is preserved (defaults to `1`)
   * pan:
   * - `x`, `y`: start point
   * - `x_shift`, `y_shift`: shift in x and y direction
   * movable_xform:
   * - `x_shift`, `y_shift`: shift in x and y direction
   * - `disable_movable_trans`: disable movable transformation
   * - `movable_state`: the status from grm_get_hover_mode
   * - `clear_locked_state`: clear movable_obj_ref pointer
   *
   * All coordinates are expected to be given as workstation coordinates (integer type)
   */
  int width, height, max_width_height;
  int x, y, x1, y1, x2, y2;
  double viewport_mid_x, viewport_mid_y;
  bool clear_locked_state = false;
  int selection_status;
  logger((stderr, "Processing input\n"));

  GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
  max_width_height = grm_max(width, height);
  logger((stderr, "Using size (%d, %d)\n", width, height));

  auto marginal_heatmap = grm_get_document_root()->querySelectorsAll("marginal_heatmap_plot");
  if (!marginal_heatmap.empty())
    {
      if (marginal_heatmap[0]->hasAttribute("x_ind")) marginal_heatmap[0]->setAttribute("x_ind", -1);
      if (marginal_heatmap[0]->hasAttribute("y_ind")) marginal_heatmap[0]->setAttribute("y_ind", -1);
    }

  if (grm_args_values(input_args, "clear_locked_state", "i", &clear_locked_state) && clear_locked_state)
    {
      movable_obj_ref.reset();
    }

  if (grm_args_values(input_args, "x", "i", &x) && grm_args_values(input_args, "y", "i", &y))
    {
      double ndc_x, ndc_y;
      char *key;

      ndc_x = (double)x / max_width_height;
      ndc_y = (double)(height - y) / max_width_height;
      logger((stderr, "x: %d, y: %d, ndc_x: %lf, ndc_y: %lf\n", x, y, ndc_x, ndc_y));

      auto subplot_element = get_subplot_from_ndc_point_using_dom(ndc_x, ndc_y);
      if (subplot_element == nullptr && grm_args_values(input_args, "move_selection", "i", &selection_status))
        {
          // needed so the cursor doesn't need to be inside the window for gredit element moving
          subplot_element = grm_get_document_root()->querySelectors("plot[plot_group]");
        }

      if (grm_args_values(input_args, "key", "s", &key))
        {
          logger((stderr, "Got key \"%s\"\n", key));

          if (strcmp(key, "r") == 0)
            {
              if (subplot_element != nullptr)
                {
                  auto coordinate_system = subplot_element->querySelectors("coordinate_system");
                  if (coordinate_system->hasAttribute("plot_type") &&
                      (static_cast<std::string>(coordinate_system->getAttribute("plot_type")) == "2d" ||
                       static_cast<std::string>(coordinate_system->getAttribute("plot_type")) == "polar"))
                    {
                      logger((stderr, "Reset single subplot coordinate ranges\n"));
                      subplot_element->setAttribute("reset_ranges", 1);
                    }
                  if (coordinate_system->hasAttribute("plot_type") &&
                      static_cast<std::string>(coordinate_system->getAttribute("plot_type")) == "3d")
                    {
                      logger((stderr, "Reset single subplot coordinate rotation\n"));
                      auto central_region = subplot_element->querySelectors("central_region");
                      central_region->setAttribute("reset_rotation", 1);
                    }
                }
              else
                {
                  logger((stderr, "Reset all subplot coordinate ranges\n"));
                  grm_set_attribute_on_all_subplots("reset_ranges", 1);
                }
            }

          return 1;
        }

      if (subplot_element != nullptr)
        {
          double angle_delta, factor;
          int xshift, yshift, xind, yind;
          int movable_status;
          std::string kind;
          double viewport[4];
          auto central_region = subplot_element->querySelectors("central_region");
          auto coordinate_system = subplot_element->querySelectors("coordinate_system");
          viewport[0] = static_cast<double>(central_region->getAttribute("viewport_x_min"));
          viewport[1] = static_cast<double>(central_region->getAttribute("viewport_x_max"));
          viewport[2] = static_cast<double>(central_region->getAttribute("viewport_y_min"));
          viewport[3] = static_cast<double>(central_region->getAttribute("viewport_y_max"));

          kind = static_cast<std::string>(subplot_element->getAttribute("kind"));
          if (kind == "marginal_heatmap")
            {
              auto current_series = subplot_element->querySelectorsAll("series_heatmap")[0];

              unsigned int x_length, y_length;
              double x_0, x_end, y_0, y_end, x_step, y_step, xind_d, yind_d;
              std::vector<double> x_steps, y_steps;
              int x_offset = 0, y_offset = 0;
              double x_min = 0, y_min = 0;

              grm_args_values(input_args, "x", "i", &x);
              grm_args_values(input_args, "y", "i", &y);
              auto x_series_key = static_cast<std::string>(current_series->getAttribute("x"));
              auto y_series_key = static_cast<std::string>(current_series->getAttribute("y"));

              std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();

              auto x_series_vec = GRM::get<std::vector<double>>((*context)[x_series_key]);
              auto y_series_vec = GRM::get<std::vector<double>>((*context)[y_series_key]);

              x_length = x_series_vec.size();
              y_length = y_series_vec.size();
              x_0 = x_series_vec[0], x_end = x_series_vec[x_length - 1];
              y_0 = y_series_vec[0], y_end = y_series_vec[y_length - 1];

              if (static_cast<int>(subplot_element->getAttribute("x_log")))
                {
                  for (int j = 0; j < (int)x_length; j++)
                    {
                      if (!grm_isnan(x_series_vec[j]))
                        {
                          x_min = x_series_vec[j];
                          break;
                        }
                      x_offset += 1;
                    }
                  x_0 = x_min;
                }
              if (static_cast<int>(subplot_element->getAttribute("y_log")))
                {
                  for (int j = 0; j < (int)y_length; j++)
                    {
                      if (!grm_isnan(y_series_vec[j]))
                        {
                          y_min = y_series_vec[j];
                          break;
                        }
                      y_offset += 1;
                    }
                  y_0 = y_min;
                }

              GRM::Render::processLimits(subplot_element);
              GRM::Render::processWindow(central_region);

              gr_savestate();
              for (const auto &elem : subplot_element->parentElement()->children())
                {
                  if (elem->hasAttribute("scale"))
                    {
                      gr_setviewport(static_cast<double>(central_region->getAttribute("viewport_x_min")),
                                     static_cast<double>(central_region->getAttribute("viewport_x_max")),
                                     static_cast<double>(central_region->getAttribute("viewport_y_min")),
                                     static_cast<double>(central_region->getAttribute("viewport_y_max")));
                      gr_setwindow(static_cast<double>(central_region->getAttribute("window_x_min")),
                                   static_cast<double>(central_region->getAttribute("window_x_max")),
                                   static_cast<double>(central_region->getAttribute("window_y_min")),
                                   static_cast<double>(central_region->getAttribute("window_y_max")));
                      gr_setscale(static_cast<int>(elem->getAttribute("scale")));
                      break;
                    }
                }
              GRM::Render::calculateCharHeight(central_region);

              gr_wctondc(&x_0, &y_0);
              gr_wctondc(&x_end, &y_end);
              x_0 = x_0 * max_width_height;
              x_end = x_end * max_width_height;
              y_0 = height - y_0 * max_width_height;
              y_end = height - y_end * max_width_height;

              x_step = (x_end - x_0) / x_length;
              y_step = (y_end - y_0) / y_length;

              if (static_cast<int>(subplot_element->getAttribute("x_log")))
                {
                  for (int j = 0; j < (int)x_length - x_offset; j++)
                    {
                      double step_x = (x_series_vec[x_length - 1] - x_min) / (x_length - x_offset);
                      double tmp = 0, a = x_min + j * step_x, b = x_min + (j + 1) * step_x;
                      gr_wctondc(&a, &tmp);
                      gr_wctondc(&b, &tmp);

                      a = a * max_width_height;
                      b = b * max_width_height;
                      x_steps.push_back(b - a);
                    }
                }
              if (static_cast<int>(subplot_element->getAttribute("y_log")))
                {
                  for (int j = 0; j < (int)y_length - y_offset; j++)
                    {
                      double step_y = (y_series_vec[y_length - 1] - y_min) / (y_length - y_offset);
                      double tmp = 0, a = y_min + j * step_y, b = y_min + (j + 1) * step_y;
                      gr_wctondc(&tmp, &a);
                      gr_wctondc(&tmp, &b);

                      a = a * max_width_height;
                      b = b * max_width_height;
                      y_steps.push_back(-(b - a));
                    }
                }

              xind_d = (x - x_0) / x_step;
              if (static_cast<int>(subplot_element->getAttribute("x_log")))
                {
                  double tmp = 0;
                  xind_d = 0;
                  for (int j = 0; j < (int)x_length - x_offset; j++)
                    {
                      if (tmp + x_steps[j] > (x - x_0)) break;
                      tmp += x_steps[j];
                      xind_d += 1;
                    }
                }

              yind_d = (y - y_0) / y_step;
              if (static_cast<int>(subplot_element->getAttribute("y_log")))
                {
                  double tmp = 0;
                  yind_d = 0;
                  for (int j = 0; j < (int)y_length - y_offset; j++)
                    {
                      if (tmp + y_steps[j] < (y - y_0)) break;
                      tmp += y_steps[j];
                      yind_d += 1;
                    }
                }

              if (xind_d < 0 || xind_d >= x_length || yind_d < 0 || yind_d >= y_length)
                {
                  xind = -1;
                  yind = -1;
                }
              else
                {
                  xind = (int)xind_d;
                  yind = (int)yind_d;
                }

              auto old_xind = static_cast<int>(marginal_heatmap[0]->getAttribute("x_ind"));
              auto old_yind = static_cast<int>(marginal_heatmap[0]->getAttribute("y_ind"));
              marginal_heatmap[0]->setAttribute("x_ind", xind);
              marginal_heatmap[0]->setAttribute("y_ind", yind);
              if (static_cast<std::string>(marginal_heatmap[0]->getAttribute("marginal_heatmap_kind")) == "line" &&
                  ((old_xind == -1 || old_yind == -1) && xind != -1 && yind != -1))
                marginal_heatmap[0]->setAttribute("_update_required", true);

              for (auto &side_region : marginal_heatmap[0]->children())
                {
                  if (side_region->localName() == "side_region")
                    {
                      for (auto &child : side_region->children())
                        {
                          std::string childKind = static_cast<std::string>(child->getAttribute("kind"));
                          if (childKind == "hist")
                            {
                              // reset bar colors
                              // bar level
                              for (auto &bars : child->children())
                                {
                                  // fill- and draw_rect level
                                  for (auto &childSeries : bars->children())
                                    {
                                      auto groups = childSeries->children(); // innerFillGroup and outerFillGroup
                                      std::shared_ptr<GRM::Element> innerFillGroup;
                                      if (groups.size() == 2)
                                        {
                                          innerFillGroup = groups[0];
                                        }
                                      else
                                        {
                                          // no fillGroups?
                                          break;
                                        }

                                      if (xind != -1)
                                        {
                                          innerFillGroup->children()[xind]->removeAttribute("fill_color_ind");
                                        }
                                      if (yind != -1)
                                        {
                                          innerFillGroup->children()[yind]->removeAttribute("fill_color_ind");
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
              gr_restorestate();
            }

          if (grm_args_values(input_args, "angle_delta", "d", &angle_delta))
            {
              double focus_x, focus_y;

              if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume",
                                 "isosurface"))
                {
                  /*
                   * TODO Zoom in 3D
                   */
                }
              else
                {
                  viewport_mid_x = (viewport[0] + viewport[1]) / 2.0;
                  viewport_mid_y = (viewport[2] + viewport[3]) / 2.0;
                  focus_x = ndc_x - viewport_mid_x;
                  focus_y = ndc_y - viewport_mid_y;
                  if (static_cast<std::string>(coordinate_system->getAttribute("plot_type")) == "polar")
                    focus_x = focus_y = 0.0;
                  logger(
                      (stderr, "Zoom to ndc focus point (%lf, %lf), angle_delta %lf\n", focus_x, focus_y, angle_delta));
                  double zoom = 1.0 - INPUT_ANGLE_DELTA_FACTOR * angle_delta;
                  auto panzoom_element = grm_get_render()->createPanzoom(focus_x, focus_y, zoom, zoom);
                  subplot_element->append(panzoom_element);
                  subplot_element->setAttribute("panzoom", true);
                }

              return 1;
            }
          else if (grm_args_values(input_args, "factor", "d", &factor))
            {
              double focus_x, focus_y;

              if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume",
                                 "isosurface"))
                {
                  /*
                   * TODO Zoom in 3D
                   */
                }
              else
                {
                  viewport_mid_x = (viewport[0] + viewport[1]) / 2.0;
                  viewport_mid_y = (viewport[2] + viewport[3]) / 2.0;
                  focus_x = ndc_x - viewport_mid_x;
                  focus_y = ndc_y - viewport_mid_y;
                  if (static_cast<std::string>(coordinate_system->getAttribute("plot_type")) == "polar")
                    focus_x = focus_y = 0.0;
                  logger((stderr, "Zoom to ndc focus point (%lf, %lf), factor %lf\n", focus_x, focus_y, factor));
                  auto panzoom_element = grm_get_render()->createPanzoom(focus_x, focus_y, factor, factor);
                  subplot_element->append(panzoom_element);
                  subplot_element->setAttribute("panzoom", true);
                }
              return 1;
            }

          if (grm_args_values(input_args, "x_shift", "i", &xshift) &&
              grm_args_values(input_args, "y_shift", "i", &yshift) &&
              grm_args_values(input_args, "move_selection", "i", &selection_status))
            {
              GRM::Render::processLimits(subplot_element);
              GRM::Render::processWindow(central_region);

              gr_savestate();
              for (const auto &elem : subplot_element->parentElement()->children())
                {
                  if (elem->hasAttribute("scale"))
                    {
                      gr_setviewport(static_cast<double>(central_region->getAttribute("viewport_x_min")),
                                     static_cast<double>(central_region->getAttribute("viewport_x_max")),
                                     static_cast<double>(central_region->getAttribute("viewport_y_min")),
                                     static_cast<double>(central_region->getAttribute("viewport_y_max")));
                      gr_setwindow(static_cast<double>(central_region->getAttribute("window_x_min")),
                                   static_cast<double>(central_region->getAttribute("window_x_max")),
                                   static_cast<double>(central_region->getAttribute("window_y_min")),
                                   static_cast<double>(central_region->getAttribute("window_y_max")));
                      gr_setscale(static_cast<int>(elem->getAttribute("scale")));
                      break;
                    }
                }
              GRM::Render::calculateCharHeight(central_region);

              for (const auto &selection : grm_get_document_root()->querySelectorsAll("[_selected=1]"))
                {
                  moveTransformationHelper(selection, ndc_x, ndc_y, xshift, yshift, false);
                }
              gr_restorestate();
            }
          else if (grm_args_values(input_args, "x_shift", "i", &xshift) &&
                   grm_args_values(input_args, "y_shift", "i", &yshift) &&
                   !grm_args_values(input_args, "movable_state", "i", &movable_status) &&
                   static_cast<std::string>(coordinate_system->getAttribute("plot_type")) != "polar")
            {
              double ndc_xshift, ndc_yshift, rotation, tilt;
              int shift_pressed;

              if (str_equals_any(kind, "wireframe", "surface", "plot3", "scatter3", "trisurface", "volume",
                                 "isosurface"))
                {
                  if (grm_args_values(input_args, "shift_pressed", "i", &shift_pressed) && shift_pressed)
                    {
                      /*
                       * TODO Translate in 3D
                       */
                    }
                  else
                    {
                      rotation = static_cast<double>(central_region->getAttribute("space_3d_phi"));
                      tilt = static_cast<double>(central_region->getAttribute("space_3d_theta"));

                      rotation += xshift * 0.2;
                      tilt -= yshift * 0.2;

                      if (tilt > 180)
                        {
                          tilt = 180;
                        }
                      else if (tilt < 0)
                        {
                          tilt = 0;
                        }
                      grm_get_render()->setAutoUpdate(false);
                      central_region->setAttribute("space_3d_phi", rotation);
                      central_region->setAttribute("space_3d_theta", tilt);
                      grm_get_render()->setAutoUpdate(true);
                    }
                }
              else
                {
                  ndc_xshift = (double)-xshift / max_width_height;
                  ndc_yshift = (double)yshift / max_width_height;
                  logger((stderr, "Translate by ndc coordinates (%lf, %lf)\n", ndc_xshift, ndc_yshift));
                  auto panzoom_element = grm_get_render()->createPanzoom(ndc_xshift, ndc_yshift, 0, 0);
                  subplot_element->append(panzoom_element);
                  subplot_element->setAttribute("panzoom", true);
                }
              return 1;
            }
          else if (grm_args_values(input_args, "x_shift", "i", &xshift) &&
                   grm_args_values(input_args, "y_shift", "i", &yshift) &&
                   grm_args_values(input_args, "movable_state", "i", &movable_status))
            {
              std::shared_ptr<GRM::Element> movable = movable_obj_ref.lock();
              double min_diff = INFINITY;
              bool disable_movable_trans = false;

              grm_args_values(input_args, "disable_movable_trans", "i", &disable_movable_trans);

              if (movable == nullptr)
                {
                  auto movable_elems = subplot_element->querySelectorsAll("[movable=1]");
                  for (const auto &move_elem : movable_elems)
                    {
                      auto bbox_x_min = static_cast<double>(move_elem->getAttribute("_bbox_x_min"));
                      auto bbox_x_max = static_cast<double>(move_elem->getAttribute("_bbox_x_max"));
                      auto bbox_y_min = static_cast<double>(move_elem->getAttribute("_bbox_y_min"));
                      auto bbox_y_max = static_cast<double>(move_elem->getAttribute("_bbox_y_max"));
                      if (bbox_x_min <= x && bbox_x_max >= x && bbox_y_min <= y && bbox_y_max >= y)
                        {
                          double diff = sqrt(pow(((bbox_x_max + bbox_x_min) / 2 - x), 2) +
                                             pow(((bbox_y_max + bbox_y_min) / 2 - y), 2));
                          if (diff < min_diff)
                            {
                              movable = move_elem;
                              min_diff = diff;
                            }
                        }
                    }
                }

              if (!disable_movable_trans && movable != nullptr)
                {
                  movable_obj_ref = movable;

                  GRM::Render::processLimits(subplot_element);
                  GRM::Render::processWindow(central_region);

                  gr_savestate();
                  for (const auto &elem : subplot_element->parentElement()->children())
                    {
                      if (elem->hasAttribute("scale"))
                        {
                          gr_setviewport(static_cast<double>(central_region->getAttribute("viewport_x_min")),
                                         static_cast<double>(central_region->getAttribute("viewport_x_max")),
                                         static_cast<double>(central_region->getAttribute("viewport_y_min")),
                                         static_cast<double>(central_region->getAttribute("viewport_y_max")));
                          gr_setwindow(static_cast<double>(central_region->getAttribute("window_x_min")),
                                       static_cast<double>(central_region->getAttribute("window_x_max")),
                                       static_cast<double>(central_region->getAttribute("window_y_min")),
                                       static_cast<double>(central_region->getAttribute("window_y_max")));
                          gr_setscale(static_cast<int>(elem->getAttribute("scale")));
                          break;
                        }
                    }
                  GRM::Render::calculateCharHeight(central_region);

                  moveTransformationHelper(movable, ndc_x, ndc_y, xshift, yshift, true);
                  gr_restorestate();
                }
            }
        }
    }

  if (grm_args_values(input_args, "x1", "i", &x1) && grm_args_values(input_args, "x2", "i", &x2) &&
      grm_args_values(input_args, "y1", "i", &y1) && grm_args_values(input_args, "y2", "i", &y2))
    {
      double focus_x, focus_y, factor_x, factor_y;
      int keep_aspect_ratio = INPUT_DEFAULT_KEEP_ASPECT_RATIO;
      std::shared_ptr<GRM::Element> subplot_element;

      grm_args_values(input_args, "keep_aspect_ratio", "i", &keep_aspect_ratio);

      if (!get_focus_and_factor_from_dom(x1, y1, x2, y2, keep_aspect_ratio, &factor_x, &factor_y, &focus_x, &focus_y,
                                         subplot_element))
        {
          return 0;
        }

      logger((stderr, "Got widget size: (%d, %d)\n", width, height));
      logger((stderr, "Got box: (%d, %d, %d, %d)\n", x1, y1, x2, y2));
      logger((stderr, "zoom focus: (%lf, %lf)\n", focus_x, focus_y));
      logger((stderr, "zoom factors: (%lf, %lf)\n", factor_x, factor_y));

      auto panzoom_element = grm_get_render()->createPanzoom(focus_x, focus_y, factor_x, factor_y);
      subplot_element->append(panzoom_element);
      subplot_element->setAttribute("panzoom", true);

      return 1;
    }

  return 0;
}

int grm_is3d(const int x, const int y)
{
  int width, height, max_width_height;
  double ndc_x, ndc_y;

  GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
  max_width_height = grm_max(width, height);
  ndc_x = (double)x / max_width_height;
  ndc_y = (double)y / max_width_height;

  auto subplot_element = get_subplot_from_ndc_points_using_dom(1, &ndc_x, &ndc_y);

  if (subplot_element && str_equals_any(static_cast<std::string>(subplot_element->getAttribute("kind")), "wireframe",
                                        "surface", "plot3", "scatter3", "trisurface", "volume", "isosurface"))
    {
      return 1;
    }
  return 0;
}

int grm_get_box(const int x1, const int y1, const int x2, const int y2, const int keep_aspect_ratio, int *x, int *y,
                int *w, int *h)
{
  int width, height, max_width_height;
  double focus_x, focus_y, factor_x, factor_y;
  double viewport_mid_x, viewport_mid_y;
  double ws_window[4], viewport[4];
  std::shared_ptr<GRM::Element> subplot_element;
  GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
  max_width_height = grm_max(width, height);
  if (!get_focus_and_factor_from_dom(x1, y1, x2, y2, keep_aspect_ratio, &factor_x, &factor_y, &focus_x, &focus_y,
                                     subplot_element))
    {
      return 0;
    }
  auto central_region = subplot_element->querySelectors("central_region");
  ws_window[0] = static_cast<double>(subplot_element->parentElement()->getAttribute("ws_window_x_min"));
  ws_window[1] = static_cast<double>(subplot_element->parentElement()->getAttribute("ws_window_x_max"));
  ws_window[2] = static_cast<double>(subplot_element->parentElement()->getAttribute("ws_window_y_min"));
  ws_window[3] = static_cast<double>(subplot_element->parentElement()->getAttribute("ws_window_y_max"));
  viewport[0] = static_cast<double>(central_region->getAttribute("viewport_x_min"));
  viewport[1] = static_cast<double>(central_region->getAttribute("viewport_x_max"));
  viewport[2] = static_cast<double>(central_region->getAttribute("viewport_y_min"));
  viewport[3] = static_cast<double>(central_region->getAttribute("viewport_y_max"));
  viewport_mid_x = (viewport[1] + viewport[0]) / 2.0;
  viewport_mid_y = (viewport[3] + viewport[2]) / 2.0;
  *w = (int)grm_round(factor_x * width * (viewport[1] - viewport[0]) / (ws_window[1] - ws_window[0]));
  *h = (int)grm_round(factor_y * height * (viewport[3] - viewport[2]) / (ws_window[3] - ws_window[2]));
  *x = (int)grm_round(((viewport_mid_x + focus_x) - ((viewport_mid_x + focus_x) - viewport[0]) * factor_x) *
                      max_width_height);
  *y = (int)grm_round(height - ((viewport_mid_y + focus_y) - ((viewport_mid_y + focus_y) - viewport[3]) * factor_y) *
                                   max_width_height);
  return 1;
}

static err_t find_nearest_tooltip(int mouse_x, int mouse_y, grm_tooltip_info_t *tooltip_info)
{
  if (nearest_tooltip == nullptr)
    {
      nearest_tooltip = tooltip_info;
    }
  else
    {
      int old_distance = (mouse_x - nearest_tooltip->x_px) * (mouse_x - nearest_tooltip->x_px) +
                         (mouse_y - nearest_tooltip->y_px) * (mouse_y - nearest_tooltip->y_px);
      int current_distance = (mouse_x - tooltip_info->x_px) * (mouse_x - tooltip_info->x_px) +
                             (mouse_y - tooltip_info->y_px) * (mouse_y - tooltip_info->y_px);
      if (current_distance < old_distance)
        {
          free(nearest_tooltip);
          nearest_tooltip = tooltip_info;
        }
      else
        {
          free(tooltip_info);
        }
    }

  return ERROR_NONE;
}

grm_tooltip_info_t *grm_get_tooltip(int mouse_x, int mouse_y)
{
  nearest_tooltip = nullptr;
  get_tooltips(mouse_x, mouse_y, find_nearest_tooltip);
  if (nearest_tooltip != nullptr && (mouse_x - nearest_tooltip->x_px) * (mouse_x - nearest_tooltip->x_px) +
                                            (mouse_y - nearest_tooltip->y_px) * (mouse_y - nearest_tooltip->y_px) >
                                        MAX_MOUSE_DIST)
    {
      nearest_tooltip->x_px = -1;
      nearest_tooltip->y_px = -1;
    }
  return nearest_tooltip;
}

static err_t collect_tooltips(int mouse_x, int mouse_y, grm_tooltip_info_t *tooltip_info)
{
  return tooltip_reflist_push_back(tooltip_list, tooltip_info);
}

grm_tooltip_info_t **grm_get_tooltips_x(int mouse_x, int mouse_y, unsigned int *array_length)
{
  grm_tooltip_info_t **tooltip_array = nullptr, **tooltip_ptr = nullptr;
  tooltip_reflist_node_t *tooltip_reflist_node = nullptr;

  tooltip_list = tooltip_reflist_new();
  error_cleanup_if(tooltip_list == nullptr);

  error_cleanup_if(get_tooltips(mouse_x, mouse_y, collect_tooltips) != ERROR_NONE);

  tooltip_array = static_cast<grm_tooltip_info_t **>(calloc(tooltip_list->size + 1, sizeof(grm_tooltip_info_t *)));
  error_cleanup_if(tooltip_array == nullptr);

  tooltip_ptr = tooltip_array;
  tooltip_reflist_node = tooltip_list->head;
  while (tooltip_reflist_node != nullptr)
    {
      *tooltip_ptr = tooltip_reflist_node->entry;
      ++tooltip_ptr;
      tooltip_reflist_node = tooltip_reflist_node->next;
    }
  *tooltip_ptr = static_cast<grm_tooltip_info_t *>(calloc(1, sizeof(grm_tooltip_info_t)));
  error_cleanup_if(*tooltip_ptr == nullptr);
  (*tooltip_ptr)->label = nullptr;
  if (array_length != nullptr)
    {
      *array_length = tooltip_list->size;
    }

  if (tooltip_list != nullptr)
    {
      tooltip_reflist_delete(tooltip_list);
      tooltip_list = nullptr;
    }

  return tooltip_array;

error_cleanup:
  if (tooltip_array != nullptr)
    {
      if (tooltip_list != nullptr)
        {
          free(tooltip_array[tooltip_list->size]);
        }
      free(tooltip_array);
    }
  if (tooltip_list != nullptr)
    {
      tooltip_reflist_node = tooltip_list->head;
      while (tooltip_reflist_node != nullptr)
        {
          free(tooltip_reflist_node->entry);
          tooltip_reflist_node = tooltip_reflist_node->next;
        }
      tooltip_reflist_delete(tooltip_list);
      tooltip_list = nullptr;
    }

  return nullptr;
}

grm_accumulated_tooltip_info_t *grm_get_accumulated_tooltip_x(int mouse_x, int mouse_y)
{
  double *y = nullptr, *y_ptr = nullptr;
  char **ylabels = nullptr, **ylabels_ptr = nullptr;
  unsigned int min_dist = UINT_MAX;
  grm_tooltip_info_t *nearest_tooltip = nullptr;
  tooltip_reflist_node_t *tooltip_reflist_node = nullptr;
  grm_accumulated_tooltip_info_t *accumulated_tooltip = nullptr;

  tooltip_list = tooltip_reflist_new();
  error_cleanup_if(tooltip_list == nullptr);

  error_cleanup_if(get_tooltips(mouse_x, mouse_y, collect_tooltips) != ERROR_NONE);

  y = static_cast<double *>(malloc(tooltip_list->size * sizeof(double)));
  error_cleanup_if(y == nullptr);
  ylabels = static_cast<char **>(malloc((tooltip_list->size + 1) * sizeof(char *)));
  error_cleanup_if(ylabels == nullptr);

  y_ptr = y;
  ylabels_ptr = ylabels;
  tooltip_reflist_node = tooltip_list->head;
  while (tooltip_reflist_node != nullptr)
    {
      grm_tooltip_info_t *current_tooltip = tooltip_reflist_node->entry;
      unsigned int current_dist = (current_tooltip->x_px - mouse_x) * (current_tooltip->x_px - mouse_x) +
                                  (current_tooltip->y_px - mouse_y) * (current_tooltip->y_px - mouse_y);
      if (current_dist < min_dist)
        {
          nearest_tooltip = current_tooltip;
          min_dist = current_dist;
        }
      *y_ptr = current_tooltip->y;
      *ylabels_ptr = (strcmp(current_tooltip->label, "") == 0) ? (char *)"y" : current_tooltip->label;
      ++y_ptr;
      ++ylabels_ptr;
      tooltip_reflist_node = tooltip_reflist_node->next;
    }
  error_cleanup_if(nearest_tooltip == nullptr);
  *ylabels_ptr = nullptr; /* terminate the ylabels array with a nullptr pointer to simplify loops */

  accumulated_tooltip = static_cast<grm_accumulated_tooltip_info_t *>(malloc(sizeof(grm_accumulated_tooltip_info_t)));
  error_cleanup_if(accumulated_tooltip == nullptr);
  accumulated_tooltip->n = tooltip_list->size;
  accumulated_tooltip->x = nearest_tooltip->x;
  accumulated_tooltip->x_px = nearest_tooltip->x_px;
  accumulated_tooltip->xlabel = nearest_tooltip->xlabel;
  accumulated_tooltip->y = y;
  accumulated_tooltip->y_px = nearest_tooltip->y_px;
  accumulated_tooltip->ylabels = ylabels;

  if (tooltip_list != nullptr)
    {
      tooltip_reflist_node = tooltip_list->head;
      while (tooltip_reflist_node != nullptr)
        {
          free(tooltip_reflist_node->entry);
          tooltip_reflist_node = tooltip_reflist_node->next;
        }
      tooltip_reflist_delete(tooltip_list);
      tooltip_list = nullptr;
    }

  return accumulated_tooltip;

error_cleanup:
  free(y);
  free(ylabels);
  free(accumulated_tooltip);
  if (tooltip_list != nullptr)
    {
      tooltip_reflist_node = tooltip_list->head;
      while (tooltip_reflist_node != nullptr)
        {
          free(tooltip_reflist_node->entry);
          tooltip_reflist_node = tooltip_reflist_node->next;
        }
      tooltip_reflist_delete(tooltip_list);
      tooltip_list = nullptr;
    }

  return nullptr;
}

err_t get_tooltips(int mouse_x, int mouse_y, err_t (*tooltip_callback)(int, int, grm_tooltip_info_t *))
{
  double x, y, x_min, x_max, y_min, y_max, mindiff = DBL_MAX, diff;
  double x_range_min, x_range_max, y_range_min, y_range_max, x_px, y_px;
  int width, height, max_width_height;
  std::vector<std::string> labels;
  unsigned int num_labels = 0;
  std::string kind;
  unsigned int x_length, y_length, z_length, series_i = 0, i;
  std::string orientation;
  int is_vertical = 0;

  auto info = static_cast<grm_tooltip_info_t *>(malloc(sizeof(grm_tooltip_info_t)));
  return_error_if(info == nullptr, ERROR_MALLOC);
  info->x_px = -1;
  info->y_px = -1;
  info->x = 0;
  info->y = 0;
  info->xlabel = (char *)"x";
  info->ylabel = (char *)"y";
  info->label = (char *)"";

  GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
  max_width_height = grm_max(width, height);
  x = (double)mouse_x / max_width_height;
  y = (double)(height - mouse_y) / max_width_height;

  auto subplot_element = get_subplot_from_ndc_points_using_dom(1, &x, &y);

  if (subplot_element != nullptr)
    {
      kind = static_cast<std::string>(subplot_element->getAttribute("kind"));
      if (subplot_element->hasAttribute("orientation"))
        {
          orientation = static_cast<std::string>(subplot_element->getAttribute("orientation"));
          is_vertical = orientation == "vertical";
        }
    }
  if (subplot_element == nullptr ||
      !str_equals_any(kind, "line", "scatter", "stem", "stairs", "heatmap", "marginal_heatmap", "contour", "imshow",
                      "contourf", "pie", "hexbin", "shade", "quiver"))
    {
      tooltip_callback(mouse_x, mouse_y, info);
      return ERROR_NONE;
    }

  GRM::Render::processLimits(subplot_element);

  gr_savestate();
  auto central_region = subplot_element->querySelectors("central_region");
  GRM::Render::processWindow(central_region);
  if (central_region->hasAttribute("viewport_x_min"))
    {
      gr_setviewport(static_cast<double>(central_region->getAttribute("viewport_x_min")),
                     static_cast<double>(central_region->getAttribute("viewport_x_max")),
                     static_cast<double>(central_region->getAttribute("viewport_y_min")),
                     static_cast<double>(central_region->getAttribute("viewport_y_max")));
    }
  if (central_region->hasAttribute("window_x_min") && kind != "pie")
    {
      gr_setwindow(static_cast<double>(central_region->getAttribute("window_x_min")),
                   static_cast<double>(central_region->getAttribute("window_x_max")),
                   static_cast<double>(central_region->getAttribute("window_y_min")),
                   static_cast<double>(central_region->getAttribute("window_y_max")));
    }
  GRM::Render::calculateCharHeight(central_region);
  gr_setscale(static_cast<int>(subplot_element->getAttribute("scale")));
  gr_ndctowc(&x, &y);

  auto axes_vec = subplot_element->querySelectorsAll("axes");
  auto coordinate_system = subplot_element->querySelectors("coordinate_system");
  auto left_side_region = subplot_element->querySelectors("side_region[location=\"left\"]");
  auto bottom_side_region = subplot_element->querySelectors("side_region[location=\"bottom\"]");
  struct
  {
    std::shared_ptr<GRM::Element> x, y;
  } label_elements{};

  if (bottom_side_region && bottom_side_region->hasAttribute("text_content")) label_elements.x = bottom_side_region;
  if (left_side_region && left_side_region->hasAttribute("text_content")) label_elements.y = left_side_region;

  if (label_elements.x && label_elements.x->hasAttribute("text_content"))
    {
      static auto xlabel = static_cast<std::string>(label_elements.x->getAttribute("text_content"));
      info->xlabel = (char *)xlabel.c_str();
    }
  else
    {
      info->xlabel = (char *)"x";
    }
  if (label_elements.y && label_elements.y->hasAttribute("text_content"))
    {
      static auto ylabel = static_cast<std::string>(label_elements.y->getAttribute("text_content"));
      info->ylabel = (char *)ylabel.c_str();
    }
  else
    {
      info->ylabel = (char *)"y";
    }

  x_range_min = (double)(mouse_x - 50) / max_width_height;
  x_range_max = (double)(mouse_x + 50) / max_width_height;
  y_range_min = (double)(height - (mouse_y + 50)) / max_width_height;
  y_range_max = (double)(height - (mouse_y - 50)) / max_width_height;
  gr_ndctowc(&x_range_min, &y_range_min);
  gr_ndctowc(&x_range_max, &y_range_max);

  x_min = static_cast<double>(subplot_element->getAttribute("_x_lim_min"));
  x_max = static_cast<double>(subplot_element->getAttribute("_x_lim_max"));
  y_min = static_cast<double>(subplot_element->getAttribute("_y_lim_min"));
  y_max = static_cast<double>(subplot_element->getAttribute("_y_lim_max"));

  x_range_min = (x_min > x_range_min) ? x_min : x_range_min;
  y_range_min = (y_min > y_range_min) ? y_min : y_range_min;
  x_range_max = (x_max < x_range_max) ? x_max : x_range_max;
  y_range_max = (y_max < y_range_max) ? y_max : y_range_max;

  std::shared_ptr<GRM::Context> context = grm_get_render()->getContext();
  auto draw_legend_element_vec = subplot_element->querySelectorsAll("legend");
  num_labels = 0;
  if (!draw_legend_element_vec.empty())
    {
      auto &draw_legend_element = draw_legend_element_vec[0];
      std::string labels_key = static_cast<std::string>(draw_legend_element->getAttribute("labels"));
      labels = GRM::get<std::vector<std::string>>((*context)[labels_key]);
      num_labels = labels.size();
    }
  if (kind == "pie")
    {
      static char output[50];
      double max_x = 0.95, min_x = 0.05, max_y = 0.05, min_y = 0.95;
      int center_x, center_y;
      double radius;
      double start_angle, end_angle, act_angle;

      gr_wctondc(&max_x, &max_y);
      max_x = max_x * max_width_height;
      max_y = height - max_y * max_width_height;
      gr_wctondc(&min_x, &min_y);
      min_x = min_x * max_width_height;
      min_y = height - min_y * max_width_height;

      /* calculate the circle */
      radius = (max_x - min_x) / 2.0;
      center_x = (int)(max_x - radius);
      center_y = (int)(max_y - radius);

      auto current_series = subplot_element->querySelectorsAll("series_pie")[0];
      auto x_key = static_cast<std::string>(current_series->getAttribute("x"));
      std::vector<double> x_series_vec;
      x_series_vec = GRM::get<std::vector<double>>((*context)[x_key]);

      x_length = x_series_vec.size();
      std::vector<double> normalized_x(x_length);
      GRM::normalize_vec(x_series_vec, &normalized_x);
      start_angle = 90.0;
      for (i = 0; i < x_length; ++i)
        {
          end_angle = start_angle - normalized_x[i] * 360.0;
          act_angle =
              acos((mouse_x - center_x) / sqrt(pow(abs(mouse_x - center_x), 2) + pow(abs(mouse_y - center_y), 2)));
          act_angle = (mouse_y - center_y > 0) ? act_angle * 180 / M_PI : 360 - act_angle * 180 / M_PI;
          if (act_angle >= 270 && act_angle <= 360) act_angle = act_angle - 360;
          act_angle *= -1;
          if (start_angle >= act_angle && end_angle <= act_angle)
            {
              snprintf(output, 50, "%f", x_series_vec[i]);
            }
          start_angle = end_angle;
        }

      if (sqrt(pow(abs(mouse_x - center_x), 2) + pow(abs(mouse_y - center_y), 2)) <= radius)
        {
          mindiff = 0;
          info->x = 0;
          info->y = 0;
          info->x_px = mouse_x;
          info->y_px = mouse_y;
          info->label = output;
        }
      tooltip_callback(mouse_x, mouse_y, info);
      gr_restorestate();
      return ERROR_NONE;
    }
  else
    {
      auto current_series_group_vec = subplot_element->querySelectorsAll("series_" + kind);
      std::vector<std::shared_ptr<GRM::Element>> current_series_vec;
      if (kind != "marginal_heatmap")
        {
          for (const auto &current_series_group : current_series_group_vec)
            {
              if (str_equals_any(kind, "hexbin", "imshow", "contour", "contourf", "heatmap", "quiver", "shade"))
                {
                  current_series_vec.push_back(current_series_group);
                }
              else
                {
                  for (const auto &current_series_group_child : current_series_group->children())
                    {
                      current_series_vec.push_back(current_series_group_child);
                    }
                }
            }
        }
      else
        {
          current_series_vec.push_back(subplot_element->querySelectors("marginal_heatmap_plot"));
        }

      for (auto &current_series : current_series_vec)
        {
          std::string x_key_string = "x";
          std::string y_key_string = "y";
          std::string z_key_string = "z";
          std::string z_key;

          mindiff = DBL_MAX;

          if (kind == "contour" || kind == "contourf")
            {
              x_key_string = "px";
              y_key_string = "py";
              z_key_string = "pz";
            }

          if (!current_series->hasAttribute(x_key_string) || !current_series->hasAttribute(y_key_string))
            {
              continue;
            }
          auto x_key = static_cast<std::string>(current_series->getAttribute(x_key_string));
          auto y_key = static_cast<std::string>(current_series->getAttribute(y_key_string));

          if (!str_equals_any(kind, "hexbin", "quiver", "line", "scatter", "stem", "stairs", "shade"))
            {
              if (!current_series->hasAttribute(z_key_string))
                {
                  mindiff = DBL_MAX;
                  continue;
                }
              z_key = static_cast<std::string>(current_series->getAttribute(z_key_string));
            }

          std::vector<double> x_series_vec;
          std::vector<double> y_series_vec;

          if (is_vertical)
            {
              x_series_vec = GRM::get<std::vector<double>>((*context)[y_key]);
              y_series_vec = GRM::get<std::vector<double>>((*context)[x_key]);
            }
          else
            {
              x_series_vec = GRM::get<std::vector<double>>((*context)[x_key]);
              y_series_vec = GRM::get<std::vector<double>>((*context)[y_key]);
            }
          std::vector<double> z_series_vec;

          if (str_equals_any(kind, "heatmap", "marginal_heatmap", "contour", "imshow", "contourf"))
            {
              z_series_vec = GRM::get<std::vector<double>>((*context)[z_key]);
              z_length = z_series_vec.size();
            }

          if (!str_equals_any(kind, "heatmap", "marginal_heatmap", "contour", "imshow", "contourf", "quiver"))
            {
              for (i = 0; i < x_series_vec.size(); i++)
                {
                  if (x_series_vec[i] < x_range_min || x_series_vec[i] > x_range_max)
                    {
                      continue;
                    }
                  x_px = x_series_vec[i];
                  y_px = y_series_vec[i];
                  gr_wctondc(&x_px, &y_px);
                  x_px = (x_px * max_width_height);
                  y_px = (height - y_px * max_width_height);
                  diff = fabs(x_px - mouse_x);
                  if (diff < mindiff && diff <= MAX_MOUSE_DIST)
                    {
                      mindiff = diff;
                      info->x = x_series_vec[i];
                      info->y = y_series_vec[i];
                      info->x_px = (int)x_px;
                      info->y_px = (int)y_px;
                      if (num_labels > series_i)
                        {
                          static std::vector<std::string> line_label = labels;
                          info->label = (char *)line_label[series_i].c_str();
                        }
                      else
                        {
                          info->label = (char *)"";
                        }
                    }
                }
            }
          else
            {
              static char output[50];
              double num;
              x_length = x_series_vec.size();
              y_length = y_series_vec.size();
              double x_0 = x_series_vec[0], x_end = x_series_vec[x_length - 1], y_0 = y_series_vec[0],
                     y_end = y_series_vec[y_length - 1];
              double x_step, y_step, x_series_idx, y_series_idx;
              double *u_series, *v_series;
              std::vector<double> x_steps, y_steps;
              int x_offset = 0, y_offset = 0;

              if (static_cast<int>(subplot_element->getAttribute("x_log")))
                {
                  for (int j = 0; j < (int)x_length; j++)
                    {
                      if (!grm_isnan(x_series_vec[j]))
                        {
                          x_min = x_series_vec[j];
                          break;
                        }
                      x_offset += 1;
                    }
                  x_0 = x_min;
                }
              if (static_cast<int>(subplot_element->getAttribute("y_log")))
                {
                  for (int j = 0; j < (int)y_length; j++)
                    {
                      if (!grm_isnan(y_series_vec[j]))
                        {
                          y_min = y_series_vec[j];
                          break;
                        }
                      y_offset += 1;
                    }
                  y_0 = y_min;
                }
              if (kind == "imshow") x_0 = x_min, x_end = x_max, y_0 = y_min, y_end = y_max;

              gr_wctondc(&x_0, &y_0);
              gr_wctondc(&x_end, &y_end);
              x_0 = x_0 * max_width_height;
              x_end = x_end * max_width_height;
              y_0 = height - y_0 * max_width_height;
              y_end = height - y_end * max_width_height;

              x_step = (x_end - x_0) / x_length;
              y_step = (y_end - y_0) / y_length;

              if (static_cast<int>(subplot_element->getAttribute("x_log")))
                {
                  for (int j = 0; j < (int)x_length - x_offset; j++)
                    {
                      double step_x = (x_series_vec[x_length - 1] - x_min) / (x_length - x_offset);
                      double tmp = 0, a = x_min + j * step_x, b = x_min + (j + 1) * step_x;
                      gr_wctondc(&a, &tmp);
                      gr_wctondc(&b, &tmp);

                      a = a * max_width_height;
                      b = b * max_width_height;
                      x_steps.push_back(b - a);
                    }
                }
              if (static_cast<int>(subplot_element->getAttribute("y_log")))
                {
                  for (int j = 0; j < (int)y_length - y_offset; j++)
                    {
                      double step_y = (y_series_vec[y_length - 1] - y_min) / (y_length - y_offset);
                      double tmp = 0, a = y_min + j * step_y, b = y_min + (j + 1) * step_y;
                      gr_wctondc(&tmp, &a);
                      gr_wctondc(&tmp, &b);

                      a = a * max_width_height;
                      b = b * max_width_height;
                      y_steps.push_back(-(b - a));
                    }
                }

              if (kind == "quiver")
                {
                  auto u_key = static_cast<std::string>(current_series->getAttribute("u"));
                  auto v_key = static_cast<std::string>(current_series->getAttribute("v"));

                  auto u_series_vec = GRM::get<std::vector<double>>((*context)[u_key]);
                  auto v_series_vec = GRM::get<std::vector<double>>((*context)[v_key]);
                  u_series = &u_series_vec[0];
                  v_series = &v_series_vec[0];
                }

              mindiff = 0;
              x_series_idx = (mouse_x - x_0) / x_step;
              if (static_cast<int>(subplot_element->getAttribute("x_log")) &&
                  str_equals_any(kind, "heatmap", "marginal_heatmap", "contour", "contourf"))
                {
                  double tmp = 0;
                  x_series_idx = x_offset;
                  for (int j = 0; j < (int)x_length - x_offset; j++)
                    {
                      if (tmp + x_steps[j] > (mouse_x - x_0)) break;
                      tmp += x_steps[j];
                      x_series_idx += 1;
                    }
                }

              y_series_idx = (mouse_y - y_0) / y_step;
              if (static_cast<int>(subplot_element->getAttribute("y_log")) &&
                  str_equals_any(kind, "heatmap", "marginal_heatmap", "contour", "contourf"))
                {
                  double tmp = 0;
                  y_series_idx = y_offset;
                  for (int j = 0; j < (int)y_length - y_offset; j++)
                    {
                      if (tmp + y_steps[j] < (mouse_y - y_0)) break;
                      tmp += y_steps[j];
                      y_series_idx += 1;
                    }
                }

              if (x_series_idx < 0 || x_series_idx >= x_length || y_series_idx < 0 || y_series_idx >= y_length)
                {
                  mindiff = DBL_MAX;
                  break;
                }
              if (kind == "quiver")
                {
                  info->xlabel = (char *)"u";
                  info->ylabel = (char *)"v";
                  info->x = u_series[(int)(y_series_idx)*x_length + (int)(x_series_idx)];
                  info->y = v_series[(int)(y_series_idx)*x_length + (int)(x_series_idx)];
                }
              else
                {
                  info->x = x_series_vec[(int)x_series_idx];
                  info->y = y_series_vec[(int)y_series_idx];
                }
              info->x_px = mouse_x;
              info->y_px = mouse_y;

              if (kind == "quiver")
                {
                  info->label = (char *)"";
                }
              else
                {
                  num = (grm_isnan(z_series_vec[(int)y_series_idx * x_length + (int)x_series_idx]))
                            ? NAN
                            : z_series_vec[(int)y_series_idx * x_length + (int)x_series_idx];
                  snprintf(output, 50, "%f", num);
                  info->label = output;
                }
            }
          tooltip_callback(mouse_x, mouse_y, info);
          if (current_series != nullptr)
            {
              info = static_cast<grm_tooltip_info_t *>(malloc(sizeof(grm_tooltip_info_t)));
              info->x_px = -1;
              info->y_px = -1;
              info->x = 0;
              info->y = 0;
              if (label_elements.x && label_elements.x->hasAttribute("text_content"))
                {
                  static auto xlabel = static_cast<std::string>(label_elements.x->getAttribute("text_content"));
                  info->xlabel = (char *)xlabel.c_str();
                }
              else
                {
                  info->xlabel = (char *)"x";
                }
              if (label_elements.y && label_elements.y->hasAttribute("text_content"))
                {
                  static auto ylabel = static_cast<std::string>(label_elements.y->getAttribute("text_content"));
                  info->ylabel = (char *)ylabel.c_str();
                }
              else
                {
                  info->ylabel = (char *)"y";
                }
              info->label = (char *)"";
              return_error_if(info == nullptr, ERROR_MALLOC);
            }
          ++series_i;
        }
    }
  gr_restorestate();
  return ERROR_NONE;
}

int grm_get_hover_mode(int mouse_x, int mouse_y, int disable_movable_xform)
{
  if (!disable_movable_xform)
    {
      int width, height, max_width_height;
      GRM::Render::getFigureSize(&width, &height, nullptr, nullptr);
      max_width_height = grm_max(width, height);

      double ndc_x, ndc_y;
      ndc_x = (double)mouse_x / max_width_height;
      ndc_y = (double)(height - mouse_y) / max_width_height;

      auto subplot_element = get_subplot_from_ndc_point_using_dom(ndc_x, ndc_y);
      if (subplot_element != nullptr)
        {
          std::shared_ptr<GRM::Element> movable = nullptr;

          auto movable_elems = subplot_element->querySelectorsAll("[movable=1]");
          for (const auto &move_elem : movable_elems)
            {
              if (move_elem != nullptr)
                {
                  double bbox_x_min, bbox_x_max, bbox_y_min, bbox_y_max;

                  bbox_x_min = static_cast<double>(move_elem->getAttribute("_bbox_x_min"));
                  bbox_x_max = static_cast<double>(move_elem->getAttribute("_bbox_x_max"));
                  bbox_y_min = static_cast<double>(move_elem->getAttribute("_bbox_y_min"));
                  bbox_y_max = static_cast<double>(move_elem->getAttribute("_bbox_y_max"));
                  if (bbox_x_min <= mouse_x && bbox_x_max >= mouse_x && bbox_y_min <= mouse_y && bbox_y_max >= mouse_y)
                    {
                      if ((static_cast<std::string>(move_elem->getAttribute("name")) == "integral_left" ||
                           static_cast<std::string>(move_elem->getAttribute("name")) == "integral_right") &&
                          move_elem->localName() == "polyline")
                        {
                          return INTEGRAL_SIDE_HOVER_MODE;
                        }
                      return MOVABLE_HOVER_MODE;
                    }
                }
            }
        }
    }
  return DEFAULT_HOVER_MODE;
}
