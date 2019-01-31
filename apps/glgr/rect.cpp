#include "rect.h"

using namespace std;

Rect::Rect()
{
  p[0] = p[1] = p[2] = p[3] = vec(-2000, -2000, 0);
}

Rect::Rect(const vec &pos, const vec &xdir, const vec &ydir, float width, float height, GLfloat camMat[4][4])
{
  vec temp;
  position = pos;
  p[0] = (pos - xdir * width / 2 - ydir * height / 2).transform2d(camMat);
  p[1] = (pos - xdir * width / 2 + ydir * height / 2).transform2d(camMat);
  p[2] = (pos + xdir * width / 2 + ydir * height / 2).transform2d(camMat);
  p[3] = (pos + xdir * width / 2 - ydir * height / 2).transform2d(camMat);
  for (int j = 4; j > 2; j--)
    {
      for (int i = 1; i < j; ++i)
        if (p[i - 1].y() < p[i].y())
          {
            temp = p[i - 1];
            p[i - 1] = p[i];
            p[i] = temp;
          }
    }
  if (p[0].x() > p[1].x())
    {
      temp = p[0];
      p[0] = p[1];
      p[1] = temp;
    }
  if (p[2].x() < p[3].x())
    {
      temp = p[2];
      p[2] = p[3];
      p[3] = temp;
    }
}

bool Rect::intersects(const Rect &rhs) const
{
  bool b = false;
  for (int i = 0; i < 4 && b == false; ++i)
    for (int j = 0; j < 4 && b == false; ++j) b = testit(p[i], p[(i + 1) % 4], rhs.p[j], rhs.p[(j + 1) % 4]);
  return b;
}

bool Rect::testit(vec a1, vec a2, vec b1, vec b2) const
{
  vec temp, d1, d2, s;
  float mat[2][3], n;
  bool b = false;
  float f;
  if (a1.x() > a2.x())
    {
      temp = a1;
      a1 = a2;
      a2 = temp;
    }
  if (b1.x() > b2.x())
    {
      temp = b1;
      b1 = b2;
      b2 = temp;
    }
  d1 = a2 - a1;
  d2 = b2 - b1;
  mat[0][0] = d1.x();
  mat[0][1] = -d2.x();
  mat[0][2] = b1.x() - a1.x();
  mat[1][0] = d1.y();
  mat[1][1] = -d2.y();
  mat[1][2] = b1.y() - a1.y();
  if (mat[0][0] == 0)
    for (int i = 0; i < 3; ++i)
      {
        f = mat[0][i];
        mat[0][i] = mat[1][i];
        mat[1][i] = f;
      }
  if (mat[0][0] != 0)
    {
      f = mat[0][0];
      for (int i = 0; i < 3; ++i) mat[0][i] /= f;
    }
  f = mat[1][0];
  for (int i = 0; i < 3; ++i) mat[1][i] -= mat[0][i] * f;
  if ((mat[1][1] == 0 && mat[1][2] == 0) || (mat[1][1] == 0 && mat[1][2] != 0))
    b = false;
  else
    {
      n = mat[1][2] / mat[1][1];
      s = b1 + n * d2;
      b = (s.x() >= a1.x() && s.x() <= a2.x());
      b = (s.x() >= b1.x() && s.x() <= b2.x() && b);
      if (a1.y() > a2.y())
        {
          temp = a1;
          a1 = a2;
          a2 = temp;
        }
      if (b1.y() > b2.y())
        {
          temp = b1;
          b1 = b2;
          b2 = temp;
        }
      b = (s.y() >= a1.y() && s.y() <= a2.y() && b);
      b = (s.y() >= b1.y() && s.y() <= b2.y() && b);
    }
  return b;
}
