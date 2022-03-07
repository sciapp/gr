//
// Created by Du Kim Nguyen on 11.01.22.
//

#ifndef GRENDER_NOTFOUNDERROR_H
#define GRENDER_NOTFOUNDERROR_H

#include <stdexcept>

class NotFoundError : public std::logic_error
{
public:
  explicit NotFoundError(const std::string &what_arg) : std::logic_error(what_arg) {}
};

#endif // GRENDER_NOTFOUNDERROR_H
