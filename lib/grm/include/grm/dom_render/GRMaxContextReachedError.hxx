#ifndef GR_GRMAXCONTEXTREACHEDERROR_HXX
#define GR_GRMAXCONTEXTREACHEDERROR_HXX

#include <stdexcept>
#include <grm/util.h>

class GRMaxContextReachedError : public std::length_error
{
public:
  EXPORT explicit GRMaxContextReachedError(const std::string &what_arg) : std::length_error(what_arg) {}
};

#endif // GR_GRMAXCONTEXTREACHEDERROR_HXX
