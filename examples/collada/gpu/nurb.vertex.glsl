#version 420 compatibility

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

invariant gl_Position;

layout(location = 0) in vec3 vertex_coord;

void main(void) {
	gl_Position = vec4(vertex_coord, 1.0);
}
