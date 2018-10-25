#include "model.hpp"

namespace engine {


static const char* VERTEX_SHADER = "\
#version 140 \n \
#pragma optimize(on) \n \
precision highp float; \n \
invariant gl_Position; \
uniform mat4 mvpMat; \
uniform mat4 modelViewMat; \
uniform mat4 normalMat; \
uniform vec3 matAmb; \
uniform vec3 matDiff; \
uniform vec3 matSpec; \
uniform float matShininess; \
uniform float matDissolve; \
uniform vec3 lightPosition; \
in vec3 vertexCoord; \
in vec3 vertexNormal; \
out vec4 outFrontColor; \
out vec4 outBackColor; \
struct LightInfo { \
 vec4 position; \
 vec4 ambient; \
 vec4 diffuse; \
 vec4 specular; \
}; \
struct MaterialInfo { \
 vec4 ambient; \
 vec4 diffuse; \
 vec4 specular; \
 vec4 emission; \
 float shininess; \
}; \
LightInfo defaultLight() { \
	LightInfo result; \
	result.position = vec4(0,0,1,0); \
	result.ambient = vec4(0,0,0,1); \
	result.diffuse = vec4(1,1,1,1); \
	result.specular = vec4(1,1,1,1); \
	return result; \
} \
MaterialInfo defaultMaterial() { \
	MaterialInfo result; \
	result.ambient = vec4(0.2, 0.2, 0.2, 1); \
 	result.diffuse = vec4(0.8, 0.8, 0.8, 1); \
 	result.specular = vec4(0, 0, 0, 1); \
 	result.emission = vec4(0,0,0,1); \
 	result.shininess = 0.001; \
 	return result; \
} \
float dot(vec4 lsh, vec4 rhs) { \
	return (lsh.x*rhs.x) + (lsh.y*rhs.y) + (lsh.z*rhs.z) + (lsh.w*rhs.w); \
} \
void getEyeSpace( out vec4 norm, out vec4 position ) { \
	norm = normalize( normalMat * vec4(vertexNormal,0) ); \
	position = modelViewMat * vec4(vertexCoord.xyz,0); \
} \
vec4 phongModel(LightInfo light, MaterialInfo mat, vec4 position, vec4 norm ) { \
	vec4 s = normalize( light.position - position ); \
	vec4 v = normalize( -position ); \
	vec4 r = reflect( -s, norm ); \
	vec4 ambient = light.ambient * mat.ambient; \
	float sDotN = max( dot(s,norm), 0.0 ); \
	vec4 diffuse = light.diffuse * mat.diffuse * sDotN; \
	vec4 specular = vec4(0.0); \
	if( sDotN > 0.0 ) { \
		float shininess = mat.shininess; \
		if(0.0 == shininess) { \
			shininess = 4; \
		} \
		specular = light.specular * mat.specular * pow( max( dot(r,v), 0.0 ), shininess ); \
	} \
	return ambient + diffuse + specular; \
} \
void main(void) { \
	vec4 eyeNorm; \
	vec4 eyePosition; \
	getEyeSpace(eyeNorm, eyePosition); \
	LightInfo light = defaultLight(); \
	light.position = vec4( lightPosition, 0); \
	MaterialInfo mat = defaultMaterial(); \
	mat.ambient = vec4(matAmb,matDissolve); \
	mat.diffuse = vec4(matDiff,matDissolve); \
	mat.specular = vec4(matSpec,matDissolve); \
	mat.shininess = matShininess; \
	outFrontColor = phongModel(light, mat, eyePosition, eyeNorm ); \
	outBackColor = phongModel(light, mat, eyePosition, -eyeNorm ); \
	gl_Position = mvpMat * vec4(vertexCoord,1.0); \
}";


static const char* FRAGMENT_SHADER = "\
#version 140 \n \
#pragma optimize(on) \n \
precision highp float; \n \
in vec4 outFrontColor; \
in vec4 outBackColor; \
const vec4 GAMMA = vec4( 1.0 / 2.2 ); \
out vec4 outFragColor; \
void main(void) { \
	if( gl_FrontFacing ) { \
		outFragColor = pow(outFrontColor, GAMMA); \
	} else { \
		outFragColor = pow( outBackColor, GAMMA); \
	} \
}";


mesh_model::mesh_model():
	model(),
	program_()
{
	gl::shader vertex(gl::shader_type::vertex, VERTEX_SHADER );
	gl::shader fragment(gl::shader_type::fragment, FRAGMENT_SHADER );
	program_ = gl::program::create( std::move(vertex), std::move(fragment) );
	program_->link();
}

mesh_model::~mesh_model()
{
}

void mesh_model::draw(const scene& scn) const
{
}



} // namespace engine
