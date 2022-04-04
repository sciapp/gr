#ifndef GR_GRAPHICS_TREE_INTERFACE_HIERARCHYREQUESTERROR_HXX
#define GR_GRAPHICS_TREE_INTERFACE_HIERARCHYREQUESTERROR_HXX

#include <stdexcept>

namespace GR
{
class HierarchyRequestError : public std::logic_error
{
public:
  explicit HierarchyRequestError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
} // namespace GR

#endif
