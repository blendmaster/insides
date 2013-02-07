
#ifndef __TRACKBALL_H
#define __TRACKBALL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* ---------------------------------------------- */

class trackball {

  glm::mat4 R;     // finished rotations
  glm::mat4 R0;    // current rotation
  glm::mat4 R0R;
  glm::mat3 NM;

  int winsize[2];  // window size

  bool ismousedown;  // true if dragging
  glm::vec3 last;  // last mouse down event
  int lasti, lastj;

  glm::vec3 _ij2xyz ( int i, int j );

 public:

  trackball ( int sx, int sy );
  void resize ( int sx, int sy );

  void mousedown ( int i, int j );
  void mousemove ( int i, int j );
  void mouseup ( int i, int j );

  glm::mat4 mv();  // modelview rotation
  glm::mat3 nm();  // normal rotation

  float *mvpointer();   // returns the modelview matrix
  float *nmpointer();   // returns the normal matrix

  bool isactive();   // is mouse button down?
};

/* ---------------------------------------------- */

#endif
