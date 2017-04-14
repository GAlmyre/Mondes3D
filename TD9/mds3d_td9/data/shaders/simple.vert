#version 330 core
#define M_PI 3.1415926535897932384626433832795

uniform mat4 obj_mat;
uniform mat4 proj_mat;
uniform mat4 view_mat;
uniform mat3 normal_mat;
uniform float torsion;

in vec3 vtx_position;
in vec3 vtx_normal;
in vec2 vtx_texcoord;

out vec3 v_normal;
out vec3 v_view;
out vec2 v_uv;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

vec3 square(vec3 center, vec3 X, vec3 Y, float p, float r, out vec3 n) {
  float a = cos(2*M_PI*p);
  float b = sin(2*M_PI*p);
  float max = max(abs(a), abs(b));
  float nt = mod(p,1);

  if(nt < 1./8)
    n = X;
  else if( nt < 3./8)
    n = Y;
  else if(nt < 5./8)
    n = -X;
  else if(nt < 7./8)
    n= -Y;
  else
    n = X;

  vec3 res = center + r*(X*a+Y*b)/max;
  return res;
}

vec3 circle(vec3 center, vec3 X, vec3 Y, float p, float r, out vec3 n) {
  n = (X*cos(2*M_PI*p)+Y*sin(2*M_PI*p));
  vec3 res = center + r * n;
  return res;
}

vec3 cylinder(vec2 uv, vec3 A, vec3 B, float r) {
  vec3 AB = A-B;
  vec3 N = normalize(cross(AB, vec3(AB.x+1,AB.y+37,AB.z-0.5789)));
  vec3 Bin = normalize(cross(AB,N));

  float a = sin(2*M_PI*uv.x);
  float b = cos(2*M_PI*uv.x);
  vec3 n;
  vec3 res = circle(A, N, Bin, uv.x, r, n) + uv.y*AB;

  return res;
}

vec3 bezier(float u, vec3 B[4], out mat3 F) {

  vec4 matU = vec4(1,u,u*u,u*u*u);
  mat4 coeff_mat = mat4(vec4(1,0,0,0),vec4(-3,3,0,0),vec4(3,-6,3,0),vec4(-1,3,-3,1));
  mat4x3 matB = mat4x3(B[0],B[1],B[2],B[3]);
  vec3 Pu = matB*coeff_mat*matU;

  // repère de Frenet
  vec3 T = matB*coeff_mat*vec4(0,1,2*u,3*u*u);
  vec3 tmp = matB*coeff_mat*vec4(0,0,2,6*u);
  vec3 Bin = cross(T, tmp);
  vec3 N = cross(T, Bin);

  F = mat3(normalize(T),normalize(Bin),normalize(N));

  return Pu;
}

vec3 cylBezierYZ(vec2 uv, vec3 B[4], float r, out vec3 n) {

  mat3 F;
  vec3 res = bezier(uv.x, B, F);
  mat4 rotation = transpose(inverse(rotationMatrix(F[0], torsion*uv.x)));
  //vec3 ret = circle(res, (rotation*vec4(F[1],1)).xyz, (rotation*vec4(F[2],1)).xyz ,uv.y,r, n);
  vec3 ret = square(res, (rotation*vec4(F[1],1)).xyz, (rotation*vec4(F[2],1)).xyz ,uv.y,r, n);
  return ret;

}

void main()
{
  v_uv  = vtx_texcoord;
  //vec3 A = vec3(1,0.5,0.5);
  //vec3 B = vec3(30,15,15);

  /* COURBE PLANAIRE X-Z
  vec3 B[4];
  B[0] = vec3(-1,0,2);
  B[1] = vec3(-0.3,0,4);
  B[2] = vec3(0.3,0,1);
  B[3] = vec3(1,0,-0.5);
  //*/

  //* COURBE "HELICE"
  vec3 B[4];
  B[0] = vec3(-0.5,-1,-1);
  B[1] = vec3(1.5,1,-0.3);
  B[2] = vec3(-1.5,1,0.3);
  B[3] = vec3(0.5,-1,1);
  //*/

  /*
  vec3 B[4];
  B[0] = vec3(-1,-0.5,-1);
  B[1] = vec3(-1,1,-0.3);
  B[2] = vec3(1,-1,0.3);
  B[3] = vec3(1,0.5,1);
  //*/

  vec3 n;
  //v_normal = normalize(normal_mat * vtx_normal);
  //vec4 p = view_mat * (obj_mat * vec4(vtx_position, 1.));
  //vec3 cyl = cylinder(vtx_texcoord, A, B, 1);
  vec3 cyl = cylBezierYZ(vtx_texcoord, B, 0.1, n);
  vec4 p = view_mat * (obj_mat * vec4(cyl, 1.));
  v_view = normalize(-p.xyz);
  gl_Position = proj_mat * p;
  v_normal = n;
  v_normal = normalize(normal_mat*v_normal);
}
