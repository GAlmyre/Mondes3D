#version 330 core

in vec3 var_color;

in vec3 o_n;
in vec3 o_v;
in vec3 o_l;
in vec3 o_dCol;
in vec3 o_sCol;
in float o_s;

out vec4 out_color;


vec3 blinn(vec3 n, vec3 v, vec3 l, vec3 dCol, vec3 sCol, float s) {
  vec3 h =(v+l)/2;
  vec3 color = max(dot(n,l),0)*(dCol+sCol*pow(max(dot(n,h),0),s));
  return color;
}

void main(void) {
  out_color = vec4(blinn(o_n, o_v, o_l, o_dCol, o_sCol, o_s),1);
}
