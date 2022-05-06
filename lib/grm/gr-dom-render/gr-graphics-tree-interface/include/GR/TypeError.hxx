#ifndef GR_GRAPHICS_TREE_INTERFACE_TYPEERROR_HXX
#define GR_GRAPHICS_TREE_INTERFACE_TYPEERROR_HXX

#include <stdexcept>

namespace GR
{
class TypeError : public std::logic_error
{
public:
  explicit TypeError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
} // namespace GR

#endif
