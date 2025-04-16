#include "grm/dom_render/Drawable.hxx"
#include "gr.h"
#include "grm/dom_render/render.hxx"

Drawable::Drawable(
    const std::shared_ptr<GRM::Element> element, const std::shared_ptr<GRM::Context> context, int gr_context_id,
    int z_index,
    std::function<void(const std::shared_ptr<GRM::Element> &, const std::shared_ptr<GRM::Context> &)> draw_function)
    : gr_context_id(gr_context_id), element(element), context(context), drawFunction(draw_function), z_index(z_index)
{
  ;
}

void Drawable::draw()
{
  gr_selectcontext(gr_context_id);
  gr_savestate();
  bool old_state;
  GRM::Render::getAutoUpdate(&old_state);
  GRM::Render::setAutoUpdate(false);
  GRM::Render::processAttributes(element);
  drawFunction(element, context);
  GRM::Render::setAutoUpdate(old_state);
  gr_restorestate();
}

const std::shared_ptr<GRM::Element> &Drawable::getElement() const
{
  return element;
}

int Drawable::getGrContextId() const
{
  return gr_context_id;
}

bool CompareZIndex::operator()(std::shared_ptr<Drawable> const &lhs, std::shared_ptr<Drawable> const &rhs)
{
  if (lhs->z_index != rhs->z_index) return lhs->z_index > rhs->z_index;
  return lhs->insertion_index > rhs->insertion_index;
}
