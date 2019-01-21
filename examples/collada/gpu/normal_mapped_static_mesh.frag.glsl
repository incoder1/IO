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

in vec2 frag_uv;
in vec4 tangent_light_position;
in vec4 tangent_eye_position;

out vec4 fragment_color;
invariant fragment_color;

vec4 phong_shading(vec4 norm) {
	vec4 s = normalize( tangent_light_position - tangent_eye_position );
	vec4 ambient = light_pads[1] * material_adse[0];
	float s_dot_nrm = max( dot(s,norm), 0.0 );
	vec4  diffuse = s_dot_nrm * light_pads[2] * material_adse[1];
	vec4 specular;
	if( s_dot_nrm > 0.0 ) {
		vec4 r = reflect( -s, norm );
		vec4 v = normalize(-tangent_eye_position);
		float shininess = pow( max( dot(r,v), 0.0 ), material_shininess);
		vec4 specular = shininess * light_pads[3] * material_adse[2];
		return ambient + clamp(diffuse,0.0, 1.0) + clamp(specular, 0.0, 1.0);
	}
	return ambient + clamp(diffuse,0.0, 1.0);
}

void main(void) {
	vec3 n = normalize( texture(normal_map_texture, frag_uv ).rgb );
	n = normalize( (n * 2.0) - 1.0 );
	vec4 color = texture( diffuse_texture, frag_uv );
	vec4 normal;
	if( gl_FrontFacing ) {
		normal = vec4(n,0.0);
	} else {
		normal = -vec4(n,0.0);
	}
	fragment_color = color + phong_shading(normal);
}
