#include "Bounding_logic.h"

#include <algorithm>
#include <vector>
#include <cmath>
#include <grm.h>

bool bounding_object_compare_function(const Bounding_object &i, const Bounding_object &j)
{
  int i_index = 1, j_index = 1;
  if (i.get_ref()->hasAttribute("z_index") && static_cast<int>(i.get_ref()->getAttribute("z_index")) < 0)
    i_index = abs(static_cast<int>(i.get_ref()->getAttribute("z_index")));
  if (j.get_ref()->hasAttribute("z_index") && static_cast<int>(j.get_ref()->getAttribute("z_index")) < 0)
    j_index = abs(static_cast<int>(j.get_ref()->getAttribute("z_index")));

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
      return idist * i_index < jdist * j_index;
    }
  return i.boundingRect().width() * i.boundingRect().height() * i_index <
         j.boundingRect().width() * j.boundingRect().height() * j_index;
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
