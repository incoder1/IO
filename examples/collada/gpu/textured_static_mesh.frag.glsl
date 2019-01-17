#version 140 
#pragma optimize(on) 
precision highp float;
in vec4 eyePosition;
in vec4 outNormal;
in vec2 outTexCoord;
uniform sampler2D textureSampler;
invariant out vec4 fragColor;
struct LightInfo {
	vec4 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};
struct MaterialInfo {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 emission;
	float shininess;
};
LightInfo defaultLight() {
	LightInfo result;
	result.position = vec4(1,1,1,0);
	result.ambient = vec4(0,0,0,1);
	result.diffuse = vec4(0.5,0.5,0.5,1);
	result.specular = vec4(0.7,0.7,0.7,1);
	return result;
}
MaterialInfo whitePlastic() {
	MaterialInfo result;
	result.ambient = vec4(0.0f,0.0f,0.0f,1.0f);
	result.diffuse = vec4(0.55f,0.55f,0.55f,1.0f);
	result.specular = vec4(0.70f,0.70f,0.70f,1.0f);
	result.emission = vec4(0,0,0,1);
	result.shininess = 32.0f;
	return result;
}
float dot(vec4 lsh, vec4 rhs) {
	return (lsh.x*rhs.x) + (lsh.y*rhs.y) + (lsh.z*rhs.z) + (lsh.w*rhs.w);
}
vec4 phongShading(LightInfo light, MaterialInfo mat, vec4 position, vec4 norm ) {
	vec4 s = normalize( light.position - position );
	vec4 v = normalize( -position );
	vec4 r = reflect( -s, norm );
	vec4 ambient = light.ambient * mat.ambient;
	float sDotN = max( dot(s,norm), 0.0 );
	vec4  diffuse = light.diffuse * mat.diffuse * sDotN;
	vec4 specular = vec4(0.0);
	if( sDotN > 0.0 ) {
		specular = light.specular * mat.specular * pow( max( dot(r,v), 0.0 ), mat.shininess );
	}
	return ambient + clamp(diffuse,0.0, 1.0) +  clamp(specular, 0.0, 1.0);
}
const float GAMMA = 1.0 / 2.2;
LightInfo light = defaultLight();
MaterialInfo mat = whitePlastic();
void main(void) {
	fragColor =  pow(texture( textureSampler, outTexCoord ),vec4(GAMMA)) + phongShading(light, mat, eyePosition, outNormal );
}