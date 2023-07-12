#ifndef GR_DRAWABLE_HXX
#define GR_DRAWABLE_HXX

#include <functional>
#include "grm/dom_render/graphics_tree/Element.hxx"
#include "grm/dom_render/context.hxx"

class Drawable
{
public:
  Drawable(
      const std::shared_ptr<GRM::Element> element, const std::shared_ptr<GRM::Context> context, int grContextId,
      int zIndex,
      std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)> drawFunction);
  void draw();
  int zIndex;
  int insertionIndex; /* used to order drawables with the same zIndex in the order of insertion */
  int getGrContextId() const;
  const std::shared_ptr<GRM::Element> &getElement() const;

private:
  int grContextId;
  const std::shared_ptr<GRM::Element> element;
  const std::shared_ptr<GRM::Context> context;
  std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)> drawFunction;
};

class CompareZIndex
{
public:
  bool operator()(std::shared_ptr<Drawable> const &lhs, std::shared_ptr<Drawable> const &rhs);
};

#endif // GR_DRAWABLE_HXX
