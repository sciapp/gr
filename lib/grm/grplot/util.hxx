#ifndef UTIL_HXX_INCLUDED
#define UTIL_HXX_INCLUDED

namespace util
{
template <class... T> void unused(T &&...) {}
} // namespace util

static bool endsWith(const std::string &str, const std::string &suffix)
{
  return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

static bool startsWith(const std::string &str, const std::string &prefix)
{
  return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

#endif /* ifndef UTIL_HXX_INCLUDED */
