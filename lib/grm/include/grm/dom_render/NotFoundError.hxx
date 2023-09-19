#ifndef GRENDER_NOTFOUNDERROR_H
#define GRENDER_NOTFOUNDERROR_H

#include <stdexcept>
#include <grm/util.h>

class NotFoundError : public std::logic_error
{
public:
  EXPORT explicit NotFoundError(const std::string &what_arg) : std::logic_error(what_arg) {}
};

#endif // GRENDER_NOTFOUNDERROR_H
