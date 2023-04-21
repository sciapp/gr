#ifndef GRM_GRAPHICS_TREE_INTERFACE_NOTSUPPORTEDERROR_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_NOTSUPPORTEDERROR_HXX

#include <stdexcept>
#include <grm/util.h>

namespace GRM
{
class EXPORT NotSupportedError : public std::logic_error
{
public:
  explicit NotSupportedError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
} // namespace GRM

#endif
