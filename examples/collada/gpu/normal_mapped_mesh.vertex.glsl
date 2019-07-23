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

uniform mat4 light_pads;

layout(location = 0) in vec3 vertex_coord;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;
layout(location = 3) in vec3 tangent;

out vec2 frag_uv;
out vec4 tangent_light_position;
out vec4 tangent_eye_position;

void main(void) {
	frag_uv = vertex_uv;
	mat3 mv3 = mat3(mv);
	vec3 eye_pos = (mv * vec4(vertex_coord, 1.0)).xyz;
	vec3 t = normalize(mv3 * tangent);
	vec3 n = normalize(mv3 * vertex_normal);
	t = normalize(t - dot(t, n) * n );
	mat3 TBN = transpose( mat3(t,cross(n,t),n) );
	tangent_light_position = vec4( (TBN * light_pads[0].xyz), light_pads[0].w);
	tangent_eye_position = vec4(TBN * eye_pos, 1.0);
	gl_Position = mvp * vec4(vertex_coord, 1.0);
}
