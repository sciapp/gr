#ifndef QT_EXAMPLE_BOUNDING_OBJECT_H
#define QT_EXAMPLE_BOUNDING_OBJECT_H

// #include "main_window.hxx"
#include <grm/dom_render/graphics_tree/util.hxx>

#include <utility>
#include <QRectF>

class Bounding_object
{
public:
  Bounding_object(int id, double xmin, double xmax, double ymin, double ymax, std::shared_ptr<GRM::Element> ref)
      : id(id), xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax), ref(std::move(ref)){

                                                                };

  [[nodiscard]] QRectF boundingRect() const;


  ~Bounding_object();

  [[nodiscard]] bool contains_point(int x, int y) const;

  [[nodiscard]] std::shared_ptr<GRM::Element> get_ref() const;

  [[nodiscard]] int get_id() const;


private:
  int id;
  double xmin, xmax, ymin, ymax;
  std::shared_ptr<GRM::Element> ref;
};


#endif // QT_EXAMPLE_BOUNDING_OBJECT_H
