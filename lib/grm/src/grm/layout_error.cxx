#include "grm/layout_error.hxx"

// TODO: namespace grm?
// using namespace grm;

InvalidArgument::InvalidArgument(const std::string &msg) : std::invalid_argument(msg) {}

err_t InvalidArgument::getErrorNumber() const
{
  return this->errorCode;
}

InvalidIndex::InvalidIndex(const std::string &msg) : InvalidArgument(msg)
{
  this->errorCode = ERROR_LAYOUT_INVALID_INDEX;
}

ContradictingAttributes::ContradictingAttributes(const std::string &msg) : InvalidArgument(msg)
{
  this->errorCode = ERROR_LAYOUT_CONTRADICTING_ATTRIBUTES;
}

InvalidArgumentRange::InvalidArgumentRange(const std::string &msg) : InvalidArgument(msg)
{
  this->errorCode = ERROR_LAYOUT_INVALID_ARGUMENT_RANGE;
}