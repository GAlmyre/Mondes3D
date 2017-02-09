#version 330 core

in vec3 vtx_color;
in vec3 vtx_position;
uniform float zoom;

out vec3 var_color;

void main()
{
  var_color = vtx_color;
  vec3 p = vec3(vtx_position.xy*zoom,-vtx_position.z);
  gl_Position = vec4(p, 1.);
}
