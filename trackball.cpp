
#include "trackball.hpp"
#include <cmath>

using namespace glm;

/* ---------------------------------------------- */

trackball::trackball ( int sx, int sy ) : R(), R0(), R0R(), ismousedown(false)
{
  winsize[0] = sx;
  winsize[1] = sy;

}

/* ---------------------------------------------- */

void trackball::resize ( int sx, int sy )
{
  winsize[0] = sx;
  winsize[1] = sy;
}

/* ---------------------------------------------- */

vec3 trackball::_ij2xyz ( int i, int j )
{
  float x = 2*((float)i)/winsize[0]-1;
  float y = 1-2*((float)j)/winsize[1];
  float x2y2 = 1-x*x-y*y;

  if (x2y2<0)
    {
      double l = sqrt(x*x+y*y);
      x = x/l;
      y = y/l;
      x2y2 = 0;

    }

  float z = sqrt(x2y2);

  return vec3(x,y,z);
}

/* ---------------------------------------------- */

void trackball::mousedown ( int i, int j )
{
  ismousedown = true;
  lasti = i;
  lastj = j;
  last = _ij2xyz(i,j);
}

/* ---------------------------------------------- */

void trackball::mousemove ( int i, int j )
{
  if (i==lasti && j==lastj)
    {
      R0 = glm::mat4();
      return;
    }

  vec3 cur = _ij2xyz(i,j);
  vec3 axis = cross(last, cur);
  R0 = rotate(mat4(),float(acos(dot(last,cur))/M_PI*180.0),axis);
  R0R = R0*R;
}

/* ---------------------------------------------- */

void trackball::mouseup ( int i, int j )
{
  if (!ismousedown)
    return;
  ismousedown = false;
  if (i==lasti && j==lastj)
    {
      R0 = mat4();
      return;
    }

  vec3 cur = _ij2xyz(i,j);
  vec3 axis = cross(last, cur);
  R0 = rotate(mat4(),float(acos(dot(last,cur))/M_PI*180.0),axis);
  R = R0*R;
  R0 = dmat4();
  R0R = R;
}

/* ---------------------------------------------- */

float *trackball::mvpointer()
{
  return &R0R[0][0];
}

/* ---------------------------------------------- */

float *trackball::nmpointer()
{
  NM = mat3(R0R);
  return &NM[0][0];
}

/* ---------------------------------------------- */

mat4 trackball::mv()
{
  return R0R;
}

/* ---------------------------------------------- */

mat3 trackball::nm()
{
  NM = mat3(R0R);
  return NM;
}

/* ---------------------------------------------- */

bool trackball::isactive()
{
  return ismousedown;
}

/* ---------------------------------------------- */

