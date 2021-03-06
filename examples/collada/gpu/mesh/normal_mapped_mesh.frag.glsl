#version 330 compatibility

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
uniform sampler2D bumpmap_texture;

in vec2 frag_uv;
in vec4 tangent_light_position;
in vec4 tangent_eye_position;

out vec4 fragment_color;
invariant fragment_color;

vec4 phong_shading(vec4 norm) {
	vec4 s;
	if(0.0 == light_pads[0].w)
		s = normalize( tangent_light_position );
	else
		s = normalize( tangent_light_position - tangent_eye_position );
	vec4 ambient = light_pads[1] * material_adse[0];
	float cos_theta = clamp(dot(norm,s), 0.0, 1.0);
	vec4 diffuse = light_pads[2] * material_adse[1] * cos_theta;
	if( cos_theta > 0.0 ) {
		vec4 v = normalize( - tangent_eye_position );
		vec4 r = reflect( -s, norm );
		float shininess = pow( max( dot(r.xyz,v.xyz), 0.0 ), material_shininess);
		vec4 specular = (light_pads[3] * material_adse[2]) * shininess;
		return ambient + clamp(diffuse,0.0, 1.0) + clamp(specular,0.0, 1.0);
	}
	return ambient + clamp(diffuse,0.0, 1.0);
}

void main(void) {
	vec3 n = normalize( texture(bumpmap_texture, frag_uv ).rgb );
	n = normalize( (n * 2.0) - 1.0 );
	vec4 diffuse_color = texture( diffuse_texture, frag_uv );
	vec4 normal;
	if( gl_FrontFacing )
		normal = vec4(n,0.0);
	else
		normal = vec4(-n,0.0);
	fragment_color = diffuse_color + phong_shading(normal);
}
