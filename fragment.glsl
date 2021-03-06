#version 420

#define E 2.71828

// ------------------- INPUT VARIABLES ----------------------

in vec3 entry;      // entry point in the model coordinate system
in vec3 world;      // world coordinates of the entry point

// ------------------- OUTPUT VARIABLE ----------------------

out vec4 fragcolor; // the final color of the fragment

// ------------------- UNIFORM VARIABLES -----------------------

uniform mat3 R;         // rotation matrix (defined by the trackball)
uniform vec3 Size;      // [sizex-1,sizey-1,sizez-1] - basically, the size of the data set

layout (binding=0) uniform sampler3D tex; 

// -------------------- THE SHADER ----------------------------

// look up model value
float valueAt(vec3 loc) {
  // The coordinates need to be scaled to [0,1] when sampling the texture.
  // (on the OpenGL side, the texture is specified as the red channel texture --
  //  see the C code).

  // because GLSL usually tiles lookups outside [0,1], clamp the coordinates
  // slightly to prevent wraparound.
  return texture(tex, clamp(loc / Size, 0.005, 0.995)).r;
}

// whether the location is within the ROI
bool inside(vec3 loc) {
  return loc.x >= 0 && loc.x <= Size.x
      && loc.y >= 0 && loc.y <= Size.y
      && loc.z >= 0 && loc.z <= Size.z;
}

#define DELTA              0.5
#define GRADIENT_DELTA     0.1
#define T_THRESHOLD        0.01
#define DIFFUSE            0.8
#define SPECULAR           0.9
#define SPECULAR_COMPONENT 20
#define LIGHT_LOC          vec3(-50, 50, 100)
#define LIGHT_INTENSITY    0.8
#define AMBIENT_INTENSITY  0.3

// coord is 0, 1, 2 (x, y z)
float centralDiff(vec3 loc, int coord) {
  vec3 high = vec3(loc);
  high[coord] += GRADIENT_DELTA;
  vec3 low = vec3(loc);
  low[coord] -= GRADIENT_DELTA;
  return valueAt(high) - valueAt(low);
}

// from the model data
vec3 gradientAt(vec3 loc) {
  return vec3(centralDiff(loc, 0),
              centralDiff(loc, 1),
              centralDiff(loc, 2));
}

// handle 0 gradients
vec3 normalGradient(vec3 gradient) {
  if (gradient.x == 0 && gradient.y == 0 && gradient.z == 0) {
    return gradient;
  } else {
    return normalize(gradient);
  }
}

// gradient is passed in to avoid recalculation
float lightAt(vec3 modelLoc, vec3 worldLoc, vec3 gradient) {
  vec3 N = R * normalGradient(gradient); // rotate into world coords
  vec3 L = normalize(worldLoc - LIGHT_LOC);
  vec3 V = normalize(worldLoc); // viewpoint is at 0,0,0

  vec3 H = normalize(V + L);

  float NdotL = max(0, dot(N, L));
  float HdotN = max(0, dot(H, N));

  return AMBIENT_INTENSITY + LIGHT_INTENSITY * (
           DIFFUSE  * NdotL +
           SPECULAR * pow(HdotN, SPECULAR_COMPONENT)
         );
}

// intensity based on distance from isosurface
float dist(float value, vec3 gradient, float isovalue, float width) {
  return max(0, 1 - abs(isovalue - value) / (width * length(gradient)));
}

// model to rgba transfer function. Comment out parts for specific models.
vec4 colorAt(vec3 loc, vec3 gradient) {
  // isosurface display
  float v = valueAt(loc);

  // bonsai
  /*float leafiness   = dist(v, gradient, 0.137254, 2);*/
  /*float branchiness = dist(v, gradient, 0.3215, 5);*/
  /*float rockiness   = dist(v, gradient, 1, 5);*/

  /*return vec4(max(0, 0.4 * branchiness + 0.5 * rockiness + 0.5 * leafiness),*/
              /*max(0, 3 * leafiness - 0.3 * branchiness + 0.2 * rockiness),*/
              /*max(0, 0.05 * branchiness + 0.1 * rockiness + 0.5 * leafiness),*/
              /*1 - (1 - branchiness * 0.1) * (1 - leafiness * 0.05) * (1 - rockiness * 2));*/

  // engine
  /*float engine  = dist(v, gradient, 0.2, 10.0);*/
  /*float insides = dist(v, gradient, 0.95, 7.0);*/

  /*return vec4(0.8 * insides + 0.3 * engine,*/
              /*0.1 * insides + 0.3 * engine,*/
              /*0.3 * engine,*/
              /*1 - (1 - engine * 0.1) * (1 - insides));*/

  // skull
  float bones = dist(v, gradient, 0.15, 5.0);
  float tubes = dist(v, gradient, 0.99, 7.0); // not sure what these are
  float teeth = dist(v, gradient, 0.40, 5.0);

  return vec4(0.5 * bones + 1 * tubes + 1 * teeth,
              0.5 * bones + 0.1 * tubes + 1 * teeth,
              0.5 * bones + 1 * teeth,
              1 - (1 - bones * 0.05) * (1 - tubes) * (1 - teeth * 0.2));
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
  vec3 deltaM = DELTA * normalize(dir);   // in model coords
  vec3 deltaW = DELTA * normalize(world); // in word coords for illumination

  vec3 I = vec3(0, 0, 0);
  float t = 1;

  for (vec3 locM = entry, locW = world;
       t > T_THRESHOLD && inside(locM);
       locM += deltaM, locW += deltaW) {

    vec3 gradient = gradientAt(locM);

    vec4 color = colorAt(locM, gradient);

    I += t * DELTA * color.a * lightAt(locM, locW, gradient) * color.rgb;
    t *= pow(E, -color.a * DELTA);
  }

  fragcolor = vec4(I, 1);
}
