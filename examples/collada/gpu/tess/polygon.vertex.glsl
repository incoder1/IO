#version 420 compatibility

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

invariant gl_Position;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

out vec3 vxt_normal;

void main() {
	vxt_normal = vertex_normal;
	gl_Position = vec4(vertex_position, 1.0);
}