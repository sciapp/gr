#include "Bounding_logic.h"

#include <algorithm>
#include <vector>
#include <cmath>

bool bounding_object_compare_function(const Bounding_object &i, const Bounding_object &j)
{
  if (abs(i.boundingRect().width() * i.boundingRect().height() - j.boundingRect().width() * j.boundingRect().height()) <
      1e-8)
    {
      double camx, camy;
      double ixmin, ixmax, iymin, iymax, idist;
      double jxmin, jxmax, jymin, jymax, jdist;

      i.get_cam(&camx, &camy);
      i.get_corner(&ixmin, &ixmax, &iymin, &iymax);
      j.get_corner(&jxmin, &jxmax, &jymin, &jymax);

      idist = sqrt(pow(((ixmax + ixmin) / 2 - camx), 2) + pow(((iymax + iymin) / 2 - camy), 2));
      jdist = sqrt(pow(((jxmax + jxmin) / 2 - camx), 2) + pow(((jymax + jymin) / 2 - camy), 2));
      return idist < jdist;
    }
  else
    {
      return i.boundingRect().width() * i.boundingRect().height() <
             j.boundingRect().width() * j.boundingRect().height();
    }
}

std::vector<Bounding_object> Bounding_logic::get_bounding_objects_at_point(int x, int y)
{
  std::vector<Bounding_object> ret;
  for (auto &bounding_object : bounding_objects)
    {
      if (bounding_object.contains_point(x, y))
        {
          bounding_object.set_cam(x, y);
          ret.push_back(bounding_object);
        }
    }
  std::sort(ret.begin(), ret.end(), bounding_object_compare_function);
  return ret;
}


void Bounding_logic::add_bounding_object(int id, double xmin, double xmax, double ymin, double ymax,
                                         std::shared_ptr<GRM::Element> ref)
{
  bounding_objects.emplace_back(Bounding_object(id, xmin, xmax, ymin, ymax, std::move(ref)));
}

void Bounding_logic::add_bounding_object(const Bounding_object &obj)
{
  bounding_objects.emplace_back(obj);
}

Bounding_logic::Bounding_logic()
{
  bounding_objects = std::vector<Bounding_object>();
}

void Bounding_logic::clear()
{
  bounding_objects.clear();
}
