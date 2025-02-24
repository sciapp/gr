#ifndef LAYOUT_ERROR_HXX_INCLUDED
#define LAYOUT_ERROR_HXX_INCLUDED

#include <string>
#include <stdexcept>
#include "grm/error.h"
#include <grm/util.h>

namespace GRM
{

class GRM_EXPORT InvalidArgument : public std::invalid_argument
{
protected:
  grm_error_t errorCode = GRM_ERROR_NONE;

public:
  InvalidArgument(const std::string &msg);
  grm_error_t getErrorNumber() const;
};

class GRM_EXPORT InvalidIndex : public InvalidArgument
{
public:
  InvalidIndex(const std::string &msg);
};
class GRM_EXPORT ContradictingAttributes : public InvalidArgument
{
public:
  ContradictingAttributes(const std::string &msg);
};
class GRM_EXPORT InvalidArgumentRange : public InvalidArgument
{
public:
  InvalidArgumentRange(const std::string &msg);
};

} // namespace GRM

#endif /* ifndef LAYOUT_ERROR_HPP_INCLUDED */
