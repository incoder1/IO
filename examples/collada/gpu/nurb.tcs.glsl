#version 420

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

layout( vertices=16 ) out;

uniform int tess_level;

void main()
{
	// vertex coord's without changes
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	
	gl_TessLevelOuter[0] = float(tess_level);
	gl_TessLevelOuter[1] = float(tess_level);
	gl_TessLevelOuter[2] = float(tess_level);
	gl_TessLevelOuter[3] = float(tess_level);
	gl_TessLevelInner[0] = float(tess_level);
	gl_TessLevelInner[1] = float(tess_level);
}