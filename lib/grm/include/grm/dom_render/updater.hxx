#ifndef GR_UPDATER_HXX
#define GR_UPDATER_HXX

#include <grm/dom_render/graphics_tree/element.hxx>
#include <grm/dom_render/graphics_tree/document.hxx>


namespace GRM
{
GRM_EXPORT void updateFilter(const std::shared_ptr<Element> &element, const std::string &attr,
                             const std::string &value);
}

#endif // GR_UPDATER_HXX
