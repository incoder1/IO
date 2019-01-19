#version 330

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

in vec4 front_color;
in vec4 back_color;

invariant out vec4 frag_color;

void main(void) {
	if( gl_FrontFacing ) {
		frag_color = front_color;
	} else {
		frag_color = back_color;
	}
}
