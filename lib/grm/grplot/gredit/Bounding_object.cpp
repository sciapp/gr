#include "Bounding_object.h"

// getter for ref
std::shared_ptr<GRM::Element> Bounding_object::get_ref() const
{
  return ref;
}

bool Bounding_object::contains_point(int x, int y) const
{
  return x >= xmin && x <= xmax && y >= ymin && y <= ymax;
}

Bounding_object::~Bounding_object() = default;

QRectF Bounding_object::boundingRect() const
{
  return {xmin, ymin, xmax - xmin, ymax - ymin};
}
