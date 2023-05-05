#ifndef GR_PARENTHASTOBEPROCESSEDBEFORECHILDERROR_HXX
#define GR_PARENTHASTOBEPROCESSEDBEFORECHILDERROR_HXX

#include <stdexcept>

class ParentHasToBeProcessedBeforeChildError : public std::logic_error
{
public:
  EXPORT explicit ParentHasToBeProcessedBeforeChildError(const std::string &what_arg) : std::logic_error(what_arg) {}
};

#endif // GR_PARENTHASTOBEPROCESSEDBEFORECHILDERROR_HXX
