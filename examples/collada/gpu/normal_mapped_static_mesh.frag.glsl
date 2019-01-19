#version 330

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

uniform mat4 light_pads;

uniform mat4 material_adse;
uniform	float material_shininess;

uniform sampler2D diffuse_texture;
uniform sampler2D normal_map_texture;

in mat3 TBN;
in vec2 frag_uv;
in vec4 tangent_light_position;
in vec4 tangent_eye_position;

out vec4 fragment_color;
invariant fragment_color;

vec4 phong_shading(vec4 eye_pos, vec4 norm) {
	vec4 s = normalize( light_pads[0] - eye_pos );
	vec4 v = normalize( -eye_pos );
	vec4 r = reflect( -s, norm );
	vec4 ambient = light_pads[1] * material_adse[0];
	float s_dot_nrm = max( dot(s,norm), 0.0 );
	vec4  diffuse = light_pads[2] * material_adse[1] * s_dot_nrm;
	vec4 specular;
	if( s_dot_nrm > 0.0 ) {
		float shininess = pow( max( dot(r,v), 0.0 ), material_shininess);
		vec4 specular = (light_pads[3] * material_adse[2]) * shininess;
		return ambient + clamp(diffuse,0.0, 1.0) +  clamp(specular, 0.0, 1.0);
	}
	return ambient + clamp(diffuse,0.0, 1.0);
}

void main(void) {
	vec3 normal = normalize( texture(normal_map_texture, frag_uv ).rgb );
	normal = normalize( (normal * 2.0) - 1.0 );
	normal = normalize(TBN * normal);
	vec4 color = texture( diffuse_texture, frag_uv );
	fragment_color =  color + phong_shading(tangent_eye_position, vec4(normal, 0.0) );
}
