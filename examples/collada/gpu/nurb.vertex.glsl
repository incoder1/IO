#version 420

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

invariant gl_Position;

layout(location = 0) in vec2 vertex_coord;

void main(void) {
	gl_Position = vec4(vertex_coord, 0.0, 1.0);
}