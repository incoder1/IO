#version 140

#pragma optimize(on)

precision highp float;

in vec2 fragTexCoords;
in vec4 tangentEyePosition;
in vec4 tangentLightPosition;
in mat3 TBN;

uniform sampler2D diffuseTexture;
uniform sampler2D normalMapTexture;

out vec4 fragColor;

invariant fragColor;
struct LightInfo {
	vec4 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

LightInfo defaultLight() {
	LightInfo result;
	result.position = vec4(0.3,0.1,1.5,0);
	result.ambient = vec4(0,0,0,1);
	result.diffuse = vec4(0.7,0.7,0.7,1);
	result.specular = vec4(0.7,0.7,0.7,1);
	return result;
}

struct MaterialInfo {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 emission;
	float shininess;
};

MaterialInfo whitePlastic() {
	MaterialInfo result;
	result.ambient =  vec4(0.0,  0.0, 0.0, 1.0);
	result.diffuse =  vec4(0.55, 0.55, 0.55, 1.0);
	result.specular = vec4(0.7,	0.7, 0.7, 1.0);
	result.emission = vec4(0,0,0,1);
	result.shininess = 0.25;
	return result;
}

MaterialInfo defaultMaterial() {
	MaterialInfo result;
	result.ambient = vec4(0.3, 0.3, 0.3, 1);
	result.diffuse = vec4(0.3, 0.3, 0.3, 1);
	result.specular = vec4(0, 0, 0, 1);
	result.emission = vec4(0,0,0,1);
	result.shininess = 0.25;
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

LightInfo light = defaultLight();
MaterialInfo mat = whitePlastic();

void main(void) {
	light.position = tangentLightPosition;
	vec3 normal = normalize( texture(normalMapTexture, fragTexCoords ).rgb );
	normal = normalize( (normal * 2.0) - 1.0 );
	normal = normalize(TBN * normal);
	vec4 color = texture( diffuseTexture, fragTexCoords );
	fragColor =  color + phongShading(light, mat, tangentEyePosition, vec4(normal, 0) );
}