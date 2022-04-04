#ifndef GR_GRAPHICS_TREE_INTERFACE_INDEXSIZEERROR_HXX
#define GR_GRAPHICS_TREE_INTERFACE_INDEXSIZEERROR_HXX

#include <stdexcept>

namespace GR
{
class IndexSizeError : public std::logic_error
{
public:
  explicit IndexSizeError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
} // namespace GR

#endif
