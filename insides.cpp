// CSCI 447 Scientific Visualization, Spring 2013
// Project 1: Volume Rendering
// Steven Ruppert

#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <string>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

#include "trackball.hpp"

using namespace std;

/* ---------------------------------------------- */
// volume data related parameters -- need to be set before the first frame is rendered

int size[3];  // size of the volume in each of the 3 dimensions
              // ROI is assumed to extend from 0 to size[0]-1 in x,
              // 0 to size[1]-1 in y and 0 to size[2]-1 in z

/* --------------------------------------------- */
/* ----- GLOBAL VARIABLES  --------------------- */
/* --------------------------------------------- */

static const int VPD_DEFAULT = 800;
static const double TWOPI = (2.0 * M_PI);
trackball *trckb;
static GLfloat zoom = 3;

static bool leftd = false;
static bool middled = false;
static int lasty;

GLint wid;               /* GLUT window id; value asigned in main() and should stay constant */
GLint vpw = VPD_DEFAULT; /* viewport dimensions; change when window is resized (resize callback) */
GLint vph = VPD_DEFAULT;

GLuint ProgramHandle;   // this is an OpenGL name/ID number of the program object
// compiled in SetUpProgram

// locations of uniform variables - values assigned in SetUpProgram
GLint PLoc, RLoc, SizeLoc, MaxLoc, fwdLoc;

// vertex array object for cube extending from -1 to 1 in all dimensions
GLint CubeVAO;

GLuint TexHandle;  // texture handle

/* --------------------------------------------- */
/* ----- TEXTURE SETUP ------------------------- */
/* --------------------------------------------- */

// This sets up the texture, and should work for the 8-bit datasets

int TextureSetup ( const char *name, int sizex, int sizey, int sizez )
{
  // this is to make it easy to work with non-power of two textures
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  size[0] = sizex;
  size[1] = sizey;
  size[2] = sizez;
  GLubyte *tex = new GLubyte[sizex*sizey*sizez];
  ifstream ifs(name,ios::binary);
  ifs.read((char*)tex,sizex*sizey*sizez*sizeof(GLubyte));
  glGenTextures(1,&TexHandle);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D,TexHandle);
  glTexImage3D(GL_TEXTURE_3D,0,GL_R8,sizex,sizey,sizez,0,GL_RED,GL_UNSIGNED_BYTE,tex);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}

/* --------------------------------------------- */
/* ----- SHADER RELATED CODE ------------------- */
/* --------------------------------------------- */

// read entire file into a string

string ReadFromFile ( const char *name )
{
  ifstream ifs(name);
  if (!ifs)
    {
      cout << "Can't open " << name << endl;
      exit(1);
    }
  stringstream sstr;
  sstr << ifs.rdbuf();
  return sstr.str();
}

/* --------------------------------------------- */

// function for printing compilation error/warning messages etc

void PrintInfoLog ( GLuint obj )
{
  int infologLength = 0;
  int maxLength;

  if(glIsShader(obj))
    glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
  else
    glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);

  char infoLog[maxLength];

  if (glIsShader(obj))
    glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
  else
    glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);

  if (infologLength > 0)
    cout << infoLog << endl;
}

/* --------------------------------------------- */

// set up program

void SetUpProgram()
{
  GLuint vid = glCreateShader(GL_VERTEX_SHADER);
  GLuint fid = glCreateShader(GL_FRAGMENT_SHADER);

  string VertexSrc = ReadFromFile("vertex.glsl");
  string FragmentSrc = ReadFromFile("fragment.glsl");

  const char *VS = VertexSrc.c_str();
  const char *FS = FragmentSrc.c_str();

  // set source of the shaders
  glShaderSource(vid,1,&VS,NULL);
  glShaderSource(fid,1,&FS,NULL);

  // compile
  glCompileShader(vid);
  glCompileShader(fid);

  // any problems?
  cout << "Vertex Shader Log: " << endl;
  PrintInfoLog(vid);
  cout << "Fragment Shader Log: " << endl;
  PrintInfoLog(fid);

  // create program
  ProgramHandle = glCreateProgram();

  // attach vertex and fragment shadets
  glAttachShader(ProgramHandle,vid);
  glAttachShader(ProgramHandle,fid);

  // link
  glLinkProgram(ProgramHandle);

  // any problems?
  cout << "Linker Log: " << endl;
  PrintInfoLog(ProgramHandle);

  // get uniform variable locations so that they can be set from C code

  PLoc = glGetUniformLocation(ProgramHandle,"P");
  //  assert(PLoc!=-1);

  RLoc = glGetUniformLocation(ProgramHandle,"R");
  //  assert(RLoc!=-1);

  SizeLoc = glGetUniformLocation(ProgramHandle,"Size");
  //  assert(SizeLoc!=-1);

  fwdLoc = glGetUniformLocation(ProgramHandle,"fwd");
  //  assert(fwdLoc!=-1);
}

/* --------------------------------------------- */
/* ---- GEOMETRY BUFFER SETUP ------------------ */
/* --------------------------------------------- */

// set up a Vertex Array Object (VAO) with two Vertex Buffer Objects (VBOs)
//  containing geometry of the cube extending from 0 to 1 in all dimensions
// returns handle of the VAO;
// note that it is called ONLY at startup - you should do the same!

GLuint setup_cube ( )
{
  GLfloat coordarray[] =
    {
      1.0,1.0,0.0,
      1.0,0.0,0.0,
      0.0,1.0,0.0,

      1.0,0.0,0.0,
      0.0,0.0,0.0,
      0.0,1.0,0.0,

      1.0,1.0,1.0,
      0.0,1.0,1.0,
      1.0,0.0,1.0,

      0.0,1.0,1.0,
      0.0,0.0,1.0,
      1.0,0.0,1.0,

      1.0,0.0,1.0,
      1.0,0.0,0.0,
      1.0,1.0,1.0,

      1.0,0.0,0.0,
      1.0,1.0,0.0,
      1.0,1.0,1.0,

      0.0,0.0,1.0,
      0.0,1.0,1.0,
      0.0,0.0,0.0,

      0.0,1.0,1.0,
      0.0,1.0,0.0,
      0.0,0.0,0.0,

      0.0,1.0,1.0,
      1.0,1.0,1.0,
      0.0,1.0,0.0,

      1.0,1.0,1.0,
      1.0,1.0,0.0,
      0.0,1.0,0.0,

      0.0,0.0,0.0,
      1.0,0.0,0.0,
      0.0,0.0,1.0,

      1.0,0.0,0.0,
      1.0,0.0,1.0,
      0.0,0.0,1.0,
    };

  // generate vertex array object handle
  GLuint vao;
  glGenVertexArrays(1,&vao);

  // ... and work with it...
  glBindVertexArray(vao);

  // generate vertex buffer object
  GLuint vbo;
  glGenBuffers(1,&vbo);

  // ... and work with it... it will contain vertex coords
  glBindBuffer(GL_ARRAY_BUFFER,vbo);

  // send the array of vertex coordinates to the vbo (basically, CPU->GPU)
  glBufferData(GL_ARRAY_BUFFER,12*3*3*sizeof(GLfloat),coordarray,GL_STATIC_DRAW);

  // make it attribute #0 -- this info will be stored in the active VAO
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);

  // enable attributes # 0 and 1 -- this info will be stored in the active VAO
  glEnableVertexAttribArray(0);

  // finish work on our buffers/arrays
  glBindBuffer(GL_ARRAY_BUFFER,0);
  glBindVertexArray(0);

  return vao;
}

/* --------------------------------------------- */

/* --------------------------------------------- */
/* ---- RENDERING ------------------------------ */
/* --------------------------------------------- */

// draw all cubes

void draw ( )
{
  /* ensure we're drawing to the correct GLUT window */
  glutSetWindow(wid);

  // straightforward OpenGL settings
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glEnable(GL_DEPTH_TEST);

  /* clear the color buffers */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use our program....
  // note that there may be several program handles in a more complex OpenGL code
  //    - we have just one here though
  glUseProgram(ProgramHandle);

  // ----------- this part of code sets the uniform variable values

  // now the matrices... Arguments: field of view, aspect, front/back clip plane distances
  glm::mat4 PMat = glm::perspective(float(zoom),1.0f,18.0f,22.0f);

  // send the matrix to shaders; note that &P[0][0] returns the pointer to matrix entries
  //   in an OpenGL-friendly format
  glUniformMatrix4fv(PLoc,1,GL_FALSE,&PMat[0][0]);

  glm::mat3 rmat = trckb->nm();  // get rotation matrix from trackball

  // pass rotation matrix to the shaders...
  glUniformMatrix3fv(RLoc,1,GL_FALSE,&rmat[0][0]);

  // take care of all other uniform variables
  glUniform1f(fwdLoc,20.0);  // mid-way between front and back clip planes

  glUniform3f(SizeLoc,size[0]-1.0,size[1]-1.0,size[2]-1.0);  // extent of the data

  // ----------- draw the first cube

  // work with our vertex array object
  // note: DONT set up buffers here -- this code is executed every frame!
  //  set them up at startup and just use what you created here
  glBindVertexArray(CubeVAO);

  // draw the current VAO
  // note that the last argument is the number of VERTICES sent into the pipeline
  // 12*3: 12 triangles, 3 vertices each
  glDrawArrays(GL_TRIANGLES,0,12*3);

  // unbind our VAO -- this is not necessary here, but it's good to do
  // once you are done working with it
  glBindVertexArray(0);

  /* flush the pipeline */
  glFlush();

  /* look at our handiwork */
  glutSwapBuffers();
}

/* --------------------------------------------- */
/* --- INTERACTION ----------------------------- */
/* --------------------------------------------- */

/* handle mouse events */

void mouse_button(GLint btn, GLint state, GLint mx, GLint my)
{
  switch( btn ) {
    case GLUT_LEFT_BUTTON:
      switch( state ) {
        case GLUT_DOWN:
	  leftd = true;
	  trckb->mousedown(mx,my);
	  break;
        case GLUT_UP:
	  leftd = false;
	  trckb->mouseup(mx,my);
	  break;
      }
      break;
    case GLUT_MIDDLE_BUTTON:
      switch( state ) {
        case GLUT_DOWN:
	  middled = true;
	  lasty = my;
	  break;
        case GLUT_UP:
	  middled = false;
	  break;
      }
      break;
  }
  glutPostRedisplay();
}

/* --------------------------------------------- */

// mouse moves with button down

GLvoid button_motion(GLint mx, GLint my)
{
  if (leftd)
    trckb->mousemove(mx,my);
  if (middled)
    {
      int dy = my-lasty;
      zoom += dy/10.0;
      if (zoom<=0.01) zoom = 0.01;
      if (zoom>=179) zoom = 179;
      lasty = my;
    }
  glutPostRedisplay();
}

/* --------------------------------------------- */

/* handle keyboard events; here, just exit if ESC is hit */

void keyboard(GLubyte key, GLint x, GLint y)
{
  switch(key) {
    case 27:  /* ESC */
              exit(0);

    default:  break;
  }
}

/* --------------------------------------------- */

/* handle resizing the glut window */

GLvoid reshape(GLint sizex, GLint sizey)
{
  glutSetWindow(wid);

  vpw = sizex;
  vph = sizey;

  glViewport(0, 0, vpw, vph);
  glutReshapeWindow(vpw, vph);

  glutPostRedisplay();
}

/* --------------------------------------------- */
/* -------- SET UP GLUT  ----------------------- */
/* --------------------------------------------- */

// initialize glut, callbacks etc.

GLint init_glut(GLint *argc, char **argv)
{
  GLint id;

  glutInit(argc,argv);

  /* size and placement hints to the window system */
  glutInitWindowSize(vpw, vph);
  glutInitWindowPosition(10,10);

  /* double buffered, RGB color mode */
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  /* create a GLUT window (not drawn until glutMainLoop() is entered) */
  id = glutCreateWindow("Volume Rendering to be");

  /* register callbacks */

  /* window size changes */
  glutReshapeFunc(reshape);

  /* keypress handling when the current window has input focus */
  glutKeyboardFunc(keyboard);

  /* mouse event handling */
  glutMouseFunc(mouse_button);           /* button press/release        */
  glutMotionFunc(button_motion);         /* mouse motion w/ button down */
  glutPassiveMotionFunc(NULL);           /* mouse motion with button up */

  /* window obscured/revealed event handler */
  glutVisibilityFunc(NULL);

  /* handling of keyboard SHIFT, ALT, CTRL keys */
  glutSpecialFunc(NULL);

  /* what to do when mouse cursor enters/exits the current window */
  glutEntryFunc(NULL);

  /* what to do on each display loop iteration */
  glutDisplayFunc(draw);

  return id;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GLint main(GLint argc, char **argv)
{

  /* initialize GLUT: register callbacks, etc */
  wid = init_glut(&argc, argv);

  trckb = new trackball(vpw,vph);

  // initialize glew and check for OpenGL 4.0 support
  glewInit();
  if (glewIsSupported("GL_VERSION_4_0"))
    cout << "Ready for OpenGL 4.0" << endl;
  else
    {
      cout << "OpenGL 4.0 not supported" << endl;;
      return 1;
    }

  // set up vertex/fragment programs
  SetUpProgram();

  // set up Vertex Array Object containing the cube geometry
  CubeVAO = setup_cube();

  // set up the texture here using the TextureSetup function
  // for example: TextureSetup("bonsai.raw",256,256,256);
  // the three integer arguments are the resolution of the data

  // main loop: keep processing events until exit is requested
  glutMainLoop();

  return 0;
}


/* --------------------------------------------- */
