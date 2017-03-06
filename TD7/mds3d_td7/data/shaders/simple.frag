#version 330 core

in vec3 v_color;
in vec3 v_normal;
in vec3 v_view;
in vec2 v_texture;

uniform vec3 lightDir;
uniform sampler2D sampler;
uniform sampler2D clouds_sampler;

out vec4 out_color;

vec3 blinn(vec3 n, vec3 v, vec3 l, vec3 dCol, vec3 sCol, float s)
{
  vec3 res = vec3(0,0,0);
  float dc = max(0,dot(n,l));
  if(dc>0) {
    res = dCol * dc;
  }
  return res;
}

void main(void) {
  float ambient = 0.4;
  float shininess = 50;
  vec3 spec_color = vec3(1,1,1);
  vec3 color_earth = texture(sampler, v_texture).xyz;
  vec3 color_clouds = texture(clouds_sampler, v_texture).xyz;
  vec3 color = mix(color_clouds, color_earth,0.5);
  out_color = vec4(ambient * color + blinn(normalize(v_normal),normalize(v_view), lightDir, color, spec_color, shininess),1.0);
}
