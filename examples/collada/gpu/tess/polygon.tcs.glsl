#version 420 compatibility

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

in vec3 normal_cs_in[];

out vec3 normal_tes_in[];
layout(vertices=4) out;

uniform int tess_level;

void main(void) {	
	normal_tes_in[gl_InvocationID] = normal_cs_in[gl_InvocationID];

	// vertex coord's without changes
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	gl_TessLevelOuter[0] = float(tess_level);
	gl_TessLevelOuter[1] = float(tess_level);
	gl_TessLevelOuter[2] = float(tess_level);
	gl_TessLevelOuter[3] = float(tess_level);
	
	gl_TessLevelInner[0] = float(tess_level);
	gl_TessLevelInner[1] = float(tess_level);
	
}

