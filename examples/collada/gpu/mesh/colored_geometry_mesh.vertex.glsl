#version 330 compatibility

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

invariant gl_Position;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 nm;

uniform mat4 light_pads;
uniform mat4 material_adse;
uniform	float material_shininess;


layout(location = 0) in vec3 vertex_coord;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec3 vertex_normal;

out vec4 front_color;
out vec4 back_color;

vec4 phong_shading(vec4 eye_pos, vec4 norm) {
	vec4 s;
	if(0.0 == light_pads[0].w)
		s = normalize( light_pads[0] );
	else
		s = normalize( light_pads[0] - eye_pos );
	vec4 v = normalize( -eye_pos );
	vec4 r = reflect( -s, norm );
	vec4 ambient = light_pads[1] * material_adse[0];
	float cos_theta = max( dot(s,norm), 0.0 );
	vec4  diffuse = light_pads[2] * material_adse[1] * cos_theta;
	vec4 specular;
	if( cos_theta > 0.0 ) {
		float shininess = pow( max( dot(r,v), 0.0 ), material_shininess);
		vec4 specular = (light_pads[3] * material_adse[2]) * shininess;
		return ambient + clamp(diffuse,0.0, 1.0) + clamp(specular, 0.0, 1.0);
	}
	return ambient + clamp(diffuse,0.0, 1.0);
}

void main(void) {
	vec4 vcoord = vec4( vertex_coord, 1.0 );
	vec4 eye_norm = normalize( nm * vec4(vertex_normal,0.0) );
	vec4 eye_pos = mv * vcoord;
	front_color = vec4(vertex_color,1.0) + phong_shading(eye_pos, eye_norm);
	back_color = vec4(vertex_color,1.0) + phong_shading(eye_pos, -eye_norm);
	gl_Position = mvp * vcoord;
}
