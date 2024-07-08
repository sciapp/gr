#ifndef GRENDER_INVALIDVALUEERROR_HXX
#define GRENDER_INVALIDVALUEERROR_HXX

#include <stdexcept>
#include <grm/util.h>

class InvalidValueError : public std::logic_error
{
public:
  EXPORT explicit InvalidValueError(const std::string &what_arg) : std::logic_error(what_arg) {}
};

#endif // GRENDER_INVALIDVALUEERROR_HXX
