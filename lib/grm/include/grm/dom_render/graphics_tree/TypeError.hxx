#ifndef GRM_GRAPHICS_TREE_INTERFACE_TYPEERROR_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_TYPEERROR_HXX

#include <stdexcept>
#include <grm/util.h>

namespace GRM
{
class EXPORT TypeError : public std::logic_error
{
public:
  explicit TypeError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
} // namespace GRM

#endif
