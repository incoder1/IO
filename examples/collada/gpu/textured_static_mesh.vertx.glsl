#version 140

#pragma optimize(on)
precision highp float;

invariant gl_Position;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 nm;

in vec3 vertex_coord;
in vec3 vertex_normal;
in vec2 vertex_uv;

out vec4 eye_position;
out vec3 frag_normal;
out vec2 frag_uv;

void main(void) {
	eye_position = mv * vec4(vertex_coord,0.0);
	frag_normal =  normalize( mat3(nm) * vertex_normal );
	frag_uv = vertex_uv;
	gl_Position = mvp *  vec4(vertex_coord, 1.0);
}
