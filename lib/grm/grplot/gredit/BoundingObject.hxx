#ifndef QT_EXAMPLE_BOUNDING_OBJECT_H
#define QT_EXAMPLE_BOUNDING_OBJECT_H

// #include "main_window.hxx"
#include <grm/dom_render/graphics_tree/util.hxx>

#include <utility>
#include <QRectF>

class BoundingObject
{
public:
  BoundingObject(int id, double xmin, double xmax, double ymin, double ymax, std::shared_ptr<GRM::Element> ref)
      : id(id), x_min(xmin), x_max(xmax), y_min(ymin), y_max(ymax), ref(std::move(ref)){

                                                                    };

  [[nodiscard]] QRectF boundingRect() const;


  ~BoundingObject();

  [[nodiscard]] bool containsPoint(int x, int y) const;

  [[nodiscard]] std::shared_ptr<GRM::Element> getRef() const;
  [[nodiscard]] int getId() const;
  void getCam(double *x, double *y) const;
  void getCorner(double *x_min, double *x_max, double *y_min, double *y_max) const;

  void setCam(double x, double y);

protected:
  double x_cam, y_cam;

private:
  int id;
  double x_min, x_max, y_min, y_max;
  std::weak_ptr<GRM::Element> ref;
};


#endif // QT_EXAMPLE_BOUNDING_OBJECT_H
