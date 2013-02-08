#version 420

// ------------------- INPUT VARIABLES ----------------------

in vec3 entry;      // entry point in the model coordinate system
in vec3 world;      // world coordinates of the entry point

// ------------------- OUTPUT VARIABLE ----------------------

out vec4 fragcolor; // the final color of the fragment

// ------------------- UNIFORM VARIABLES -----------------------

uniform mat3 R;         // rotation matrix (defined by the trackball)
uniform vec3 Size;      // [sizex-1,sizey-1,sizez-1] - basically, the size of the data set

layout (binding=0) uniform sampler3D tex;  // this is the texture, plugged into
        // the texture attachment point #0
        // you'll probably want to keep it that way
        // note that this may lead to an error if your
        // hardware does not support OpenGL 4.2 -- let me
        // know if this comes up (an easy alternative exists)

// -------------------- THE SHADER ----------------------------

// look up model value
float valueAt(vec3 coords) {
  // The coordinates need to be scaled to [0,1] when sampling the texture.
  // (on the OpenGL side, the texture is specified as the red channel texture --
  //  see the C code).

  return texture(tex,(1/Size.x,1/Size.y,1/Size.z) * coords).r;
}

void main() {

  // Since the viewpoint in world coordinates is at (0,0,0),
  //    "world" is the direction of the eye ray.
  // However, it needs to be translated into the model coordinate
  //    system since this is what we'll be working with when
  //    performing texture lookup.
  // The rotation of the dataset (R) needs to be undone.
  // Hence inverse(R)=transpose(R) needs to be applied

  vec3 dir = transpose(R)*world;  // direction of the ray in model coordinate system

  // At this point, you have all the data to determine the samples
  // in the model coordinates.

  // if you compute:
  // vec3 onestep = step_size*normalize(dir);
  // then the i-th sample is at entry+i*dir (in the model coordinate system)

  vec3 delta = 0.01 * normalize(dir);

  vec3 loc = entry;
  for (int i = 0; i < 100; ++i) {
    if (valueAt(loc) > 0.0) {
      fragcolor = vec4(1,0,0,1);   // just make the fragment white
      return;
    }
    loc += delta;
  }

  // Include your compositing routine here.
  // A couple of things to remember/guidelines:
  //   -- Values in the texture (which were sent as 8-bit unsigned ints)
  //  are going to appear as floating point values in [0,1];
  //      this means that you need to scale the "interesting isovalues"
  //  listed somewhere in the project directory by 1.0/255.0
  //   -- Texture boundary can lead to some odd artifacts.
  //      The easy solution is to define the step size for gradient
  //  computation in the model coordinate system (0.5 should work fine).
  //      Then, ignore all samples that are not in [1,Size.x-1]x[1,Size.y-1]x[1,Size.z-1]
  //  in your compositing algorithm. Basically, shave the data by 1 on each side.
  //  This will prevent the algorithm to sample across the boundary.
  //   -- Beware of negative color (abs H dot N and N dot L or map negative values to zero)
  //   -- Beware of zero gradient: it's going to happen. In particular, don't normalize
  //  zero gradients. A simple trick to avoid problems is to return a tiny but nonzero
  //  gradient it the gradient is even tinier. Then, you don't have to worry about
  //  nonzero gradients at all.
  //   -- Some useful built-in functions: float length(vec3), float dot(vec3,vec3)
  //  pow(float,float) [it should be clear what they do]; see GLSL specification
  //  for more.
  //   -- I think things are going to be easier if you consistently express
  //  all the important parameters in the model coordinate units.
  //   This includes step sizes (for samples and gradient) and the distance 
  //  parameter for the isosurface transfer function.
  //  Remember about scaling before texture lookup (see above).

  fragcolor = vec4(0,0,0,1);   // just make the fragment white
}
