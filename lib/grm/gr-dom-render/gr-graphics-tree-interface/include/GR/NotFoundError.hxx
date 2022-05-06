#ifndef GR_GRAPHICS_TREE_INTERFACE_NOTFOUNDERROR_HXX
#define GR_GRAPHICS_TREE_INTERFACE_NOTFOUNDERROR_HXX

#include <stdexcept>

namespace GR
{
class NotFoundError : public std::logic_error
{
public:
  explicit NotFoundError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
} // namespace GR

#endif
