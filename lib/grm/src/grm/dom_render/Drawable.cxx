#include "grm/dom_render/Drawable.hxx"
#include "gr.h"

Drawable::Drawable(
    const std::shared_ptr<GRM::Element> element, const std::shared_ptr<GRM::Context> context, int grContextId,
    int zIndex,
    std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)> drawFunction)
    : grContextId(grContextId), element(element), context(context), drawFunction(drawFunction), zIndex(zIndex)
{
  gr_savestateincontext(grContextId);
}

void Drawable::draw()
{
  gr_selectcontext(grContextId);
  drawFunction(element, context);
}

int Drawable::getGrContextId() const
{
  return grContextId;
}

bool CompareZIndex::operator()(std::shared_ptr<Drawable> const &lhs, std::shared_ptr<Drawable> const &rhs)
{
  if (lhs->zIndex != rhs->zIndex)
    return lhs->zIndex > rhs->zIndex;
  else
    return lhs->insertionIndex > rhs->insertionIndex;
}
