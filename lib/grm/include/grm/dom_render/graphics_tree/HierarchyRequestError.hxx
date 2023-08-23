#ifndef GRM_GRAPHICS_TREE_INTERFACE_HIERARCHYREQUESTERROR_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_HIERARCHYREQUESTERROR_HXX

#include <stdexcept>
#include <grm/util.h>

namespace GRM
{
class EXPORT HierarchyRequestError : public std::logic_error
{
public:
  explicit HierarchyRequestError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
} // namespace GRM

#endif
