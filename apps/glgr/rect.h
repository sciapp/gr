#ifndef RECT_H_INCLUDED
#define RECT_H_INCLUDED

#include <qgl.h>
#include "vec.h"

class Rect
{
private:
  vec p[4];
  bool testit(vec, vec, vec, vec) const;
  vec position;

public:
  Rect();
  Rect(const vec &pos, const vec &xdir, const vec &ydir, float width, float height, GLfloat camMat[4][4]);
  bool intersects(const Rect &) const;
  vec pos() const { return position; }
};
#endif
