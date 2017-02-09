#version 330 core

in vec3 var_color;

out vec4 out_color;
uniform float color;

void main(void) {
  if(color == 1)
    out_color = vec4(var_color,1);
  if(color == 2)
    out_color = vec4(1,1,1,1);
}
