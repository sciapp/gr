#include "BoundingObject.hxx"

// getter for ref
std::shared_ptr<GRM::Element> BoundingObject::getRef() const
{
  return ref.lock();
}

bool BoundingObject::containsPoint(int x, int y) const
{
  return x >= x_min && x <= x_max && y >= y_min && y <= y_max;
}

BoundingObject::~BoundingObject() = default;

QRectF BoundingObject::boundingRect() const
{
  return {x_min, y_min, x_max - x_min, y_max - y_min};
}

int BoundingObject::getId() const
{
  return id;
}

void BoundingObject::getCam(double *x, double *y) const
{
  *x = x_cam;
  *y = y_cam;
}

void BoundingObject::getCorner(double *x_min, double *x_max, double *y_min, double *y_max) const
{
  *x_min = this->x_min;
  *x_max = this->x_max;
  *y_min = this->y_min;
  *y_max = this->y_max;
}

void BoundingObject::setCam(double x, double y)
{
  x_cam = x;
  y_cam = y;
}
