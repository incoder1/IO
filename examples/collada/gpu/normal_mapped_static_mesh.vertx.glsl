#version 140

#pragma optimize(on)

precision highp float;

invariant gl_Position;
uniform mat4 mvpMat;
uniform mat4 modelViewMat;

const vec3 light_position =  vec3(0.3,0.1,2.0);

in vec3 vertexCoord;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec3 aTangent;

out vec2 fragTexCoords;
out vec4 tangentLightPosition;
out vec4 tangentEyePosition;
out mat3 TBN;

void main(void) {
	mat3 mv = mat3(modelViewMat);
	vec3 eyePosition = mv * vertexCoord;
	fragTexCoords = vertexTexCoord;
	vec3 t = normalize(mv * aTangent);
	vec3 n = normalize(mv * vertexNormal);
	TBN = transpose( mat3(t,cross(n,t),n) );
	tangentEyePosition = vec4( (TBN * eyePosition), 0);
	tangentLightPosition = vec4( (TBN * light_position ), 0);
	gl_Position = mvpMat * vec4(vertexCoord,1.0);
}
