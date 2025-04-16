#include "grm/layout_error.hxx"

using namespace GRM;

InvalidArgument::InvalidArgument(const std::string &msg) : std::invalid_argument(msg) {}

grm_error_t InvalidArgument::getErrorNumber() const
{
  return this->errorCode;
}

InvalidIndex::InvalidIndex(const std::string &msg) : InvalidArgument(msg)
{
  this->errorCode = GRM_ERROR_LAYOUT_INVALID_INDEX;
}

ContradictingAttributes::ContradictingAttributes(const std::string &msg) : InvalidArgument(msg)
{
  this->errorCode = GRM_ERROR_LAYOUT_CONTRADICTING_ATTRIBUTES;
}

InvalidArgumentRange::InvalidArgumentRange(const std::string &msg) : InvalidArgument(msg)
{
  this->errorCode = GRM_ERROR_LAYOUT_INVALID_ARGUMENT_RANGE;
}
