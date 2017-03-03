#version 330 core

in vec3 vtx_color;
in vec3 vtx_position;
in vec3 vtx_normal;
uniform mat4 transformMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat3 normal_mat;
uniform vec3 spec_color;
uniform vec3 light_dir;
uniform float specular;

out vec3 o_n;
out vec3 o_v;
out vec3 o_l;
out vec3 o_dCol;
out vec3 o_sCol;
out float o_s;

out vec3 var_color;

void main() {

  vec3 normal = normal_mat*vtx_normal;
  normal = normalize(normal);
  var_color.x = abs(normal.x);
  var_color.y = abs(normal.y);
  var_color.z = abs(normal.z);

  vec4 p = vec4(vtx_position,1);
  gl_Position = projectionMatrix*viewMatrix*transformMatrix*p;
  vec4 position = viewMatrix*transformMatrix*p;
  vec4 light = transpose(inverse(viewMatrix))*vec4(light_dir,1);
  o_n = normal;
  o_v = normalize(-position.xyz);
  o_l = normalize(light.xyz);
  o_dCol = vtx_color;
  o_sCol = spec_color;
  o_s = specular;

}
