
#ifndef GR_GRAPHICS_TREE_INTERFACE_NOTSUPPORTEDERROR_HXX
#define GR_GRAPHICS_TREE_INTERFACE_NOTSUPPORTEDERROR_HXX

#include <stdexcept>

namespace GR
{
class NotSupportedError : public std::logic_error
{
public:
  explicit NotSupportedError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
} // namespace GR

#endif