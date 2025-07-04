#include "BoundingLogic.hxx"

#include <algorithm>
#include <vector>
#include <cmath>
#include <grm.h>

bool boundingObjectCompareFunction(const BoundingObject &i, const BoundingObject &j)
{
  int i_index = 1, j_index = 1;
  if (i.getRef()->hasAttribute("z_index") && static_cast<int>(i.getRef()->getAttribute("z_index")) < 0)
    i_index = abs(static_cast<int>(i.getRef()->getAttribute("z_index")));
  if (j.getRef()->hasAttribute("z_index") && static_cast<int>(j.getRef()->getAttribute("z_index")) < 0)
    j_index = abs(static_cast<int>(j.getRef()->getAttribute("z_index")));

  if (abs(i.boundingRect().width() * i.boundingRect().height() - j.boundingRect().width() * j.boundingRect().height()) <
      1e-8)
    {
      double camx, camy;
      double i_xmin, i_xmax, i_ymin, i_ymax, i_dist;
      double j_xmin, j_xmax, j_ymin, j_ymax, j_dist;

      i.getCam(&camx, &camy);
      i.getCorner(&i_xmin, &i_xmax, &i_ymin, &i_ymax);
      j.getCorner(&j_xmin, &j_xmax, &j_ymin, &j_ymax);

      i_dist = sqrt(pow(((i_xmax + i_xmin) / 2 - camx), 2) + pow(((i_ymax + i_ymin) / 2 - camy), 2));
      j_dist = sqrt(pow(((j_xmax + j_xmin) / 2 - camx), 2) + pow(((j_ymax + j_ymin) / 2 - camy), 2));
      return i_dist * i_index < j_dist * j_index;
    }
  return i.boundingRect().width() * i.boundingRect().height() * i_index <
         j.boundingRect().width() * j.boundingRect().height() * j_index;
}

std::vector<BoundingObject> BoundingLogic::getBoundingObjectsAtPoint(int x, int y, bool grid_hidden)
{
  std::vector<BoundingObject> ret;
  for (auto &bounding_object : bounding_objects)
    {
      if (grid_hidden &&
          (bounding_object.getRef()->localName() == "grid_line" || bounding_object.getRef()->localName() == "tick" ||
           bounding_object.getRef()->localName() == "tick_group"))
        continue;
      if (bounding_object.containsPoint(x, y))
        {
          bounding_object.setCam(x, y);
          ret.push_back(bounding_object);
        }
    }
  std::sort(ret.begin(), ret.end(), boundingObjectCompareFunction);
  return ret;
}


void BoundingLogic::addBoundingObject(int id, double xmin, double xmax, double ymin, double ymax,
                                      std::shared_ptr<GRM::Element> ref)
{
  bounding_objects.emplace_back(BoundingObject(id, xmin, xmax, ymin, ymax, std::move(ref)));
}

void BoundingLogic::addBoundingObject(const BoundingObject &obj)
{
  bounding_objects.emplace_back(obj);
}

BoundingLogic::BoundingLogic()
{
  bounding_objects = std::vector<BoundingObject>();
}

void BoundingLogic::clear()
{
  bounding_objects.clear();
}
