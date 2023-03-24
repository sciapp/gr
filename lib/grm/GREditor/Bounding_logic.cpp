#include "Bounding_logic.h"

#include <utility>
#include <iostream>
#include <algorithm>
#include <vector>

bool bounding_object_compare_funciton(const Bounding_object &i, const Bounding_object &j)
{
  return i.boundingRect().width() * i.boundingRect().height() < j.boundingRect().width() * j.boundingRect().height();
}

std::vector<Bounding_object> Bounding_logic::get_bounding_objects_at_point(int x, int y)
{
  std::vector<Bounding_object> ret;
  for (auto &bounding_object : bounding_objects)
    {
      if (bounding_object.contains_point(x, y))
        {
          ret.push_back(bounding_object);
        }
    }
  std::sort(ret.begin(), ret.end(), bounding_object_compare_funciton);
  return ret;
}


void Bounding_logic::add_bounding_object(int id, double xmin, double xmax, double ymin, double ymax,
                                         std::shared_ptr<GR::Element> ref)
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
