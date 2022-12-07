#ifndef UTIL_HXX_INCLUDED
#define UTIL_HXX_INCLUDED

#include <string>

namespace util
{
template <class... T> void unused(T &&...) {}
} // namespace util

bool endsWith(const std::string &str, const std::string &suffix);
bool startsWith(const std::string &str, const std::string &prefix);

#endif /* ifndef UTIL_HXX_INCLUDED */
