#ifdef _WIN32
/*
 * Headers on Windows can define `min` and `max` as macros which causes
 * problem when using `std::min` and `std::max`
 * -> Define `NOMINMAX` to prevent the definition of these macros
 */
#define NOMINMAX
#endif

#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include <array>
#include <set>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cfloat>
#include <grm/dom_render/graphics_tree/element.hxx>
#include <grm/dom_render/graphics_tree/document.hxx>
#include <grm/dom_render/graphics_tree/value.hxx>
#include <grm/dom_render/graphics_tree/util.hxx>
#include <grm/dom_render/render.hxx>
#include <grm/dom_render/not_found_error.hxx>
#include <grm/dom_render/invalid_value_error.hxx>
#include <grm/dom_render/context.hxx>
#include <grm/dom_render/creator.hxx>
#include <grm/dom_render/updater.hxx>
#include <grm/dom_render/render_util.hxx>
#include <grm/dom_render/process_attributes.hxx>
#include <grm/dom_render/process_elements.hxx>
#include <grm/dom_render/casts.hxx>
#include "gks.h"
#include "gr.h"
#include "gr3.h"
#include "grm/layout.hxx"
#include "grm/plot_int.h"
#include "grm/utilcpp_int.hxx"
#include "grm/dom_render/manage_z_index.hxx"
#include "grm/dom_render/drawable.hxx"
#include "grm/dom_render/manage_gr_context_ids.hxx"
#include "grm/dom_render/manage_custom_color_index.hxx"


std::shared_ptr<GRM::Element> global_root;
std::shared_ptr<GRM::Element> active_figure;
std::shared_ptr<GRM::Render> global_render;
static std::shared_ptr<GRM::Creator> global_creator;
std::priority_queue<std::shared_ptr<Drawable>, std::vector<std::shared_ptr<Drawable>>, CompareZIndex> z_queue;
bool z_queue_is_being_rendered = false;
std::map<std::shared_ptr<GRM::Element>, int> parent_to_context;
ManageGRContextIds gr_context_id_manager;
ManageZIndex z_index_manager;
ManageCustomColorIndex custom_color_index_manager;
GRM::GroupMask group_mask;

static std::set<std::string> valid_context_keys = valid_context_attributes;

static int *previous_scatter_marker_type = plot_scatter_markertypes;
static int *previous_line_marker_type = plot_scatter_markertypes;

std::map<int, std::weak_ptr<GRM::Element>> &boundingMap()
{
  /* See the `id_pool` function above for a detailed explanation why this routine is needed. */
  static auto bounding_map = new std::map<int, std::weak_ptr<GRM::Element>>;
  return *bounding_map;
}

static bool automatic_update = false;
static bool redraw_ws = false;
static bool bounding_boxes = !getenv("GRDISPLAY") || (getenv("GRDISPLAY") && strcmp(getenv("GRDISPLAY"), "view") != 0);
static bool first_call = true;
static bool highlighted_attr_exist = false;
static const char *grm_tmp_dir = nullptr;
static bool enable_editor = false;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ utility functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

GRM::PushDrawableToZQueue::PushDrawableToZQueue(
    std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)> draw_function)
    : draw_function(std::move(draw_function))
{
  ;
}

void GRM::PushDrawableToZQueue::operator()(const std::shared_ptr<GRM::Element> &element,
                                           const std::shared_ptr<GRM::Context> &context)
{
  int context_id;
  auto parent = element->parentElement();

  if (auto search = parent_to_context.find(parent); search != parent_to_context.end())
    {
      context_id = search->second;
    }
  else
    {
      context_id = gr_context_id_manager.getUnusedGRContextId();
      gr_savecontext(context_id);
      parent_to_context[parent] = context_id;
    }
  auto drawable = std::make_shared<Drawable>(element, context, context_id, z_index_manager.getZIndex(), draw_function);
  drawable->insertion_index = (int)z_queue.size();
  custom_color_index_manager.saveContext(context_id);
  z_queue.push(drawable);
}

void bboxReceiveCallback(int id, double x_min, double x_max, double y_min, double y_max)
{
  if ((x_min == DBL_MAX || x_max == -DBL_MAX || y_min == DBL_MAX || y_max == -DBL_MAX) || boundingMap()[id].expired())
    return;
  auto element = boundingMap()[id].lock();
  element->setAttribute("_bbox_id", id);
  element->setAttribute("_bbox_x_min", x_min);
  element->setAttribute("_bbox_x_max", x_max);
  element->setAttribute("_bbox_y_min", y_min);
  element->setAttribute("_bbox_y_max", y_max);
}

void maskReceiveCallback(unsigned int width, unsigned int height, unsigned int *mask)
{
  group_mask.own(width, height, &mask);
}

void GRM::addValidContextKey(std::string key)
{
  valid_context_keys.emplace(key);
}

const GRM::GroupMask *GRM::getGroupMask()
{
  return &group_mask;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ render functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void renderHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Recursive helper function for render; Not part of render class
   * Only renders / processes children if the parent is in parent_types (group etc.)
   * Used for traversing the tree
   *
   * \param[in] element A GRM::Element
   * \param[in] context A GRM::Context
   */
  gr_savestate();
  z_index_manager.saveState();
  custom_color_index_manager.saveState();

  processElement(element, context);
  // needed for 3d cases to make sure gr_inqvpsize returns the correct width and height
  if (element->localName() == "figure" && redraw_ws && first_call && element->hasAttribute("active") &&
      static_cast<int>(element->getAttribute("active")))
    {
      gr_clearws();
      gr_updatews();
      first_call = false;
    }
  if (element->hasChildNodes() && parent_types.count(element->localName()))
    {
      for (const auto &child : element->children())
        {
          if (child->localName() == "figure" && !static_cast<int>(child->getAttribute("active"))) continue;
          if (child->localName() == "plot" &&
              (active_figure->querySelectors("plot[_active=\"1\"]") != nullptr ||
               active_figure->querySelectors("plot[_active_through_update=\"1\"]") != nullptr) &&
              ((!child->hasAttribute("_active") || !static_cast<int>(child->getAttribute("_active"))) &&
               ((!child->hasAttribute("_active_through_update") ||
                 !static_cast<int>(child->getAttribute("_active_through_update"))))))
            continue;
          renderHelper(child, context);
        }
    }

  custom_color_index_manager.restoreState();
  z_index_manager.restoreState();
  gr_restorestate();
}

static void missingBboxCalculator(const std::shared_ptr<GRM::Element> &element,
                                  const std::shared_ptr<GRM::Context> &context, double *bbox_xmin = nullptr,
                                  double *bbox_xmax = nullptr, double *bbox_ymin = nullptr, double *bbox_ymax = nullptr)
{
  int width, height;
  double m_width, m_height;
  double elem_bbox_xmin = DBL_MAX, elem_bbox_xmax = -DBL_MAX, elem_bbox_ymin = DBL_MAX, elem_bbox_ymax = -DBL_MAX;
  GRM::getFigureSize(&width, &height, &m_width, &m_height);

  if (element->hasAttribute("_bbox_id") && static_cast<int>(element->getAttribute("_bbox_id")) >= 0 &&
      !element->hasChildNodes())
    {
      *bbox_xmin = static_cast<double>(element->getAttribute("_bbox_x_min"));
      *bbox_xmax = static_cast<double>(element->getAttribute("_bbox_x_max"));
      *bbox_ymin = static_cast<double>(element->getAttribute("_bbox_y_min"));
      *bbox_ymax = static_cast<double>(element->getAttribute("_bbox_y_max"));
    }
  else
    {
      if (element->hasChildNodes() && parent_types.count(element->localName()))
        {
          for (const auto &child : element->children())
            {
              double tmp_bbox_xmin = DBL_MAX, tmp_bbox_xmax = -DBL_MAX, tmp_bbox_ymin = DBL_MAX,
                     tmp_bbox_ymax = -DBL_MAX;
              missingBboxCalculator(child, context, &tmp_bbox_xmin, &tmp_bbox_xmax, &tmp_bbox_ymin, &tmp_bbox_ymax);
              elem_bbox_xmin = grm_min(elem_bbox_xmin, tmp_bbox_xmin);
              elem_bbox_xmax = grm_max(elem_bbox_xmax, tmp_bbox_xmax);
              elem_bbox_ymin = grm_min(elem_bbox_ymin, tmp_bbox_ymin);
              elem_bbox_ymax = grm_max(elem_bbox_ymax, tmp_bbox_ymax);
            }
        }
    }

  if (element->localName() != "root" &&
      (!element->hasAttribute("_bbox_id") || static_cast<int>(element->getAttribute("_bbox_id")) < 0 ||
       element->hasChildNodes()))
    {
      if (!(elem_bbox_xmin == DBL_MAX || elem_bbox_xmax == -DBL_MAX || elem_bbox_ymin == DBL_MAX ||
            elem_bbox_ymax == -DBL_MAX))
        {
          if (element->hasAttribute("_bbox_id"))
            {
              /* In this case the element already has a negative (placeholder) bounding box id which can be reused by
                 turning into positive. */
              element->setAttribute("_bbox_id", -static_cast<int>(element->getAttribute("_bbox_id")));
            }
          else
            {
              element->setAttribute("_bbox_id", idPool().next());
            }

          elem_bbox_xmin = grm_max(0.0, elem_bbox_xmin);
          elem_bbox_xmax = grm_min(width, elem_bbox_xmax);
          elem_bbox_ymin = grm_max(0.0, elem_bbox_ymin);
          elem_bbox_ymax = grm_min(height, elem_bbox_ymax);

          if (element->hasAttribute("viewport_x_min") && element->hasAttribute("viewport_x_max") &&
              element->hasAttribute("viewport_y_min") && element->hasAttribute("viewport_y_max"))
            {
              double viewport[4];
              auto aspect_ratio = m_width / m_height;

              // get the visual viewport and not the real internal viewport which is getting used for the calculations
              viewport[0] = static_cast<double>(element->getAttribute("viewport_x_min"));
              viewport[1] = static_cast<double>(element->getAttribute("viewport_x_max"));
              viewport[2] = static_cast<double>(element->getAttribute("viewport_y_min"));
              viewport[3] = static_cast<double>(element->getAttribute("viewport_y_max"));

              elem_bbox_xmin = width * viewport[0] * (aspect_ratio < 1 ? 1.0 / aspect_ratio : 1.0);
              elem_bbox_xmax = width * viewport[1] * (aspect_ratio < 1 ? 1.0 / aspect_ratio : 1.0);
              elem_bbox_ymin = height * (1.0 - viewport[2] * (aspect_ratio > 1 ? aspect_ratio : 1.0));
              elem_bbox_ymax = height * (1.0 - viewport[3] * (aspect_ratio > 1 ? aspect_ratio : 1.0));

              if (elem_bbox_ymin > elem_bbox_ymax)
                {
                  auto tmp = elem_bbox_ymin;
                  elem_bbox_ymin = elem_bbox_ymax;
                  elem_bbox_ymax = tmp;
                }
            }

          element->setAttribute("_bbox_x_min", elem_bbox_xmin);
          element->setAttribute("_bbox_x_max", elem_bbox_xmax);
          element->setAttribute("_bbox_y_min", elem_bbox_ymin);
          element->setAttribute("_bbox_y_max", elem_bbox_ymax);
        }

      if (bbox_xmin != nullptr) *bbox_xmin = elem_bbox_xmin;
      if (bbox_xmax != nullptr) *bbox_xmax = elem_bbox_xmax;
      if (bbox_ymin != nullptr) *bbox_ymin = elem_bbox_ymin;
      if (bbox_ymax != nullptr) *bbox_ymax = elem_bbox_ymax;
    }
}

static void renderZQueue(const std::shared_ptr<GRM::Context> &context)
{
  z_queue_is_being_rendered = true;

  gr_savestate();
  for (; !z_queue.empty(); z_queue.pop())
    {
      const auto &drawable = z_queue.top();
      auto element = drawable->getElement();

      if (!element->parentElement()) continue;
      if (strEqualsAny(element->localName(), "tick", "text", "grid_line"))
        {
          auto coordinate_system = element->parentElement()->parentElement()->parentElement();
          if (coordinate_system != nullptr && coordinate_system->localName() == "coordinate_system" &&
              coordinate_system->hasAttribute("hide") && static_cast<int>(coordinate_system->getAttribute("hide")))
            continue;
        }

      if (bounding_boxes)
        {
          int bbox_id;
          if (element->hasAttribute("_bbox_id"))
            {
              bbox_id = std::abs(static_cast<int>(element->getAttribute("_bbox_id")));
            }
          else
            {
              bbox_id = idPool().next();
            }
          gr_setbboxcallback(bbox_id, &bboxReceiveCallback, enable_editor ? &maskReceiveCallback : nullptr);
          boundingMap()[bbox_id] = element;
        }

      custom_color_index_manager.selectContext(drawable->getGrContextId());
      drawable->draw();

      if (bounding_boxes) gr_cancelbboxcallback();
    }
  gr_context_id_manager.markAllIdsAsUnused();
  parent_to_context = {};
  gr_unselectcontext();
  gr_restorestate();
  z_queue_is_being_rendered = false;
}

static void initializeGridElements(const std::shared_ptr<GRM::Element> &element, GRM::Grid *grid)
{
  if (element->hasChildNodes())
    {
      for (const auto &child : element->children())
        {
          std::string prefix = "";
          int row_start, row_stop, col_start, col_stop;
          if (child->localName() != "layout_grid_element" && child->localName() != "layout_grid") return;

          auto abs_height = (child->hasAttribute("viewport_height_abs"))
                                ? static_cast<double>(child->getAttribute("viewport_height_abs"))
                                : -1.0;
          auto abs_width = (child->hasAttribute("viewport_width_abs"))
                               ? static_cast<double>(child->getAttribute("viewport_width_abs"))
                               : -1.0;
          auto relative_height = (child->hasAttribute("viewport_height_rel"))
                                     ? static_cast<double>(child->getAttribute("viewport_height_rel"))
                                     : -1.0;
          auto relative_width = (child->hasAttribute("viewport_width_rel"))
                                    ? static_cast<double>(child->getAttribute("viewport_width_rel"))
                                    : -1.0;
          auto aspect_ratio =
              (child->hasAttribute("aspect_ratio")) ? static_cast<double>(child->getAttribute("aspect_ratio")) : -1.0;
          auto fit_parents_height = static_cast<int>(child->getAttribute("fit_parents_height"));
          auto fit_parents_width = static_cast<int>(child->getAttribute("fit_parents_width"));
          if (child->localName() != "layout_grid") prefix = "_";
          row_start = static_cast<int>(child->getAttribute(prefix + "start_row"));
          row_stop = static_cast<int>(child->getAttribute(prefix + "stop_row"));
          col_start = static_cast<int>(child->getAttribute(prefix + "start_col"));
          col_stop = static_cast<int>(child->getAttribute(prefix + "stop_col"));
          auto *slice = new GRM::Slice(row_start, row_stop, col_start, col_stop);

          if (child->localName() == "layout_grid_element")
            {
              auto *cur_grid_element =
                  new GRM::GridElement(abs_height, abs_width, fit_parents_height, fit_parents_width, relative_height,
                                       relative_width, aspect_ratio);
              cur_grid_element->element_in_dom = child;
              grid->setElement(slice, cur_grid_element);
            }

          if (child->localName() == "layout_grid")
            {
              auto nrows = static_cast<int>(child->getAttribute("num_row"));
              auto ncols = static_cast<int>(child->getAttribute("num_col"));

              auto *cur_grid = new GRM::Grid(nrows, ncols, abs_height, abs_width, fit_parents_height, fit_parents_width,
                                             relative_height, relative_width, aspect_ratio);
              cur_grid->element_in_dom = child;
              grid->setElement(slice, cur_grid);
              initializeGridElements(child, cur_grid);
            }
        }
    }
}

static void finalizeGrid(const std::shared_ptr<GRM::Element> &figure)
{
  GRM::Grid *root_grid = nullptr;
  if (figure->hasChildNodes())
    {
      bool auto_update;
      global_render->getAutoUpdate(&auto_update);
      global_render->setAutoUpdate(false);
      for (const auto &child : figure->children())
        {
          if (child->localName() == "layout_grid")
            {
              auto n_rows = static_cast<int>(child->getAttribute("num_row"));
              auto n_cols = static_cast<int>(child->getAttribute("num_col"));
              root_grid = new GRM::Grid(n_rows, n_cols);
              child->setAttribute("viewport_normalized_x_min", 0.0);
              child->setAttribute("viewport_normalized_x_max", 1.0);
              child->setAttribute("viewport_normalized_y_min", 0.0);
              child->setAttribute("viewport_normalized_y_max", 1.0);

              initializeGridElements(child, root_grid);
              root_grid->finalizePlot();
              break;
            }
        }
      global_render->setAutoUpdate(auto_update);
    }
}

void GRM::Render::render(const std::shared_ptr<GRM::Document> &document,
                         const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * static GRM::Render::render receiving external document and context
   *
   * \param[in] document A GRM::Document that will be rendered
   * \param[in] ext_context A GRM::Context
   */
  global_root->setAttribute("_modified", false);
  if (auto root = document->firstChildElement(); root->hasChildNodes())
    {
      if (global_root->querySelectors("[_highlighted=\"1\"]"))
        highlighted_attr_exist = true;
      else
        highlighted_attr_exist = false;
      for (const auto &child : root->children())
        {
          gr_savestate();
          ::renderHelper(child, ext_context);
          gr_restorestate();
        }
    }
  global_root->setAttribute("_modified", false); // reset the modified flag, cause all updates are made
}

void GRM::Render::render(std::shared_ptr<GRM::Document> const &document)
{
  /*!
   * GRM::Render::render that receives an external document but uses the GRM::Render instance's context.
   *
   * \param[in] document A GRM::Document that will be rendered
   */
  global_root->setAttribute("_modified", false);
  if (auto root = document->firstChildElement(); root->hasChildNodes())
    {
      if (global_root->querySelectors("[_highlighted=\"1\"]"))
        highlighted_attr_exist = true;
      else
        highlighted_attr_exist = false;
      for (const auto &child : root->children())
        {
          gr_savestate();
          ::renderHelper(child, this->context);
          gr_restorestate();
        }
    }
  global_root->setAttribute("_modified", false); // reset the modified flag, cause all updates are made
}

void GRM::Render::render(const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   *GRM::Render::render uses GRM::Render instance's document and an external context
   *
   * \param[in] ext_context A GRM::Context
   */
  const auto root = this->firstChildElement();
  global_root->setAttribute("_modified", false);
  if (root->hasChildNodes())
    {
      if (global_root->querySelectors("[_highlighted=\"1\"]"))
        highlighted_attr_exist = true;
      else
        highlighted_attr_exist = false;
      for (const auto &child : root->children())
        {
          gr_savestate();
          ::renderHelper(child, ext_context);
          gr_restorestate();
        }
    }
  global_root->setAttribute("_modified", false); // reset the modified flag, cause all updates are made
}

void GRM::Render::render()
{
  /*!
   * GRM::Render::render uses both instance's document and context
   */
  const auto root = this->firstChildElement();
  global_root = root;
  if (root->hasChildNodes())
    {
      auto old_state = automatic_update;
      active_figure = this->firstChildElement()->querySelectorsAll("[active=1]")[0];
      const unsigned int indent = 2;

      redraw_ws = true;
      if (!global_render) GRM::Render::createRender();
      applyRootDefaults(root);
      if (loggerEnabled())
        {
          std::cerr << toXML(root, GRM::SerializerOptions{std::string(indent, ' '),
                                                          GRM::SerializerOptions::InternalAttributesFormat::PLAIN})
                    << "\n";
        }
      if (static_cast<int>(root->getAttribute("_clear_ws"))) gr_clearws();
      automatic_update = false;
      root->setAttribute("_modified", true);
      automatic_update = old_state;

      if (global_root->querySelectors("[_highlighted=\"1\"]"))
        highlighted_attr_exist = true;
      else
        highlighted_attr_exist = false;

      finalizeGrid(active_figure);
      renderHelper(root, this->context);
      renderZQueue(this->context);
      if (active_figure->hasAttribute("_kind_changed")) active_figure->removeAttribute("_kind_changed");
      automatic_update = false;
      root->setAttribute("_modified", false); // reset the modified flag, cause all updates are made
      automatic_update = old_state;
      if (root->hasAttribute("_update_ws") && static_cast<int>(root->getAttribute("_update_ws"))) gr_updatews();
      if (bounding_boxes) missingBboxCalculator(root, this->context);
      if (loggerEnabled())
        {
          std::cerr << toXML(root, GRM::SerializerOptions{std::string(indent, ' '),
                                                          GRM::SerializerOptions::InternalAttributesFormat::PLAIN})
                    << "\n";
          if (bounding_boxes) idPool().print(std::cerr, true);
        }
      redraw_ws = false;
      // reset marker types
      previous_scatter_marker_type = plot_scatter_markertypes;
      previous_line_marker_type = plot_scatter_markertypes;
    }
}

void GRM::Render::processTree()
{
  if (global_root->querySelectors("[_highlighted=\"1\"]"))
    highlighted_attr_exist = true;
  else
    highlighted_attr_exist = false;

  global_root->setAttribute("_modified", true);
  finalizeGrid(active_figure);
  renderHelper(global_root, this->context);
  renderZQueue(this->context);
  global_root->setAttribute("_modified", false); // reset the modified flag, cause all updates are made
}

static void highlightHelper(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  /*!
   * Recursive helper function for renderMaskHighlight; Not part of render class
   * Only renders / processes children if the parent is in parent_types (group etc.)
   * Used for traversing the tree
   *
   * \param[in] element A GRM::Element
   * \param[in] context A GRM::Context
   */
  gr_savestate();
  z_index_manager.saveState();
  custom_color_index_manager.saveState();

  processElement(element, context);
  if (element->hasChildNodes() && parent_types.count(element->localName()))
    {
      for (const auto &child : element->children())
        {
          if (child->localName() == "figure" && !static_cast<int>(child->getAttribute("active"))) continue;
          highlightHelper(child, context);
        }
    }

  custom_color_index_manager.restoreState();
  z_index_manager.restoreState();
  gr_restorestate();
}

static void processParents(const std::shared_ptr<GRM::Element> &element, const std::shared_ptr<GRM::Context> &context)
{
  if (element->localName() == "figure")
    {
      processElement(element, context);
      return;
    }
  processParents(element->parentElement(), context);
  processElement(element, context);
}

void GRM::Render::renderMaskHighlight(std::shared_ptr<GRM::Element> highlighted_element,
                                      void (*image_callback)(int, unsigned int, unsigned int, unsigned int,
                                                             unsigned int, unsigned int *))
{
  auto bbox_id = static_cast<int>(highlighted_element->getAttribute("_bbox_id"));
  auto old_state = automatic_update;
  const auto root = this->firstChildElement();
  global_root = root;
  active_figure = this->firstChildElement()->querySelectorsAll("[active=1]")[0];
  gr_savestate();
  applyRootDefaults(root);
  if (static_cast<int>(global_root->getAttribute("_clear_ws"))) gr_clearws();
  automatic_update = false;
  redraw_ws = true;
  finalizeGrid(active_figure);
  if (highlighted_element->localName() != "figure") processParents(highlighted_element, this->context);
  highlightHelper(highlighted_element, this->context);
  gr_beginpartial(bbox_id, image_callback);
  renderZQueue(this->context);
  gr_endpartial(bbox_id);
  automatic_update = old_state;
  if (root->hasAttribute("_update_ws") && static_cast<int>(root->getAttribute("_update_ws"))) gr_updatews();
  redraw_ws = false;
  gr_restorestate();
}

void GRM::Render::finalize()
{
  gr_context_id_manager.destroyGRContexts();
}

std::shared_ptr<GRM::Render> GRM::Render::createRender()
{
  /*!
   * This function can be used to create a Render object
   */
  global_render = std::shared_ptr<Render>(new Render());
  global_render->ownerDocument()->setUpdateFct(&renderCaller, &updateFilter);
  global_render->ownerDocument()->setContextFct(&deleteContextAttribute, &updateContextAttribute);
  global_render->ownerDocument()->setElementCleanupFct(&cleanupElement);
  global_creator = grm_get_creator();

  return global_render;
}

GRM::Render::Render()
{
  /*!
   * This is the constructor for GRM::Render
   */
  this->context = std::make_shared<GRM::Context>();
}

std::shared_ptr<GRM::Context> GRM::Render::getContext()
{
  return context;
}

/*
 * Searches in elementToTooltip for attribute_name and returns a string vector
 * containing:
 * [0] The default-value for this attribute
 * [1] The description for this attribute
 */
std::vector<std::string> GRM::Render::getDefaultAndTooltip(const std::shared_ptr<Element> &element,
                                                           const std::string &attribute_name)
{
  static std::unordered_map<std::string, std::vector<std::string>> attribute_to_tooltip{
      {std::string("abs_downwards_e"),
       std::vector<std::string>{"None", "A context reference for the absolute errors facing downwards"}},
      {std::string("abs_upwards_e"),
       std::vector<std::string>{"None", "A context reference for the absolute errors facing upwards"}},
      {std::string("active"), std::vector<std::string>{"None", "Determines whether the element is shown/active"}},
      {std::string("adjust_x_lim"),
       std::vector<std::string>{"1", "Determines whether the x-limits should be automatically adjusted"}},
      {std::string("adjust_y_lim"),
       std::vector<std::string>{"1", "Determines whether the y-limits should be automatically adjusted"}},
      {std::string("adjust_z_lim"),
       std::vector<std::string>{"1", "Determines whether the z-limits should be automatically adjusted"}},
      {std::string("algorithm"), std::vector<std::string>{"sum", "The algorithm used for the calculation"}},
      {std::string("ambient"),
       std::vector<std::string>{"0.2", "The ambient light. Values between 0.0 and 1.0 are valid"}},
      {std::string("angle_label"), std::vector<std::string>{"", "The angle labels for the theta axes"}},
      {std::string("angle_line_num"), std::vector<std::string>{"8", "The number of angle lines"}},
      {std::string("arc_label"), std::vector<std::string>{"", "The arc labels for the radial axes"}},
      {std::string("aspect_ratio"),
       std::vector<std::string>{
           "None", "Defines the aspect ratio of the layout element which will define its width and height"}},
      {std::string("axis_type"), std::vector<std::string>{"None", "Determines whether the axis is used for x or y"}},
      {std::string("bar_width"), std::vector<std::string>{"None", "The width of all bars"}},
      {std::string("bin_counts"), std::vector<std::string>{"None", "References the bin counts stored in the context"}},
      {std::string("bin_edges"), std::vector<std::string>{"None", "References the bin edges stored in the context"}},
      {std::string("bin_nr"), std::vector<std::string>{"None", "Specify the polar bar by a number"}},
      {std::string("bin_width"), std::vector<std::string>{"None", "The width all bins have"}},
      {std::string("bin_widths"), std::vector<std::string>{"None", "References the bin widths stored in the context"}},
      {std::string("bins"), std::vector<std::string>{"None", "References the bin-values stored in the context"}},
      {std::string("border_color_ind"),
       std::vector<std::string>{"0", "Sets the color of the markers border according to the current colormap"}},
      {std::string("border_width"), std::vector<std::string>{"None", "Sets the width of the markers border"}},
      {std::string("c"), std::vector<std::string>{"None", "References the color-values stored in the context"}},
      {std::string("c_lim_max"), std::vector<std::string>{"NAN", "The upper color-limit"}},
      {std::string("c_lim_min"), std::vector<std::string>{"NAN", "The lower color-limit"}},
      {std::string("c_range_max"), std::vector<std::string>{"None", "The upper color-value"}},
      {std::string("c_range_min"), std::vector<std::string>{"None", "The lower color-value"}},
      {std::string("cap_x_max"), std::vector<std::string>{"None", "The maximum x-value for the error cap"}},
      {std::string("cap_x_min"), std::vector<std::string>{"None", "The minimum x-value for the error cap"}},
      {std::string("char_height"), std::vector<std::string>{"None", "The height of the characters"}},
      {std::string("char_up_x"),
       std::vector<std::string>{"None", "X component of the character up vector. Used to rotate the text"}},
      {std::string("char_up_y"),
       std::vector<std::string>{"None", "Y Component of the character up vector. Used to rotate the text"}},
      {std::string("clip_negative"),
       std::vector<std::string>{"False",
                                "Determines whether negative radii are clipped, otherwise they will be mirrored"}},
      {std::string("clip_region"),
       std::vector<std::string>{
           "0", "Defines whether a rectangular(0) or elliptic(1) clip region will be used to clip the displayed plot"}},
      {std::string("col_span"),
       std::vector<std::string>{"None", "Define the number of columns the grid element contains"}},
      {std::string("color_ind_values"),
       std::vector<std::string>{"None", "References the color-values stored in the context in index format"}},
      {std::string("color_model"),
       std::vector<std::string>{"None", "The used color model for the image. Valid options are hsv and rgb"}},
      {std::string("color_rgb"),
       std::vector<std::string>{"None", "References the color-value stored in the context in rgb format"}},
      {std::string("color_rgb_values"),
       std::vector<std::string>{"None", "References the color-values stored in the context in rgb format"}},
      {std::string("colored"), std::vector<std::string>{"1", "Determines whether the quiver plot is shown in color"}},
      {std::string("colormap"), std::vector<std::string>{"viridis", "Sets the current colormap"}},
      {std::string("colormap_inverted"),
       std::vector<std::string>{"0", "Determines whether the colormap should be inverted"}},
      {std::string("count"), std::vector<std::string>{"None", "The count value of a polar bar"}},
      {std::string("counts"),
       std::vector<std::string>{"None", "References the polar histogram counts stored in the context"}},
      {std::string("d_max"),
       std::vector<std::string>{
           "None", "Used as maximum data value when applying the colormap. If it is negative, the variable will be set "
                   "to the actual occurring maximum and that value will be used instead"}},
      {std::string("d_min"),
       std::vector<std::string>{
           "None", "Used as minimum data value when applying the colormap. If it is negative, the variable will be set "
                   "to the actual occurring maximum and that value will be used instead"}},
      {std::string("data"),
       std::vector<std::string>{"None",
                                "References the data values in the context which will be displayed in the graphic"}},
      {std::string("diffuse"),
       std::vector<std::string>{"0.8", "The diffuse light. Values between 0.0 and 1.0 are valid"}},
      {std::string("disable_x_trans"),
       std::vector<std::string>{
           "0", "Determines whether the parameters for movable transformation in x direction are ignored"}},
      {std::string("disable_y_trans"),
       std::vector<std::string>{
           "0", "Determines whether the parameters for movable transformation in y direction are ignored"}},
      {std::string("downwards_cap_color"),
       std::vector<std::string>{"None", "The index based color-value for the downwards caps"}},
      {std::string("downwards_e"), std::vector<std::string>{"None", "The y-value for the caps of the downward error"}},
      {std::string("draw_edges"),
       std::vector<std::string>{
           "0", "Used in combination with theta- and r-colormap to determine whether edges should be drawn"}},
      {std::string("draw_grid"), std::vector<std::string>{"None", "Determines whether the axis has grid lines or not"}},
      {std::string("edge_width"), std::vector<std::string>{"None", "The width of all edges"}},
      {std::string("element_type"), std::vector<std::string>{"None", "The type of the overlay element"}},
      {std::string("end_angle"), std::vector<std::string>{"None", "The end angle of the element"}},
      {std::string("error_bar_color"),
       std::vector<std::string>{"None", "The color-value for the error bar excluding the caps"}},
      {std::string("error_bar_x"), std::vector<std::string>{"None", "The x-value for the error"}},
      {std::string("error_bar_y_max"), std::vector<std::string>{"None", "The upper y-value for the error"}},
      {std::string("error_bar_y_min"), std::vector<std::string>{"None", "The lower y-value for the error"}},
      {std::string("fill_color_ind"), std::vector<std::string>{"None", "Sets the current fill color in index format"}},
      {std::string("fill_color_rgb"), std::vector<std::string>{"None", "Sets the current fill color in RGB format"}},
      {std::string("fill_int_style"),
       std::vector<std::string>{"None", "Sets the index of the current fill interior style"}},
      {std::string("fill_style"),
       std::vector<std::string>{"None", "If the fill_int_style is set to hatch or pattern the fill style defines which "
                                        "pattern or hatch should be used"}},
      {std::string("fit_parents_height"),
       std::vector<std::string>{"None", "Toggle if the parent grid should match the element`s height"}},
      {std::string("fit_parents_width"),
       std::vector<std::string>{"None", "Toggle if the parent grid should match the element`s width"}},
      {std::string("flip_col_and_row"),
       std::vector<std::string>{"False", "Define if the rows and cols inside the layout should be flipped"}},
      {std::string("font"), std::vector<std::string>{"computermodern", "The used text font"}},
      {std::string("font_precision"), std::vector<std::string>{"precision_outline", "The precision of the text font"}},
      {std::string("height"), std::vector<std::string>{"None", "The height of the element"}},
      {std::string("height_abs"),
       std::vector<std::string>{"None",
                                "The absolut height of the imported graphic. The value has to be between 0.0 and 1.0"}},
      {std::string("hide"), std::vector<std::string>{"1", "Determines whether the element will be visible or not"}},
      {std::string("indices"),
       std::vector<std::string>{"None",
                                "References the bars which are calculated as inner bars stored in the context"}},
      {std::string("int_lim_high"), std::vector<std::string>{"None", "Sets the upper-limit for the integral"}},
      {std::string("int_lim_low"), std::vector<std::string>{"0", "Sets the lower-limit for the integral"}},
      {std::string("int_limits_high"),
       std::vector<std::string>{"None", "References the upper integral-limits stored in the context"}},
      {std::string("int_limits_low"),
       std::vector<std::string>{"None", "References the lower integral-limits stored in the context"}},
      {std::string("is_major"), std::vector<std::string>{"None", "Determines whether the tick is a major tick"}},
      {std::string("is_mirrored"), std::vector<std::string>{"None", "Determines whether the tick is mirrored"}},
      {std::string("isovalue"), std::vector<std::string>{"0.5", "The used isovalue"}},
      {std::string("keep_aspect_ratio"),
       std::vector<std::string>{"1", "Determines whether the aspect ratio should be kept"}},
      {std::string("keep_radii_axes"),
       std::vector<std::string>{"False",
                                "Determines whether the radii axes respect the ranges defined by r_lim_max/min"}},
      {std::string("keep_size_if_swapped"),
       std::vector<std::string>{"True", "If the position of this layout grid element is changed it's size is kept"}},
      {std::string("keep_window"),
       std::vector<std::string>{"1", "Determines whether the window will be inflicted by attribute changes"}},
      {std::string("kind"),
       std::vector<std::string>{
           "None", "Defines which kind the displayed series has. Depending on the set kind the kind can be changed"}},
      {std::string("label"),
       std::vector<std::string>{"None", "The series label which will be displayed in the legend"}},
      {std::string("label_pos"),
       std::vector<std::string>{"None", "The offset from the axis where the label should be placed"}},
      {std::string("labels"),
       std::vector<std::string>{"None", "The labels for all pie segments which will be displayed in the legend"}},
      {std::string("levels"), std::vector<std::string>{"20", "Number of contour levels"}},
      {std::string("line_color_ind"), std::vector<std::string>{"1", "Color for the lines in index format"}},
      {std::string("line_color_rgb"), std::vector<std::string>{"None", "Color for the lines in rgb format"}},
      {std::string("line_spec"), std::vector<std::string>{"", "Sets the string specifier for line styles"}},
      {std::string("line_type"), std::vector<std::string>{"None", "The type of the line"}},
      {std::string("line_width"), std::vector<std::string>{"None", "The width of the line"}},
      {std::string("location"), std::vector<std::string>{"None", "The elements location"}},
      {std::string("major_count"), std::vector<std::string>{"None", "Defines the how many tick is a major tick"}},
      {std::string("major_h"),
       std::vector<std::string>{
           "0 or 1000",
           "Determines whether contour lines are labeled. An offset of 1000 will colors the contour lines"}},
      {std::string("marginal_heatmap_kind"), std::vector<std::string>{"all", "The marginal heatmap kind (all, line)"}},
      {std::string("marginal_heatmap_side_plot"),
       std::vector<std::string>{"None",
                                "Used in marginal heatmap children to specify that the viewport and window from "
                                "the marginal heatmap is used to calculate the ones of the current element"}},
      {std::string("marker_color_ind"),
       std::vector<std::string>{"989", "Sets the color of the marker according to the current colormap"}},
      {std::string("marker_size"), std::vector<std::string>{"None", "Sets the size of the displayed markers"}},
      {std::string("marker_sizes"),
       std::vector<std::string>{"None", "References the marker sizes stored in the context"}},
      {std::string("marker_type"), std::vector<std::string>{"None", "Sets the marker type"}},
      {std::string("max_value"), std::vector<std::string>{"None", "The maximum-value of the axis"}},
      {std::string("max_y_length"), std::vector<std::string>{"None", "The maximum y length inside the barplot"}},
      {std::string("min_value"), std::vector<std::string>{"None", "The minimum-value of the axis"}},
      {std::string("mirrored_axis"), std::vector<std::string>{"0", "Determines whether the axis should be mirrored"}},
      {std::string("movable"),
       std::vector<std::string>{"0", "Determines whether the element can be moved via interaction. This attribute "
                                     "allows to only move certain parts"}},
      {std::string("name"), std::vector<std::string>{"None", "The name of the element"}},
      {std::string("num_bins"), std::vector<std::string>{"None", "Number of bins"}},
      {std::string("num_col"), std::vector<std::string>{"None", "Number of columns"}},
      {std::string("num_color_values"), std::vector<std::string>{"None", "Number of displayed color-values"}},
      {std::string("num_row"), std::vector<std::string>{"None", "Number of rows"}},
      {std::string("num_tick_labels"), std::vector<std::string>{"None", "Number of tick labels"}},
      {std::string("num_ticks"), std::vector<std::string>{"None", "Number of ticks"}},
      {std::string("norm"), std::vector<std::string>{"None", "Specify the used normalisation"}},
      {std::string("only_square_aspect_ratio"),
       std::vector<std::string>{"0", "Determines whether the aspect ratio must be square and should be retained"}},
      {std::string("orientation"),
       std::vector<std::string>{"horizontal", "The orientation of all elements. Only works for some 2D kinds."}},
      {std::string("origin"), std::vector<std::string>{"None", "The origin of the axis. Needed if org != min_value"}},
      {std::string("theta"), std::vector<std::string>{"None", "References the theta angles stored in the context"}},
      {std::string("theta_dim"), std::vector<std::string>{"None", "The dimension of the theta angles"}},
      {std::string("theta_max"), std::vector<std::string>{"None", "The upper theta angle of the polar cell array"}},
      {std::string("theta_min"), std::vector<std::string>{"None", "The lower theta angle of the polar cell array"}},
      {std::string("theta_flip"),
       std::vector<std::string>{"0", "Determines whether the theta angles should be flipped"}},
      {std::string("theta_lim_max"), std::vector<std::string>{"None", "The upper theta-limit"}},
      {std::string("theta_lim_min"), std::vector<std::string>{"None", "The lower theta-limit"}},
      {std::string("plot_group"),
       std::vector<std::string>{"None", "The plot group. Its used when more than one plot exists in the tree"}},
      {std::string("plot_type"), std::vector<std::string>{"None", "The type of the plot. It can be 2D, 3D or polar"}},
      {std::string("polar_with_pan"),
       std::vector<std::string>{"False", "Determines whether panning is allowed on polar plots"}},
      {std::string("pos"),
       std::vector<std::string>{
           "None", "F.e. where the x axis should be placed in relation to the y axis (position on the y axis)"}},
      {std::string("position"),
       std::vector<std::string>{"None", "Defines the position of a grid_layout element. The first numbers defines the "
                                        "row the second the column. Between both numbers must stand a space."}},
      {std::string("px"), std::vector<std::string>{"None", "References the px-values stored in the context. The "
                                                           "px-values are the modified version of the x-values"}},
      {std::string("py"), std::vector<std::string>{"None", "References the py-values stored in the context. The "
                                                           "py-values are the modified version of the y-values"}},
      {std::string("pz"), std::vector<std::string>{"None", "References the pz-values stored in the context. The "
                                                           "pz-values are the modified version of the z-values"}},
      {std::string("r"), std::vector<std::string>{"None", "References the radius-values stored in the context"}},
      {std::string("r_dim"), std::vector<std::string>{"None", "The dimension of the radius-values"}},
      {std::string("r_lim_max"), std::vector<std::string>{"None", "The upper radius-limit"}},
      {std::string("r_lim_min"), std::vector<std::string>{"None", "The lower radius-limit"}},
      {std::string("r_log"), std::vector<std::string>{"0", "Set if the r-values are logarithmic"}},
      {std::string("r_range_max"), std::vector<std::string>{"None", "The upper radius-value"}},
      {std::string("r_range_min"), std::vector<std::string>{"None", "The lower radius-value"}},
      {std::string("r_max"), std::vector<std::string>{"None", "The upper-value for the radius"}},
      {std::string("r_min"), std::vector<std::string>{"None", "The lower-value for the radius"}},
      {std::string("ref_x_axis_location"), std::vector<std::string>{"x", "The by the series referenced x axis"}},
      {std::string("ref_y_axis_location"), std::vector<std::string>{"y", "The by the series referenced y axis"}},
      {std::string("rel_downwards_e"),
       std::vector<std::string>{"None", "A context reference for the relative errors facing downward"}},
      {std::string("rel_upwards_e"),
       std::vector<std::string>{"None", "A context reference for the relative errors facing upwards"}},
      {std::string("resample_method"), std::vector<std::string>{"None", "The used resample method"}},
      {std::string("row_span"),
       std::vector<std::string>{"None", "Define the number of rows the grid element contains"}},
      {std::string("scale"), std::vector<std::string>{"None", "The set scale"}},
      {std::string("scientific_format"),
       std::vector<std::string>{"None", "Set the used format which will determine how a specific text will be drawn. "
                                        "The text can be plain (default) or for example interpreted with LaTeX"}},
      {std::string("select_specific_xform"),
       std::vector<std::string>{
           "None", "Selects a predefined transformation from world-coordinates to normalized device-coordinates"}},
      {std::string("series_index"), std::vector<std::string>{"None", "The index of the inner series"}},
      {std::string("set_text_color_for_background"),
       std::vector<std::string>{"False", "0 or 1. If this flag is true, the text color will be changed depending on "
                                         "the color of the background to increase the contrast"}},
      {std::string("size_x"), std::vector<std::string>{"None", "The figure width"}},
      {std::string("size_x_type"), std::vector<std::string>{"double", "The figure width type (integer, double, ...)"}},
      {std::string("size_x_unit"), std::vector<std::string>{"px", "The figure width unit (px, ...)"}},
      {std::string("size_y"), std::vector<std::string>{"None", "The figure height"}},
      {std::string("size_y_type"), std::vector<std::string>{"double", "The figure height type (integer, double, ...)"}},
      {std::string("size_y_unit"), std::vector<std::string>{"px", "The figure height unit (px, ...)"}},
      {std::string("space_rotation"), std::vector<std::string>{"0", "The rotation for space"}},
      {std::string("space_tilt"), std::vector<std::string>{"90", "The tilt for space"}},
      {std::string("space_z_max"), std::vector<std::string>{"None", "The upper z-coordinate for space"}},
      {std::string("space_z_min"), std::vector<std::string>{"None", "The lower z-coordinate for space"}},
      {std::string("space_3d_camera_distance"),
       std::vector<std::string>{"None", "The camera distance for the 3D space"}},
      {std::string("space_3d_fov"),
       std::vector<std::string>{
           "None", "The field of view for the 3D space. If the fov is NaN or 0 an orthographic projection will be "
                   "used, while every other value will apply a perspective projection to the displayed plot"}},
      {std::string("space_3d_phi"), std::vector<std::string>{"40.0", "The phi angle for the 3D space"}},
      {std::string("space_3d_theta"), std::vector<std::string>{"60.0", "The theta angle for the 3D space"}},
      {std::string("specular"),
       std::vector<std::string>{"0.7", "The specular light. Values between 0.0 and 1.0 are valid"}},
      {std::string("specular_power"), std::vector<std::string>{"128", "The specular light power"}},
      {std::string("stairs"),
       std::vector<std::string>{"0", "This is a format of the polar histogram where only outline edges are drawn"}},
      {std::string("start_angle"), std::vector<std::string>{"None", "The start angle of the element"}},
      {std::string("start_col"), std::vector<std::string>{"None", "The start column"}},
      {std::string("start_row"), std::vector<std::string>{"None", "The start row"}},
      {std::string("step_where"), std::vector<std::string>{"None", "Sets where the next stair step should start"}},
      {std::string("stop_col"), std::vector<std::string>{"None", "The stop column"}},
      {std::string("stop_row"), std::vector<std::string>{"None", "The stop row"}},
      {std::string("style"), std::vector<std::string>{"default", "The barplot style (default, lined, stacked)"}},
      {std::string("text"), std::vector<std::string>{"None", "The text displayed by this element"}},
      {std::string("text_align_horizontal"),
       std::vector<std::string>{
           "None", "The horizontal text alignment. Defines where the horizontal anker point of the test is placed"}},
      {std::string("text_align_vertical"),
       std::vector<std::string>{
           "None", "The vertical text alignment. Defines where the vertical anker point of the test is placed"}},
      {std::string("text_color_ind"), std::vector<std::string>{"None", "The index of the text color"}},
      {std::string("text_encoding"), std::vector<std::string>{"utf8", "The internal text encoding"}},
      {std::string("text_x0"), std::vector<std::string>{"None", "The left x position of the text"}},
      {std::string("text_y0"), std::vector<std::string>{"None", "The left y position of the text"}},
      {std::string("theta"), std::vector<std::string>{"None", "References the theta-values stored in the context"}},
      {std::string("theta_dim"), std::vector<std::string>{"None", "The dimension of the theta-values"}},
      {std::string("theta_data_lim_max"), std::vector<std::string>{"None", "The upper theta limit only for the data"}},
      {std::string("theta_data_lim_min"), std::vector<std::string>{"None", "The lower theta limit only for the data"}},
      {std::string("theta_flip"), std::vector<std::string>{"0", "Set if the theta-values gets flipped"}},
      {std::string("theta_lim_max"), std::vector<std::string>{"None", "The upper theta limit"}},
      {std::string("theta_lim_min"), std::vector<std::string>{"None", "The lower theta limit"}},
      {std::string("theta_range_max"), std::vector<std::string>{"None", "The upper theta value"}},
      {std::string("theta_range_min"), std::vector<std::string>{"None", "The lower theta value"}},
      {std::string("tick"), std::vector<std::string>{"None", "The polar ticks or the interval between minor ticks"}},
      {std::string("tick_label"), std::vector<std::string>{"", "The label which will be placed next to the tick"}},
      {std::string("tick_orientation"), std::vector<std::string>{"None", "The orientation of the axes ticks"}},
      {std::string("tick_size"), std::vector<std::string>{"0.005", "The size of the ticks"}},
      {std::string("title"), std::vector<std::string>{"None", "The plot title"}},
      {std::string("transformation"), std::vector<std::string>{"histogram_equalized", "The used transformation"}},
      {std::string("transparency"),
       std::vector<std::string>{"None", "Sets the transparency-value. Valid values are between 0.0 and 1.0"}},
      {std::string("trim_col"),
       std::vector<std::string>{"False", "Define if the program should remove empty cells columnwise inside the layout "
                                         "grid if this reduces the number of columns"}},
      {std::string("trim_row"),
       std::vector<std::string>{"False", "Define if the program should remove empty cells rowwise inside the layout "
                                         "grid if this reduces the number of rows"}},
      {std::string("u"), std::vector<std::string>{"None", "References the u-values stored in the context"}},
      {std::string("uniform_abs_downwards_e"),
       std::vector<std::string>{"None",
                                "The uniform absolute error facing downwards. It is applied at all error points"}},
      {std::string("uniform_abs_upwards_e"),
       std::vector<std::string>{"None",
                                "The uniform absolute error facing upwards. It is applied at all error points"}},
      {std::string("uniform_rel_downwards_e"),
       std::vector<std::string>{"None",
                                "The uniform relative error facing downwards. It is applied at all error points"}},
      {std::string("uniform_rel_upwards_e"),
       std::vector<std::string>{"None",
                                "The uniform relative error facing upwards. It is applied at all error points"}},
      {std::string("upwards_cap_color"),
       std::vector<std::string>{"None", "The index based color-value for the upwards caps"}},
      {std::string("upwards_e"), std::vector<std::string>{"None", "The y-value for the caps of the upward error"}},
      {std::string("use_gr3"),
       std::vector<std::string>{"1", "Determines whether the GR3 or the GR surface should be used"}},
      {std::string("use_grplot_changes"),
       std::vector<std::string>{
           "0", "If set some aspects in the code gets handled different which should lead to more uniform results. "
                "This could change some aspects in the displayed plot if compared to GR"}},
      {std::string("v"), std::vector<std::string>{"None", "References the v-values stored in the context"}},
      {std::string("value"), std::vector<std::string>{"None", "The value/number of the tick"}},
      {std::string("viewport_height_abs"),
       std::vector<std::string>{"None",
                                "Absolut viewport height as a percentage. Values between 0.0 and 1.0 are valid"}},
      {std::string("viewport_height_rel"),
       std::vector<std::string>{"None", "Viewport height as a percentage in relation to the other plots with relative "
                                        "height. Values between 0.0 and 1.0 are valid"}},
      {std::string("viewport_normalized_x_max"),
       std::vector<std::string>{"None", "The upper normalized viewport x-coordinate"}},
      {std::string("viewport_normalized_x_min"),
       std::vector<std::string>{"None", "The lower normalized viewport x-coordinate"}},
      {std::string("viewport_normalized_y_max"),
       std::vector<std::string>{"None", "The upper normalized viewport y-coordinate"}},
      {std::string("viewport_normalized_y_min"),
       std::vector<std::string>{"None", "The lower normalized viewport y-coordinate"}},
      {std::string("viewport_offset"), std::vector<std::string>{"None", "The offset for the side region viewport"}},
      {std::string("viewport_x_max"), std::vector<std::string>{"None", "The upper viewport x-coordinate"}},
      {std::string("viewport_x_min"), std::vector<std::string>{"None", "The lower viewport x-coordinate"}},
      {std::string("viewport_y_max"), std::vector<std::string>{"None", "The upper viewport y-coordinate"}},
      {std::string("viewport_y_min"), std::vector<std::string>{"None", "The lower viewport y-coordinate"}},
      {std::string("viewport_width_abs"),
       std::vector<std::string>{"None",
                                "Absolute viewport width as a percentage. Values between 0.0 and 1.0 are valid"}},
      {std::string("viewport_width_rel"),
       std::vector<std::string>{"None", "Viewport width as a percentage in relation to the other plots with relative "
                                        "width. Values between 0.0 and 1.0 are valid"}},
      {std::string("weights"),
       std::vector<std::string>{
           "None", "References the weights stored in the context which get applied to the histogram bins"}},
      {std::string("width"),
       std::vector<std::string>{"None", "The width of the side region element - inflicting the viewport"}},
      {std::string("width_abs"),
       std::vector<std::string>{"None",
                                "The absolut width of the imported graphic. The value has to be between 0 and 1."}},
      {std::string("window_x_max"), std::vector<std::string>{"None", "The upper window x-coordinate"}},
      {std::string("window_x_min"), std::vector<std::string>{"None", "The lower window x-coordinate"}},
      {std::string("window_y_max"), std::vector<std::string>{"None", "The upper window y-coordinate"}},
      {std::string("window_y_min"), std::vector<std::string>{"None", "The lower window y-coordinate"}},
      {std::string("window_z_max"), std::vector<std::string>{"None", "The upper window z-coordinate"}},
      {std::string("window_z_min"), std::vector<std::string>{"None", "The lower window z-coordinate"}},
      {std::string("world_coordinates"), std::vector<std::string>{"None", "The used world space (ndc or wc)"}},
      {std::string("ws_viewport_x_max"),
       std::vector<std::string>{"None", "The upper workstation viewport x-coordinate"}},
      {std::string("ws_viewport_x_min"),
       std::vector<std::string>{"None", "The lower workstation viewport x-coordinate"}},
      {std::string("ws_viewport_y_max"),
       std::vector<std::string>{"None", "The upper workstation viewport y-coordinate"}},
      {std::string("ws_viewport_y_min"),
       std::vector<std::string>{"None", "The lower workstation viewport y-coordinate"}},
      {std::string("ws_window_x_max"), std::vector<std::string>{"None", "The lower workstation window x-coordinate"}},
      {std::string("ws_window_x_min"), std::vector<std::string>{"None", "The upper workstation window x-coordinate"}},
      {std::string("ws_window_y_max"), std::vector<std::string>{"None", "The lower workstation window y-coordinate"}},
      {std::string("ws_window_y_min"), std::vector<std::string>{"None", "The upper workstation window y-coordinate"}},
      {std::string("x"), std::vector<std::string>{"None", "References the x-values stored in the context"}},
      {std::string("x_bins"), std::vector<std::string>{"1200", "Bins in x direction"}},
      {std::string("x_dim"), std::vector<std::string>{"None", "The dimension of the x-values"}},
      {std::string("x_flip"), std::vector<std::string>{"0", "Determines whether the x axis should be flipped"}},
      {std::string("x_grid"), std::vector<std::string>{"1", "Determines whether a x grid is shown"}},
      {std::string("x_ind"),
       std::vector<std::string>{" 1", "An index which is used to highlight a specific x position"}},
      {std::string("x_label"), std::vector<std::string>{"None", "The label of the x axis"}},
      {std::string("x_label_3d"), std::vector<std::string>{"None", "The label of the 3D x axis"}},
      {std::string("x_lim_max"), std::vector<std::string>{"None", "The upper x-limit"}},
      {std::string("x_lim_min"), std::vector<std::string>{"None", "The lower x-limit"}},
      {std::string("x_log"), std::vector<std::string>{"0", "Determines whether the x-values are logarithmic"}},
      {std::string("x_major"),
       std::vector<std::string>{"5", "Unitless integer-values specifying the number of minor tick intervals "
                                     "between major tick marks. Values of 0 or 1 imply no minor ticks. Negative "
                                     "values specify no labels will be drawn for the x axis"}},
      {std::string("x_max"), std::vector<std::string>{"None", "The upper x-coordinate of the element"}},
      {std::string("x_max_shift_ndc"),
       std::vector<std::string>{"0", "The upper x border shift for movable transformation in NDC space"}},
      {std::string("x_max_shift_wc"),
       std::vector<std::string>{"0", "The upper x border shift for movable transformation in WC space"}},
      {std::string("x_min"), std::vector<std::string>{"None", "The lower x-coordinate of the element"}},
      {std::string("x_min_shift_ndc"),
       std::vector<std::string>{"0", "The lower x border shift for movable transformation in NDC space"}},
      {std::string("x_min_shift_wc"),
       std::vector<std::string>{"0", "The lower x border shift for movable transformation in WC space"}},
      {std::string("x_origin"),
       std::vector<std::string>{"0", "The world-coordinates of the origin (point of intersection) of the x axis"}},
      {std::string("x_origin_pos"),
       std::vector<std::string>{"low",
                                "The world-coordinates position of the origin (point of intersection) of the x axis"}},
      {std::string("x_range_max"), std::vector<std::string>{"None", "The upper x-value"}},
      {std::string("x_range_min"), std::vector<std::string>{"None", "The lower x-value"}},
      {std::string("x_shift_ndc"),
       std::vector<std::string>{"0", "The x direction shift for movable transformation in NDC space"}},
      {std::string("x_shift_wc"),
       std::vector<std::string>{"0", "The x direction shift for movable transformation in WC space"}},
      {std::string("x_tick"), std::vector<std::string>{"1", "The interval between minor tick marks on the x axis"}},
      {std::string("x1"), std::vector<std::string>{"None", "The lower x-coordinate"}},
      {std::string("x2"), std::vector<std::string>{"None", "The upper x-coordinate"}},
      {std::string("y"), std::vector<std::string>{"None", "References the y-values stored in the context"}},
      {std::string("y_bins"), std::vector<std::string>{"1200", "Bins in y direction"}},
      {std::string("y_dim"), std::vector<std::string>{"None", "The dimension of the y-values"}},
      {std::string("y_flip"), std::vector<std::string>{"0", "Determines whether the y axis should be flipped"}},
      {std::string("y_grid"), std::vector<std::string>{"1", "When set a y grid is created"}},
      {std::string("y_ind"),
       std::vector<std::string>{"-1", "An index which is used to highlight a specific y position"}},
      {std::string("y_label"), std::vector<std::string>{"None", "The label of the y axis"}},
      {std::string("y_label_3d"), std::vector<std::string>{"None", "The label of the 3D y axis"}},
      {std::string("y_labels"), std::vector<std::string>{"None", "References the y labels stored in the context"}},
      {std::string("y_lim_max"), std::vector<std::string>{"None", "The upper y-limit"}},
      {std::string("y_lim_min"), std::vector<std::string>{"None", "The lower y-limit"}},
      {std::string("y_line"), std::vector<std::string>{"None", "Determines whether there is a y line"}},
      {std::string("y_log"), std::vector<std::string>{"0", "Determines whether the y-values are logarithmic"}},
      {std::string("y_major"),
       std::vector<std::string>{"5", "Unitless integer-values specifying the number of minor tick intervals "
                                     "between major tick marks. Values of 0 or 1 imply no minor ticks. Negative "
                                     "values specify no labels will be drawn for the y axis"}},
      {std::string("y_max"), std::vector<std::string>{"None", "The upper y-coordinate of the element"}},
      {std::string("y_max_shift_ndc"),
       std::vector<std::string>{"0", "The upper y border shift for movable transformation in NDC space"}},
      {std::string("y_max_shift_wc"),
       std::vector<std::string>{"0", "The upper y border shift for movable transformation in WC space"}},
      {std::string("y_min"), std::vector<std::string>{"None", "The lower y-coordinate of the element"}},
      {std::string("y_min_shift_ndc"),
       std::vector<std::string>{"0", "The lower y border shift for movable transformation in NDC space"}},
      {std::string("y_min_shift_wc"),
       std::vector<std::string>{"0", "The lower y border shift for movable transformation in WC space"}},
      {std::string("y_origin"),
       std::vector<std::string>{"0", "The world-coordinates of the origin (point of intersection) of the y axis"}},
      {std::string("y_origin_pos"),
       std::vector<std::string>{"low",
                                "The world-coordinates position of the origin (point of intersection) of the y axis"}},
      {std::string("y_range_max"), std::vector<std::string>{"None", "The upper y-value"}},
      {std::string("y_range_min"), std::vector<std::string>{"None", "The lower y-value"}},
      {std::string("y_shift_ndc"),
       std::vector<std::string>{"0", "The y direction shift for movable transformation in NDC space"}},
      {std::string("y_shift_wc"),
       std::vector<std::string>{"0", "The y direction shift for movable transformation in WC space"}},
      {std::string("y_tick"), std::vector<std::string>{"1", "The interval between minor tick marks on the y axis"}},
      {std::string("y1"), std::vector<std::string>{"None", "The lower y-coordinate"}},
      {std::string("y2"), std::vector<std::string>{"None", "The upper y-coordinate"}},
      {std::string("z"), std::vector<std::string>{"None", "References the z-values stored in the context"}},
      {std::string("z_dims"), std::vector<std::string>{"None", "References the z dimensions stored in the context"}},
      {std::string("z_flip"), std::vector<std::string>{"0", "Determines whether the z axis should be flipped"}},
      {std::string("z_grid"), std::vector<std::string>{"1", "When set a z grid is created"}},
      {std::string("z_label_3d"), std::vector<std::string>{"None", "The label of the 3D z axis"}},
      {std::string("z_lim_max"), std::vector<std::string>{"NAN", "The upper z-limit"}},
      {std::string("z_lim_min"), std::vector<std::string>{"NAN", "The lower z-limit"}},
      {std::string("z_log"), std::vector<std::string>{"0", "Determines whether the z-values are logarithmic"}},
      {std::string("z_index"), std::vector<std::string>{"0", "Sets the render order compared to the other elements"}},
      {std::string("z_major"),
       std::vector<std::string>{"5", "Unitless integer-values specifying the number of minor tick intervals "
                                     "between major tick marks. Values of 0 or 1 imply no minor ticks. Negative "
                                     "values specify no labels will be drawn for the z axis"}},
      {std::string("z_max"),
       std::vector<std::string>{
           "None",
           "The maximum z-coordinate of a contour(f) plot (after transforming the input data to a rectangular grid)"}},
      {std::string("z_min"),
       std::vector<std::string>{
           "None",
           "The minimum z-coordinate of a contour(f) plot (after transforming the input data to a rectangular grid)"}},
      {std::string("z_origin"),
       std::vector<std::string>{"0", "The world-coordinates of the origin (point of intersection) of the z axis"}},
      {std::string("z_origin_pos"),
       std::vector<std::string>{"low",
                                "The world-coordinates position of the origin (point of intersection) of the z axis"}},
      {std::string("z_range_max"), std::vector<std::string>{"None", "The upper z-value"}},
      {std::string("z_range_min"), std::vector<std::string>{"None", "The lower z-value"}},
      {std::string("z_tick"), std::vector<std::string>{"1", "The interval between minor tick marks on the z axis"}},
  };
  if (attribute_to_tooltip.count(attribute_name))
    {
      if (element->localName() == "text")
        {
          if (attribute_name == "width") return std::vector<std::string>{"None", "The width of the text"};
          if (attribute_name == "x") return std::vector<std::string>{"None", "x position of the text"};
          if (attribute_name == "y") return std::vector<std::string>{"None", "y position of the text"};
        }
      return attribute_to_tooltip[attribute_name];
    }
  return std::vector<std::string>{"", "No description found"};
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ setter functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void GRM::Render::setClipRegion(const std::shared_ptr<GRM::Element> &element, int region)
{
  /*!
   * This function can be used to set the clip region of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] region The desired clip region
   */
  element->setAttribute("clip_region", region);
}

void GRM::Render::setViewport(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax, double ymin,
                              double ymax)
{
  /*!
   * This function can be used to set the viewport of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the viewport (0 <= xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the viewport (xmin < xmax <= 1)
   * \param[in] ymin TThe bottom vertical coordinate of the viewport (0 <= ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the viewport (ymin < ymax <= 1)
   */
  element->setAttribute("viewport_x_min", xmin);
  element->setAttribute("viewport_x_max", xmax);
  element->setAttribute("viewport_y_min", ymin);
  element->setAttribute("viewport_y_max", ymax);
}

void GRM::Render::setWSViewport(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax, double ymin,
                                double ymax)
{
  /*!
   * This function can be used to set the ws_viewport of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the viewport (0 <= xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the viewport (xmin < xmax <= 1)
   * \param[in] ymin TThe bottom vertical coordinate of the viewport (0 <= ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the viewport (ymin < ymax <= 1)
   */
  element->setAttribute("ws_viewport_x_min", xmin);
  element->setAttribute("ws_viewport_x_max", xmax);
  element->setAttribute("ws_viewport_y_min", ymin);
  element->setAttribute("ws_viewport_y_max", ymax);
}

void GRM::Render::setWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin, double ymax)
{
  /*!
   * This function can be used to set the window of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the window (xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the window (xmin < xmax)
   * \param[in] ymin The bottom vertical coordinate of the window (ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the window (ymin < ymax)
   */
  element->setAttribute("window_x_min", xmin);
  element->setAttribute("window_x_max", xmax);
  element->setAttribute("window_y_min", ymin);
  element->setAttribute("window_y_max", ymax);
}

void GRM::Render::setWSWindow(const std::shared_ptr<Element> &element, double xmin, double xmax, double ymin,
                              double ymax)
{
  /*!
   * This function can be used to set the ws_window of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the window (xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the window (xmin < xmax)
   * \param[in] ymin The bottom vertical coordinate of the window (ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the window (ymin < ymax)
   */
  element->setAttribute("ws_window_x_min", xmin);
  element->setAttribute("ws_window_x_max", xmax);
  element->setAttribute("ws_window_y_min", ymin);
  element->setAttribute("ws_window_y_max", ymax);
}

void GRM::Render::setMarkerType(const std::shared_ptr<Element> &element, int type)
{
  /*!
   * This function can be used to set a MarkerType of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type An Integer setting the MarkerType
   */
  element->setAttribute("marker_type", type);
}

void GRM::Render::setMarkerType(const std::shared_ptr<Element> &element, const std::string &types_key,
                                std::optional<std::vector<int>> types, const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to set a vector of MarkerTypes of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] types_key A string used as a key for storing the types
   * \param[in] types A vector containing the MarkerTypes
   * \param[in] ext_context A GRM::Context used for storing types. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (types != std::nullopt) (*use_context)[types_key] = *types;
  element->setAttribute("marker_types", types_key);
}

void GRM::Render::setMarkerSize(const std::shared_ptr<Element> &element, double size)
{
  /*!
   * This function can be used to set a MarkerSize of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type A Double setting the MarkerSize
   */
  element->setAttribute("marker_size", size);
}

void GRM::Render::setMarkerSize(const std::shared_ptr<Element> &element, const std::string &sizes_key,
                                std::optional<std::vector<double>> sizes,
                                const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to set a vector of MarkerTypes of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] sizes_key A string used as a key for storing the sizes
   * \param[in] sizes A vector containing the MarkerSizes
   * \param[in] ext_context A GRM::Context used for storing sizes. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (sizes != std::nullopt) (*use_context)[sizes_key] = *sizes;
  element->setAttribute("marker_sizes", sizes_key);
}

void GRM::Render::setMarkerColorInd(const std::shared_ptr<Element> &element, int color)
{
  /*!
   * This function can be used to set a MarkerColorInd of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] color An Integer setting the MarkerColorInd
   */
  element->setAttribute("marker_color_ind", color);
}

void GRM::Render::setMarkerColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                                    std::optional<std::vector<int>> colorinds,
                                    const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to set a vector of MarkerColorInds of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] colorinds_key A string used as a key for storing the colorinds
   * \param[in] colorinds A vector containing the MarkerColorInds
   * \param[in] ext_context A GRM::Context used for storing colorinds. By default it uses GRM::Render's GRM::Context
   * object but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (colorinds != std::nullopt) (*use_context)[colorinds_key] = *colorinds;
  element->setAttribute("marker_color_indices", colorinds_key);
}

void GRM::Render::setLineType(const std::shared_ptr<Element> &element, const std::string &types_key,
                              std::optional<std::vector<int>> types, const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to set a vector of LineTypes of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] types_key A string used as a key for storing the types
   * \param[in] types A vector containing the LineTypes
   * \param[in] ext_context A GRM::Context used for storing types. By default it uses GRM::Render's GRM::Context object
   * but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (types != std::nullopt) (*use_context)[types_key] = *types;
  element->setAttribute("line_types", types_key);
}

void GRM::Render::setLineType(const std::shared_ptr<Element> &element, int type)
{
  /*!
   * This function can be used to set a LineType of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type An Integer setting the LineType
   */
  element->setAttribute("line_type", type);
}

void GRM::Render::setLineWidth(const std::shared_ptr<Element> &element, const std::string &widths_key,
                               std::optional<std::vector<double>> widths,
                               const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to set a vector of LineWidths of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] widths_key A string used as a key for storing the widths
   * \param[in] widths A vector containing the LineWidths
   * \param[in] ext_context A GRM::Context used for storing widths. By default it uses GRM::Render's GRM::Context
   * object but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (widths != std::nullopt) (*use_context)[widths_key] = *widths;
  element->setAttribute("line_widths", widths_key);
}

void GRM::Render::setLineWidth(const std::shared_ptr<Element> &element, double width)
{
  /*!
   * This function can be used to set a LineWidth of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] type A Double setting the LineWidth
   */
  element->setAttribute("line_width", width);
}

void GRM::Render::setLineColorInd(const std::shared_ptr<Element> &element, const std::string &colorinds_key,
                                  std::optional<std::vector<int>> colorinds,
                                  const std::shared_ptr<GRM::Context> &ext_context)
{
  /*!
   * This function can be used to set a vector of LineColorInds of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] colorinds_key A string used as a key for storing the colorinds
   * \param[in] colorinds A vector containing the Colorinds
   * \param[in] ext_context A GRM::Context used for storing colorinds. By default it uses GRM::Render's GRM::Context
   * object but an external GRM::Context can be used
   */
  std::shared_ptr<GRM::Context> use_context = (ext_context == nullptr) ? context : ext_context;
  if (colorinds != std::nullopt) (*use_context)[colorinds_key] = *colorinds;
  element->setAttribute("line_color_indices", colorinds_key);
}

void GRM::Render::setLineColorInd(const std::shared_ptr<Element> &element, int color)
{
  /*!
   * This function can be used to set LineColorInd of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] color An Integer value setting the LineColorInd
   */
  element->setAttribute("line_color_ind", color);
}

void GRM::Render::setCharUp(const std::shared_ptr<Element> &element, double ux, double uy)
{
  /*!
   * This function can be used to set CharUp of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] ux  X coordinate of the text up vector
   * \param[in] uy  y coordinate of the text up vector
   */
  element->setAttribute("char_up_x", ux);
  element->setAttribute("char_up_y", uy);
}

void GRM::Render::setTextAlign(const std::shared_ptr<Element> &element, int horizontal, int vertical)
{
  /*!
   * This function can be used to set TextAlign of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] horizontal  Horizontal text alignment
   * \param[in] vertical Vertical text alignment
   */
  element->setAttribute("text_align_horizontal", horizontal);
  element->setAttribute("text_align_vertical", vertical);
}

void GRM::Render::setTextWidthAndHeight(const std::shared_ptr<Element> &element, double width, double height)
{
  /*!
   * This function can be used to set the width and height of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] width Width of the Element
   * \param[in] height Height of the Element
   */
  element->setAttribute("width", width);
  element->setAttribute("height", height);
}

void GRM::Render::setLineSpec(const std::shared_ptr<Element> &element, const std::string &spec)
{
  /*!
   * This function can be used to set the linespec of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] spec An std::string
   */
  element->setAttribute("line_spec", spec);
}

void GRM::Render::setColorRep(const std::shared_ptr<Element> &element, int index, double red, double green, double blue)
{
  /*!
   * This function can be used to set the colorrep of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index Color index in the range 0 to 1256
   * \param[in] red Red intensity in the range 0.0 to 1.0
   * \param[in] green Green intensity in the range 0.0 to 1.0
   * \param[in] blue Blue intensity in the range 0.0 to 1.0
   */
  std::stringstream stream;
  std::string hex;
  int precision = 255;
  auto red_int = static_cast<int>(red * precision + 0.5), green_int = static_cast<int>(green * precision + 0.5),
       blue_int = static_cast<int>(blue * precision + 0.5);

  stream << std::hex << (red_int << 16 | green_int << 8 | blue_int); // Convert RGB to hex
  auto name = "colorrep." + std::to_string(index);
  element->setAttribute(name, stream.str());
}

void GRM::Render::setFillIntStyle(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the fillintstyle of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index The style of fill to be used
   */
  element->setAttribute("fill_int_style", index);
}

void GRM::Render::setFillColorInd(const std::shared_ptr<GRM::Element> &element, int color)
{
  /*!
   * This function can be used to set the fillcolorind of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] color The fill area color index (COLOR < 1256)
   */
  element->setAttribute("fill_color_ind", color);
}

void GRM::Render::setFillStyle(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the fillintstyle of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index The fill style index to be used
   */
  element->setAttribute("fill_style", index);
}

void GRM::Render::setScale(const std::shared_ptr<GRM::Element> &element, int scale)
{
  /*!
   * This function can be used to set the scale of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index The scale index to be used
   */
  element->setAttribute("scale", scale);
}

void GRM::Render::setWindow3d(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax, double ymin,
                              double ymax, double zmin, double zmax)
{
  /*!
   * This function can be used to set the window3d of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] xmin The left horizontal coordinate of the window (xmin < xmax)
   * \param[in] xmax The right horizontal coordinate of the window (xmin < xmax)
   * \param[in] ymin The bottom vertical coordinate of the window (ymin < ymax)
   * \param[in] ymax The top vertical coordinate of the window (ymin < ymax)
   * \param[in] zmin min z-value
   * \param[in] zmax max z-value
   */
  element->setAttribute("window_x_min", xmin);
  element->setAttribute("window_x_max", xmax);
  element->setAttribute("window_y_min", ymin);
  element->setAttribute("window_y_max", ymax);
  element->setAttribute("window_z_min", zmin);
  element->setAttribute("window_z_max", zmax);
}

void GRM::Render::setSpace3d(const std::shared_ptr<GRM::Element> &element, double fov, double camera_distance)
{
  /*! This function can be used to set the space3d of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] phi: azimuthal angle of the spherical coordinates
   * \param[in] theta: polar angle of the spherical coordinates
   * \param[in] fov: vertical field of view(0 or NaN for orthographic projection)
   * \param[in] camera_distance: distance between the camera and the focus point (in arbitrary units, 0 or NaN for the
   * radius of the object's smallest bounding sphere)
   */
  element->setAttribute("space_3d_fov", fov);
  element->setAttribute("space_3d_camera_distance", camera_distance);
}

void GRM::Render::setSpace(const std::shared_ptr<Element> &element, double zmin, double zmax, int rotation, int tilt)
{
  /*!
   * This function can be used to set the space of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] zmin
   * \param[in] zmax
   * \param[in] rotation
   * \param[in] tilt
   */
  element->setAttribute("space_z_min", zmin);
  element->setAttribute("space_z_max", zmax);
  element->setAttribute("space_rotation", rotation);
  element->setAttribute("space_tilt", tilt);
}

void GRM::Render::setSelectSpecificXform(const std::shared_ptr<Element> &element, int transform)
{
  /*! This function can be used to set the window 3d of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] transform Select a predefined transformation from world coordinates to normalized device coordinates.
   */
  element->setAttribute("select_specific_xform", transform);
}

void GRM::Render::setTextColorInd(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the textcolorind of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index The color index
   */
  element->setAttribute("text_color_ind", index);
}

void GRM::Render::setBorderColorInd(const std::shared_ptr<GRM::Element> &element, int index)
{
  /*!
   * This function can be used to set the bordercolorind of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] index The color index
   */
  element->setAttribute("border_color_ind", index);
}

void GRM::Render::setBorderWidth(const std::shared_ptr<GRM::Element> &element, double width)
{
  /*!
   * This function can be used to set the bordercolorind of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] width The border width
   */
  element->setAttribute("border_width", width);
}

void GRM::Render::setCharHeight(const std::shared_ptr<GRM::Element> &element, double height)
{
  /*!
   * This function can be used to set the char height of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] height the char height
   */
  element->setAttribute("char_height", height);
}

void GRM::Render::setTransparency(const std::shared_ptr<GRM::Element> &element, double transparency)
{
  /*!
   * This function can be used to set the transparency of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] transparency The transparency
   */
  element->setAttribute("transparency", transparency);
}

void GRM::Render::setResampleMethod(const std::shared_ptr<GRM::Element> &element, int resample)
{
  /*!
   * This function can be used to set the resamplemethod of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] resample The resample method
   */
  element->setAttribute("resample_method", resample);
}

void GRM::Render::setTextEncoding(const std::shared_ptr<Element> &element, int encoding)
{
  /*!
   * This function can be used to set the textencoding of a GRM::Element
   *
   * \param[in] element A GRM::Element
   * \param[in] encoding The textencoding
   */
  element->setAttribute("text_encoding", encoding);
}

void GRM::Render::setViewportNormalized(const std::shared_ptr<GRM::Element> &element, double xmin, double xmax,
                                        double ymin, double ymax)
{
  element->setAttribute("_viewport_normalized_x_min_org", xmin);
  element->setAttribute("_viewport_normalized_x_max_org", xmax);
  element->setAttribute("_viewport_normalized_y_min_org", ymin);
  element->setAttribute("_viewport_normalized_y_max_org", ymax);
}

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element, const std::string &color_indices_key,
                               const std::vector<int> &color_indices, const std::shared_ptr<GRM::Context> &ext_context)
{
  auto use_context = (ext_context == nullptr) ? context : ext_context;
  element->setAttribute("set_next_color", true);
  if (!color_indices.empty())
    {
      (*use_context)[color_indices_key] = color_indices;
      element->setAttribute("color_ind_values", color_indices_key);
    }
  else
    {
      throw NotFoundError("Color indices are missing in vector\n");
    }
}

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element, const std::string &color_rgb_values_key,
                               const std::vector<double> &color_rgb_values,
                               const std::shared_ptr<GRM::Context> &ext_context)
{
  auto use_context = (ext_context == nullptr) ? context : ext_context;
  element->setAttribute("set_next_color", true);
  if (!color_rgb_values.empty())
    {
      (*use_context)[color_rgb_values_key] = color_rgb_values;
      element->setAttribute("color_rgb_values", color_rgb_values_key);
    }
}

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element,
                               std::optional<std::string> color_indices_key,
                               std::optional<std::string> color_rgb_values_key)
{
  if (color_indices_key != std::nullopt)
    {
      element->setAttribute("color_ind_values", (*color_indices_key));
      element->setAttribute("set_next_color", true);
    }
  else if (color_rgb_values_key != std::nullopt)
    {
      element->setAttribute("set_next_color", true);
      element->setAttribute("color_rgb_values", (*color_rgb_values_key));
    }
}

void GRM::Render::setNextColor(const std::shared_ptr<GRM::Element> &element)
{
  element->setAttribute("set_next_color", true);
  element->setAttribute("snc_fallback", true);
}

void GRM::Render::setOriginPosition(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                                    const std::string &y_org_pos)
{
  element->setAttribute("x_origin_pos", x_org_pos);
  element->setAttribute("y_origin_pos", y_org_pos);
}

void GRM::Render::setOriginPosition3d(const std::shared_ptr<GRM::Element> &element, const std::string &x_org_pos,
                                      const std::string &y_org_pos, const std::string &z_org_pos)
{
  setOriginPosition(element, x_org_pos, y_org_pos);
  element->setAttribute("z_origin_pos", z_org_pos);
}

void GRM::Render::setGR3LightParameters(const std::shared_ptr<GRM::Element> &element, double ambient, double diffuse,
                                        double specular, double specular_power)
{
  element->setAttribute("ambient", ambient);
  element->setAttribute("diffuse", diffuse);
  element->setAttribute("specular", specular);
  element->setAttribute("specular_power", specular_power);
}

void GRM::Render::setAutoUpdate(bool update)
{
  automatic_update = update;
}

void GRM::Render::setEnableEditor(bool update)
{
  enable_editor = update;
}

void GRM::Render::setActiveFigure(const std::shared_ptr<GRM::Element> &element)
{
  auto result = this->firstChildElement()->querySelectorsAll("[active=1]");
  for (auto &child : result)
    {
      child->setAttribute("active", 0);
    }
  element->setAttribute("active", 1);
}

void GRM::Render::setFirstCall(bool call)
{
  first_call = call;
}

void GRM::Render::setPreviousScatterMarkerType(int *previous_type)
{
  previous_scatter_marker_type = previous_type;
}

void GRM::Render::setPreviousLineMarkerType(int *previous_type)
{
  previous_line_marker_type = previous_type;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ getter functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::shared_ptr<GRM::Element> GRM::Render::getActiveFigure()
{
  return active_figure;
}

void GRM::Render::getAutoUpdate(bool *update)
{
  *update = automatic_update;
}

void GRM::Render::getEnableEditor(bool *update)
{
  *update = enable_editor;
}

bool GRM::Render::getFirstCall()
{
  return first_call;
}

bool GRM::Render::getZQueueIsBeingRendered()
{
  return z_queue_is_being_rendered;
}

ManageZIndex *GRM::Render::getZIndexManager()
{
  return &z_index_manager;
}

bool GRM::Render::getRedrawWs()
{
  return redraw_ws;
}

bool GRM::Render::getHighlightedAttrExist()
{
  return highlighted_attr_exist;
}

int *GRM::Render::getPreviousScatterMarkerType()
{
  return previous_scatter_marker_type;
}

int *GRM::Render::getPreviousLineMarkerType()
{
  return previous_line_marker_type;
}

bool GRM::Render::getViewport(const std::shared_ptr<GRM::Element> &element, double *xmin, double *xmax, double *ymin,
                              double *ymax)
{
  if (element->hasAttribute("viewport_x_min") && element->hasAttribute("viewport_x_max") &&
      element->hasAttribute("viewport_y_min") && element->hasAttribute("viewport_x_max"))
    {
      *xmin = static_cast<double>(element->getAttribute("viewport_x_min"));
      *xmax = static_cast<double>(element->getAttribute("viewport_x_max"));
      *ymin = static_cast<double>(element->getAttribute("viewport_y_min"));
      *ymax = static_cast<double>(element->getAttribute("viewport_y_max"));

      if (element->localName() == "central_region")
        {
          auto plot_parent = element;
          getPlotParent(plot_parent);

          if (auto kind = static_cast<std::string>(plot_parent->getAttribute("_kind"));
              kind != "imshow" && kinds_3d.count(kind) == 0)
            {
              auto left_border = static_cast<double>(element->getAttribute("_left_axis_border"));
              auto right_border = static_cast<double>(element->getAttribute("_right_axis_border"));
              auto bottom_border = static_cast<double>(element->getAttribute("_bottom_axis_border"));
              auto top_border = static_cast<double>(element->getAttribute("_top_axis_border"));

              if (kind == "pie" || polar_kinds.count(kind) > 0)
                {
                  *xmin = static_cast<double>(element->getAttribute("_before_centering_polar_vp_x_min"));
                  *xmax = static_cast<double>(element->getAttribute("_before_centering_polar_vp_x_max"));
                  *ymin = static_cast<double>(element->getAttribute("_before_centering_polar_vp_y_min"));
                  *ymax = static_cast<double>(element->getAttribute("_before_centering_polar_vp_y_max"));
                }

              *xmin += left_border;
              *xmax -= right_border;
              *ymin += bottom_border;
              *ymax -= top_border;

              if (kind == "pie" || polar_kinds.count(kind) > 0)
                {
                  bool top_text_margin = false;

                  if (auto top_side_region = plot_parent->querySelectors("side_region[location=\"top\"]");
                      top_side_region && top_side_region->hasAttribute("text_content"))
                    top_text_margin = true;

                  auto x_center = 0.5 * (*xmin + *xmax);
                  auto y_center = 0.5 * (*ymin + *ymax);
                  auto r = 0.45 * grm_min(*xmax - *xmin, *ymax - *ymin);
                  if (top_text_margin)
                    {
                      r *= 0.975;
                      y_center -= 0.025 * r;
                    }

                  *xmin = x_center - r;
                  *xmax = x_center + r;
                  *ymin = y_center - r;
                  *ymax = y_center + r;
                }
            }
        }
      else if (strEqualsAny(element->localName(), "side_plot_region", "colorbar") ||
               (element->localName() == "side_region" && element->querySelectors("side_plot_region")))
        {
          double plot_vp[4];
          std::string location;
          auto plot_parent = element;
          auto ref_vp_element = element->parentElement();
          getPlotParent(plot_parent);

          if (element->hasAttribute("location"))
            location = static_cast<std::string>(element->getAttribute("location"));
          else if (ref_vp_element->hasAttribute("location"))
            location = static_cast<std::string>(ref_vp_element->getAttribute("location"));
          else if (ref_vp_element->parentElement()->hasAttribute("location"))
            location = static_cast<std::string>(ref_vp_element->parentElement()->getAttribute("location"));
          else if (ref_vp_element->parentElement()->parentElement()->hasAttribute("location"))
            location =
                static_cast<std::string>(ref_vp_element->parentElement()->parentElement()->getAttribute("location"));

          if (!GRM::Render::getViewport(plot_parent, &plot_vp[0], &plot_vp[1], &plot_vp[2], &plot_vp[3]))
            throw NotFoundError(plot_parent->localName() + " doesn't have a viewport but it should.\n");

          if (strEqualsAny(location, "left", "right"))
            {
              *ymin += 0.025 * (plot_vp[3] - plot_vp[2]);
              *ymax -= 0.025 * (plot_vp[3] - plot_vp[2]);

              if (element->hasAttribute("_viewport_offset") ||
                  (element->localName() == "colorbar" && element->parentElement()->hasAttribute("_viewport_offset")))
                {
                  auto offset = element->localName() == "colorbar"
                                    ? static_cast<double>(element->parentElement()->getAttribute("_viewport_offset"))
                                    : static_cast<double>(element->getAttribute("_viewport_offset"));
                  if (location == "right")
                    *xmax -= offset;
                  else if (location == "left")
                    *xmin += offset;
                }
            }
          else if (strEqualsAny(location, "bottom", "top"))
            {
              *xmin += 0.025 * (plot_vp[1] - plot_vp[0]);
              *xmax -= 0.025 * (plot_vp[1] - plot_vp[0]);

              if (element->hasAttribute("_viewport_offset") ||
                  (element->localName() == "colorbar" && element->parentElement()->hasAttribute("_viewport_offset")))
                {
                  auto offset = element->localName() == "colorbar"
                                    ? static_cast<double>(element->parentElement()->getAttribute("_viewport_offset"))
                                    : static_cast<double>(element->getAttribute("_viewport_offset"));
                  if (location == "top")
                    *ymax -= offset;
                  else if (location == "bottom")
                    *ymin += offset;
                }
            }
        }
      return true;
    }
  return false;
}

std::shared_ptr<GRM::Context> GRM::Render::getRenderContext()
{
  return this->context;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ filter functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void GRM::renderCaller()
{
  if (global_root && static_cast<int>(global_root->getAttribute("_modified")) && automatic_update)
    global_render->processTree();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ modify context ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void GRM::updateContextAttribute(const std::shared_ptr<GRM::Element> &element, const std::string &attr,
                                 const GRM::Value &old_value)
{
  // when the attribute is a context the counter for the attribute value inside the context must be modified
  if (valid_context_keys.find(attr) != valid_context_keys.end())
    {
      if (auto new_value = element->getAttribute(attr); new_value.isString())
        {
          auto context = global_render->getContext();
          (*context)[attr].useContextKey(static_cast<std::string>(new_value), static_cast<std::string>(old_value));
        }
    }
}

void GRM::deleteContextAttribute(const std::shared_ptr<GRM::Element> &element)
{
  auto elem_attribs = element->getAttributeNames();
  std::vector<std::string> attr_inter;
  std::vector<std::string> elem_attribs_vec(elem_attribs.begin(), elem_attribs.end());
  std::vector<std::string> valid_keys_copy(valid_context_keys.begin(), valid_context_keys.end());

  std::sort(elem_attribs_vec.begin(), elem_attribs_vec.end());
  std::sort(valid_keys_copy.begin(), valid_keys_copy.end());
  std::set_intersection(elem_attribs_vec.begin(), elem_attribs_vec.end(), valid_keys_copy.begin(),
                        valid_keys_copy.end(), std::back_inserter(attr_inter));

  auto context = global_render->getContext();
  for (const auto &attr : attr_inter)
    {
      if (auto value = element->getAttribute(attr); value.isString())
        (*context)[attr].decrementKey(static_cast<std::string>(value));
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ cleanup element ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void GRM::cleanupElement(GRM::Element &element)
{
  if (element.hasAttribute("_bbox_id"))
    {
      auto bbox_id = std::abs(static_cast<int>(element.getAttribute("_bbox_id")));
      idPool().release(bbox_id);
      boundingMap().erase(bbox_id);
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ history ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const char *GRM::Render::initializeHistory()
{
  if (grm_tmp_dir == nullptr) grm_tmp_dir = createTmpDir();
  if (grm_tmp_dir == nullptr) throw(std::runtime_error("Tmp directory could not be created"));

  return grm_tmp_dir;
}
