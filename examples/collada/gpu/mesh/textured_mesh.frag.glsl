#version 330 compatibility

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

uniform mat4 light_pads;
uniform mat4 material_adse;
uniform  float material_shininess;

uniform sampler2D diffuse_texture;

in vec4 eye_position;
in vec4 frag_normal;
in vec2 frag_uv;

invariant out vec4 frag_color;

vec4 phong_shading(vec4 texel, vec4 norm) {
  vec4 light_dir;
  if(0.0 == light_pads[0].w)
    light_dir = normalize( light_pads[0] );
  else
    light_dir = normalize( light_pads[0] - eye_position );
  float lambertian = max( dot(light_dir,norm), 0.0 );
  vec4 ambient = light_pads[1] * material_adse[0];
  vec4 diffuse =  texel + ( (light_pads[2] * material_adse[1]) * lambertian );
  vec4 result = ambient + diffuse;
  if( lambertian > 0.0 ) {
    vec4 view_dir = normalize( -eye_position );
    vec4 reflect_dir = reflect( -light_dir, norm );
    float spec_angle = max(dot(reflect_dir,view_dir), 0.0);
    float shininess = pow( spec_angle, material_shininess);
    vec4 specular = (light_pads[3] * material_adse[2]) * shininess;
    result += specular;
  }
  return result;
}

void main(void) {
  vec4 texel = texture(diffuse_texture, frag_uv);
  if( gl_FrontFacing ) {
    frag_color = phong_shading(texel, frag_normal );
  } else {
    frag_color = phong_shading(texel, -frag_normal );
  }
}
