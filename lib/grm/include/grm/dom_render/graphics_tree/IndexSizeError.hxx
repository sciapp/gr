#ifndef GRM_GRAPHICS_TREE_INTERFACE_INDEXSIZEERROR_HXX
#define GRM_GRAPHICS_TREE_INTERFACE_INDEXSIZEERROR_HXX

#include <stdexcept>
#include <grm/util.h>

namespace GRM
{
class EXPORT IndexSizeError : public std::logic_error
{
public:
  explicit IndexSizeError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
} // namespace GRM

#endif
