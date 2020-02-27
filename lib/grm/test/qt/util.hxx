#ifndef UTIL_HXX_INCLUDED
#define UTIL_HXX_INCLUDED value

#include <iostream>
#include <iomanip>
#include <QPoint>

namespace util
{

template <typename T> int sgn(T x)
{
  return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}

template <class... T> void unused(T &&...) {}

} // namespace util

inline std::ostream &operator<<(std::ostream &os, const QPoint &point)
{
  return os << "(" << std::setw(4) << point.x() << ", " << std::setw(4) << point.y() << ")";
}

#endif /* ifndef UTIL_HXX_INCLUDED */
