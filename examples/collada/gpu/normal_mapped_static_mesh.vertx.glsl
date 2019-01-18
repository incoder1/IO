#version 140

#pragma optimize(on)

precision highp float;

invariant gl_Position;

uniform mat4 mvp;
uniform mat4 mv;

uniform mat4 light_pads;

in vec3 vertex_coord;
in vec3 vertex_normal;
in vec2 vertex_uv;
in vec3 tangent;

out mat3 TBN;
out vec2 frag_uv;
out vec4 tangent_light_position;
out vec4 tangent_eye_position;

void main(void) {
	mat3 mv3 = mat3(mv);
	vec3 eye_pos = mv3 * vertex_coord;
	frag_uv = vertex_uv;
	vec3 t = normalize(mv3 * tangent);
	vec3 n = normalize(mv3 * vertex_normal);
	TBN = transpose( mat3(t,cross(n,t),n) );
	tangent_light_position = vec4( (TBN * vec3(light_pads[0].xyz) ), 0.0);
	tangent_eye_position = vec4( (TBN * eye_pos), 0.0);
	gl_Position = mvp * vec4(vertex_coord, 1.0);
}
