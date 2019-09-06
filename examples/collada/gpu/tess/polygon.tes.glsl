#version 420 compatibility

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

invariant gl_Position;


uniform mat4 mvp;
uniform mat4 nm;

out vec3 normal_tes_in[];

// Vertex position in camera coords
out vec4 tess_position;
// Vertex normal in camera coords.
out vec4 tess_normal;

layout(quads) in;


// Linear interpolation
vec4 linear(in vec4 p0, in vec4 p1, in vec4 p2, in vec4 p3,in float u,in float v) {
	float iu = 1.0 - u;
	float iv = 1.0 - v;
	return
		(p0 * (iu * iv) ) +
		(p1 * (u * iv) ) +
		(p3 * (v * iu) ) +
		(p2 * (u * v) );
}

void main(void) {
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	vec4 p2 = gl_in[2].gl_Position;
	vec4 p3 = gl_in[3].gl_Position;
	
	// rectangle have same normal vector
	tess_normal = normalize( nm * vec4(normal_tes_in[0],0.0) );
	
	gl_Position = mvp * linear(p0, p1, p2, p3, gl_TessCoord.x, gl_TessCoord.y);
}

