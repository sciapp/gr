#ifndef LAYOUT_ERROR_HXX_INCLUDED
#define LAYOUT_ERROR_HXX_INCLUDED

#include <string>
#include "grm/error.h"
#include <stdexcept>

// TODO: namespace grm?
// namespace grm
//{

class InvalidArgument : public std::invalid_argument
{
protected:
  err_t errorCode = ERROR_NONE;

public:
  InvalidArgument(const std::string &msg);
  err_t getErrorNumber() const;
};

class InvalidIndex : public InvalidArgument
{
public:
  InvalidIndex(const std::string &msg);
};
class ContradictingAttributes : public InvalidArgument
{
public:
  ContradictingAttributes(const std::string &msg);
};
class InvalidArgumentRange : public InvalidArgument
{
public:
  InvalidArgumentRange(const std::string &msg);
};

//} // namespace grm

#endif /* ifndef LAYOUT_ERROR_HPP_INCLUDED */