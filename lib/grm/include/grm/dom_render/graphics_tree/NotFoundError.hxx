#ifndef GRM_GRAPHICS_TREE_INTERFACE_NOTFOUNDERROR_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_NOTFOUNDERROR_HXX

#include <stdexcept>
#include <grm/util.h>

namespace GRM
{
class EXPORT NotFoundError : public std::logic_error
{
public:
  explicit NotFoundError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
} // namespace GRM

#endif
