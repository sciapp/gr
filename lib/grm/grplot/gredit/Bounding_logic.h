#ifndef QT_EXAMPLE_BOUNDING_LOGIC_H
#define QT_EXAMPLE_BOUNDING_LOGIC_H
#include "Bounding_object.h"
#include <grm/dom_render/graphics_tree/util.hxx>


class Bounding_logic
{
public:
  Bounding_logic();
  ~Bounding_logic() = default;
  void add_bounding_object(int id, double xmin, double xmax, double ymin, double ymax,
                           std::shared_ptr<GR::Element> ref);
  void add_bounding_object(const Bounding_object &obj);
  std::vector<Bounding_object> get_bounding_objects_at_point(int x, int y);
  void clear();

private:
  std::vector<Bounding_object> bounding_objects;
};


#endif // QT_EXAMPLE_BOUNDING_LOGIC_H
