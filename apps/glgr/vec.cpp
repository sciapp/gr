#include "vec.h"
#include <math.h>

vec::vec()
{
  v[0] = 0;
  v[1] = 0;
  v[2] = 0;
}

vec::vec(float x, float y, float z)
{
  v[0] = x;
  v[1] = y;
  v[2] = z;
}

vec::vec(float *x)
{
  v[0] = x[0];
  v[1] = x[1];
  v[2] = x[2];
}

vec::vec(const vec &rhs)
{
  v[0] = rhs.x();
  v[1] = rhs.y();
  v[2] = rhs.z();
}

const vec &vec::operator=(const vec &rhs)
{
  if (this == &rhs) return *this;
  v[0] = rhs.x();
  v[1] = rhs.y();
  v[2] = rhs.z();
  return *this;
}

const vec &vec::operator+=(const vec &rhs)
{
  v[0] += rhs.x();
  v[1] += rhs.y();
  v[2] += rhs.z();
  return *this;
}

const vec &vec::operator-=(const vec &rhs)
{
  v[0] -= rhs.x();
  v[1] -= rhs.y();
  v[2] -= rhs.z();
  return *this;
}

const vec &vec::operator*=(double rhs)
{
  v[0] *= rhs;
  v[1] *= rhs;
  v[2] *= rhs;
  return *this;
}

const vec &vec::operator/=(double rhs)
{
  v[0] /= rhs;
  v[1] /= rhs;
  v[2] /= rhs;
  return *this;
}

vec vec::operator-(void) const
{
  return *this * -1;
}

const vec vec::transform2d(GLfloat camMat[4][4]) const
{
  float x, y;
  x = camMat[0][0] * v[0] + camMat[1][0] * v[1] + camMat[2][0] * v[2];
  y = camMat[0][1] * v[0] + camMat[1][1] * v[1] + camMat[2][1] * v[2];
  return vec(x, y, 0);
}

const vec vec::transform3d(GLfloat camMat[4][4]) const
{
  float x, y, z;
  x = camMat[0][0] * v[0] + camMat[1][0] * v[1] + camMat[2][0] * v[2];
  y = camMat[0][1] * v[0] + camMat[1][1] * v[1] + camMat[2][1] * v[2];
  z = camMat[0][2] * v[0] + camMat[1][2] * v[1] + camMat[2][2] * v[2];
  return vec(x, y, z);
}

float vec::quadNorm(void) const
{
  return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

const vec vec::abs(void) const
{
  return vec(fabs(v[0]), fabs(v[1]), fabs(v[2]));
}

const vec operator+(const vec &lhs, const vec &rhs)
{
  return vec(lhs) += rhs;
}

const vec operator-(const vec &lhs, const vec &rhs)
{
  return vec(lhs) -= rhs;
}

const vec operator*(const vec &lhs, double z)
{
  return vec(lhs) *= z;
}

const vec operator*(double z, const vec &rhs)
{
  return vec(rhs) *= z;
}

const vec operator/(const vec &lhs, double z)
{
  return vec(lhs) /= z;
}

const vec operator*(GLfloat camMat[4][4], const vec &rhs)
{
  float x, y, z;
  x = camMat[0][0] * rhs.x() + camMat[1][0] * rhs.y() + camMat[2][0] * rhs.z();
  y = camMat[0][1] * rhs.x() + camMat[1][1] * rhs.y() + camMat[2][1] * rhs.z();
  z = camMat[0][2] * rhs.x() + camMat[1][2] * rhs.y() + camMat[2][2] * rhs.z();
  return vec(x, y, z);
}

bool operator==(const vec &lhs, const vec &rhs)
{
  return (lhs.x() == rhs.x() && lhs.y() == rhs.y() && lhs.z() == rhs.z());
}

bool operator!=(const vec &lhs, const vec &rhs)
{
  return (lhs.x() != rhs.x() || lhs.y() != rhs.y() || lhs.z() != rhs.z());
}

bool operator!=(const vec &lhs, const vec &rhs);
