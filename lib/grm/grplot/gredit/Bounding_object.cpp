#include "Bounding_object.h"

// getter for ref
std::shared_ptr<GRM::Element> Bounding_object::get_ref() const
{
  return ref.lock();
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

int Bounding_object::get_id() const
{
  return id;
}

void Bounding_object::get_cam(double *x, double *y) const
{
  *x = xcam;
  *y = ycam;
}

void Bounding_object::get_corner(double *x_min, double *x_max, double *y_min, double *y_max) const
{
  *x_min = xmin;
  *x_max = xmax;
  *y_min = ymin;
  *y_max = ymax;
}

void Bounding_object::set_cam(double x, double y)
{
  xcam = x;
  ycam = y;
}
