#ifndef GRENDER_TYPEERROR_HXX
#define GRENDER_TYPEERROR_HXX

#include <stdexcept>
#include <grm/util.h>

class TypeError : public std::logic_error
{
public:
  EXPORT explicit TypeError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
#endif // GRENDER_TYPEERROR_HXX
