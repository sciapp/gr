//
// Created by Du Kim Nguyen on 03.02.22.
//

#ifndef GRENDER_TYPEERROR_HXX
#define GRENDER_TYPEERROR_HXX
#include <stdexcept>

class TypeError : public std::logic_error
{
public:
  explicit TypeError(const std::string &what_arg) : std::logic_error(what_arg) {}
};
#endif // GRENDER_TYPEERROR_HXX
