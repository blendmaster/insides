#version 420

// ------------------- INPUT VARIABLES ----------------------

// these are coordinates of a unit cube - all in {0,1}
layout(location=0) in vec3 coord;

// ------------------- OUTPUT VARIABLES ----------------------

out vec3 entry;   // corner of the data set; will be interpolated on rasterization stage
    // interpolated value is the entry point into the ROI behind a pixel
    // in the "model" coordinate system, in which the data set extends from
    // (0,0,0) to (sizex-1,sizey-1,sizez-1)
    // [hence the name -- possibly puzzling here, but making sense in the fragment shader]

out vec3 world;   // world coordinates of a vertex 
    // Will be interpolated to provide world coordinates of the 
    // entry point. This will make it easy to compute 
    // the eye ray direction in the fragment shader.
    // [note that the viewpoint is at the origin of the world coord sys]

// ------------------- UNIFORM VARIABLES -----------------------

uniform mat3 R;         // rotation matrix (defined by the trackball)
uniform mat4 P;         // projection matrix
uniform vec3 Size;      // [sizex-1,sizey-1,sizez-1] - basically, the size of the data set
uniform float fwd;      // amount of forward translation with respect to camera
      // (half way between front and back clipping planes)

// -------------------- THE SHADER ----------------------------

void main() {

  entry = Size*coord;     // corner of the data set
                                // note that * is the coordinate-wise multiplication
                                // [use dot(.,.) to compute dot product in GLSL]
        // basically, we are applying a non-uniform scale
        // with scaling factors equal to Size.x/y/z

  world = 1.0/length(Size)*R*(entry-0.5*Size)-vec3(0,0,fwd);
    // the sequence of transformations applied to corner:
    //  - translate by -0.5*dataset size [this takes its center to origin]
    //  - apply the rotation R -- provided by the trackball interface [see C code]
    //  - scale by 1/diagonal of the data set
    // this should cause the data set to fit into the view volume nicely

  gl_Position = P*vec4(world,1);  // apply projection to the world coords of the vertex
    // note that world is of type vec3 and P is 4x4 so the homogenous coordinate
    // needs to be added
}
