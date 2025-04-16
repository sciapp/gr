#ifndef QT_EXAMPLE_BOUNDING_LOGIC_H
#define QT_EXAMPLE_BOUNDING_LOGIC_H
#include "BoundingObject.hxx"
#include <grm/dom_render/graphics_tree/util.hxx>


class BoundingLogic
{
public:
  BoundingLogic();
  ~BoundingLogic() = default;
  void addBoundingObject(int id, double xmin, double xmax, double ymin, double ymax, std::shared_ptr<GRM::Element> ref);
  void addBoundingObject(const BoundingObject &obj);
  std::vector<BoundingObject> getBoundingObjectsAtPoint(int x, int y);
  void clear();

private:
  std::vector<BoundingObject> bounding_objects;
};


#endif // QT_EXAMPLE_BOUNDING_LOGIC_H
