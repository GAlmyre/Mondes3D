#version 330 core

in vec3 vtx_color;
in vec3 vtx_position;
uniform mat4 transformMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
//uniform float zoom;
//uniform vec2 mvtVect;

out vec3 var_color;

vec3 blinn(vec3 n, vec3 v, vec3 l, vec3 dCol, vec3 sCol, float s) {

}

void main() {
  var_color = vtx_color;

  vec4 p = vec4(vtx_position,1);
  gl_Position = projectionMatrix*viewMatrix*transformMatrix*p;
}
