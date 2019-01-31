#ifndef VEK_H_INCLUDED
#define VEK_H_INCLUDED

#include <qgl.h>

class vec
{
private:
  float v[3];

public:
  vec(float *x);
  vec(float x, float y, float z = 0);
  vec();
  vec(const vec &rhs);
  void setX(float value) { v[0] = value; }
  void setY(float value) { v[1] = value; }
  void setZ(float value) { v[2] = value; }
  float x() const { return v[0]; }
  float y() const { return v[1]; }
  float z() const { return v[2]; }
  const float *getv() const { return v; };
  const vec &operator=(const vec &rhs);
  const vec &operator+=(const vec &rhs);
  const vec &operator-=(const vec &rhs);
  const vec &operator*=(double rhs);
  const vec &operator/=(double rhs);
  vec operator+(void) const { return *this; }
  vec operator-(void) const;
  const vec transform2d(GLfloat camMat[4][4]) const;
  const vec transform3d(GLfloat camMat[4][4]) const;
  float quadNorm(void) const;
  const vec abs(void) const;
};

const vec operator+(const vec &lhs, const vec &rhs);
const vec operator-(const vec &lhs, const vec &rhs);
const vec operator*(const vec &lhs, double z);
const vec operator*(double z, const vec &lhs);
const vec operator/(const vec &lhs, double z);
const vec operator*(GLfloat camMat[4][4], const vec &rhs);

bool operator==(const vec &lhs, const vec &rhs);
bool operator!=(const vec &lhs, const vec &rhs);

#endif
