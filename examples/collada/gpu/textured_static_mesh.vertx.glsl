#version 140

#pragma optimize(on)
precision highp float;

invariant gl_Position;

uniform mat4 mvpMat;
uniform mat4 modelViewMat;
uniform mat4 normalMat;

in vec3 vertexCoord;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

out vec4 eyePosition;
out vec4 outNormal;
out vec2 outTexCoord;

void main(void) {
	eyePosition = modelViewMat * vec4(vertexCoord,0.0);
	outNormal =  normalize( normalMat * vec4(vertexNormal, 0.0) );
	outTexCoord = vertexTexCoord;
	gl_Position = mvpMat * vec4(vertexCoord,1.0);
}