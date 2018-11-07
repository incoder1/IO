#include "stdafx.hpp"
#include "model.hpp"

namespace engine {

// untextured_static_mesh
const char* untextured_static_mesh::VERTEX_SHADER = "\
#version 140\n\
#pragma optimize(on)\n\
precision highp float;\n\
invariant gl_Position;\n\
uniform mat4 mvpMat;\n\
uniform mat4 modelViewMat;\n\
uniform mat4 normalMat;\n\
in vec3 vertexCoord;\n\
in vec3 vertexNormal;\n\
in vec3 vertexColor;\n\
out vec4 outFrontColor;\n\
out vec4 outBackColor;\n\
struct LightInfo {\n\
 vec4 position;\n\
 vec4 ambient;\n\
 vec4 diffuse;\n\
 vec4 specular;\n\
};\n\
struct MaterialInfo {\n\
 vec4 ambient;\n\
 vec4 diffuse;\n\
 vec4 specular;\n\
 vec4 emission;\n\
 float shininess;\n\
};\n\
LightInfo defaultLight() {\n\
	LightInfo result;\n\
	result.position = vec4(1,1,2,0);\n\
	result.ambient = vec4(0,0,0,1);\n\
	result.diffuse = vec4(1,1,1,1);\n\
	result.specular = vec4(1,1,1,1);\n\
	return result;\n\
}\n\
MaterialInfo defaultMaterial() {\n\
	MaterialInfo result;\n\
	result.ambient = vec4(0.2, 0.2, 0.2, 1);\n\
 	result.diffuse = vec4(0.8, 0.8, 0.8, 1);\n\
 	result.specular = vec4(0, 0, 0, 1);\n\
 	result.emission = vec4(0,0,0,1);\n\
 	result.shininess = 0.001;\n\
 	return result;\n\
}\n\
float dot(vec4 lsh, vec4 rhs) {\n\
  return (lsh.x*rhs.x) + (lsh.y*rhs.y) + (lsh.z*rhs.z) + (lsh.w*rhs.w);\n\
}\n\
void getEyeSpace( out vec4 norm, out vec4 position ) {\n\
	norm = normalize( normalMat * vec4(vertexNormal,0) );\n\
	position = modelViewMat * vec4(vertexCoord.xyz,0);\n\
}\n\
vec4 phongModel(LightInfo light, MaterialInfo mat, vec4 position, vec4 norm ) {\n\
	vec4 s = normalize( light.position - position );\n\
	vec4 v = normalize( -position );\n\
	vec4 r = reflect( -s, norm );\n\
	vec4 ambient = light.ambient * mat.ambient;\n\
	float sDotN = max( dot(s,norm), 0.0 );\n\
	vec4 diffuse = light.diffuse * mat.diffuse * sDotN;\n\
	vec4 specular = vec4(0.0);\n\
	if( sDotN > 0.0 ) {\n\
		specular = light.specular * mat.specular * pow( max( dot(r,v), 0.0 ), mat.shininess );\n\
	}\n\
	return ambient + diffuse + specular;\n\
}\n\
LightInfo light = defaultLight();\n\
MaterialInfo mat = defaultMaterial();\n\
void main(void) {\
	vec4 eyeNorm;\
	vec4 eyePosition;\
	getEyeSpace(eyeNorm, eyePosition);\
	outFrontColor = vec4(vertexColor,1) + phongModel(light, mat, eyePosition, eyeNorm );\
	outBackColor = vec4(vertexColor,1) + phongModel(light, mat, eyePosition, -eyeNorm );\
	gl_Position = mvpMat * vec4(vertexCoord,1.0);\
}";

const char* untextured_static_mesh::FRAGMENT_SHADER = "\
#version 140 \n\
#pragma optimize(on)\n\
precision highp float;\n\
in vec4 outFrontColor;\n\
in vec4 outBackColor;\n\
out vec4 outFragColor;\n\
void main(void) {\
	if( gl_FrontFacing ) {\
		outFragColor = outFrontColor;\
	} else {\
		outFragColor = outBackColor;\
	}\
}";

untextured_static_mesh::untextured_static_mesh(const float *vertex, std::size_t vsize,const uint32_t* index,std::size_t isize):
	model(),
	program_(),
	vbo_(),
	ibo_(),
	isize_(isize),
	mvpUL_(-1),
	modelVeiwMatUL_(-1),
	normalMatUL_(-1)
{
	vbo_ = gl::buffer::create( vertex, vsize,
							   gl::buffer_type::ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW
							 );
	ibo_ = gl::buffer::create( index, isize_*sizeof(uint32_t),
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
							   gl::data_type::UNSIGNED_INT,
							   gl::buffer_usage::STATIC_DRAW );

	gl::shader vertex_sh(gl::shader_type::vertex, VERTEX_SHADER );
	gl::shader fragment_sh(gl::shader_type::fragment, FRAGMENT_SHADER );
	program_ = gl::program::create( std::move(vertex_sh), std::move(fragment_sh) );

	program_->bind_attrib_location(0, "vertexCoord");
	program_->bind_attrib_location(1, "vertexColor");
	program_->bind_attrib_location(2, "vertexNormal");

	program_->pass_vertex_attrib_array(0, vbo_, false, 9, 3, 0);
	program_->pass_vertex_attrib_array(1, vbo_, false, 9, 3, 3);
	program_->pass_vertex_attrib_array(2, vbo_, false, 9, 3, 6);

	program_->link();

	mvpUL_ = program_->uniform_location("mvpMat");
	modelVeiwMatUL_ = program_->uniform_location("modelViewMat");
	normalMatUL_ = program_->uniform_location("normalMat");
}

void untextured_static_mesh::draw(const scene& scn) const
{
	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;
	::glm::mat4 normal_mat;
	scn.get_matrix(projection_mat,model_view_mat,normal_mat);

	program_->start();
	::glUniformMatrix4fv(mvpUL_, 1, GL_FALSE, glm::value_ptr( projection_mat * model_view_mat ) );
	::glUniformMatrix4fv(modelVeiwMatUL_, 1, GL_FALSE, glm::value_ptr( model_view_mat ) );
	::glUniformMatrix4fv(normalMatUL_, 1, GL_FALSE, glm::value_ptr( normal_mat ) );

	ibo_->bind();
	::glDrawElements(GL_TRIANGLES, isize_, GL_UNSIGNED_INT, 0);
	ibo_->unbind();

	program_->stop();
}

//textured_static_mesh

const char* textured_static_mesh::VERTEX_SHADER = "\
#version 140 \n\
#pragma optimize(on)\n\
precision highp float;\
invariant gl_Position;\
uniform mat4 mvpMat;\
uniform mat4 modelViewMat;\
uniform mat4 normalMat;\
in vec3 vertexCoord;\
in vec3 vertexNormal;\
in vec2 vertexTexCoord;\
out vec4 outPosition;\
out vec4 outNormal;\
out vec2 outTexCoord;\
void main(void) {\
	outPosition = modelViewMat * vec4(vertexCoord,1);\
	outNormal =  normalize( normalMat * vec4(vertexNormal, 0.0) );\
	outTexCoord = vertexTexCoord;\
	gl_Position = mvpMat * vec4(vertexCoord,1);\
}";

const char* textured_static_mesh::FRAGMENT_SHADER = "\
#version 140 \n\
#pragma optimize(on) \n\
precision highp float;\
in vec4 outPosition;\
in vec4 outNormal;\
in vec2 outTexCoord;\
uniform sampler2D textureSampler;\
out vec4 outFragColor;\
struct LightInfo {\
	vec4 position;\
	vec4 ambient;\
	vec4 diffuse;\
	vec4 specular;\
};\
struct MaterialInfo {\
	vec4 ambient;\
	vec4 diffuse;\
	vec4 specular;\
	vec4 emission;\
	float shininess;\
};\
LightInfo defaultLight() {\
	LightInfo result;\
	result.position = vec4(0,0.5,3,0);\
	result.ambient = vec4(0,0,0,1);\
	result.diffuse = vec4(0.5,0.5,0.5,1);\
	result.specular = vec4(0.3,0.3,0.3,1);\
	return result;\
}\
MaterialInfo defaultMaterial() {\
	MaterialInfo result;\
	result.ambient = vec4(0.2, 0.2, 0.2, 1);\
	result.diffuse = vec4(0.8, 0.8, 0.8, 1);\
	result.specular = vec4(0, 0, 0, 1);\
	result.emission = vec4(0,0,0,1);\
	result.shininess = 0;\
	return result;\
}\
float dot(vec4 lsh, vec4 rhs) {\
	return (lsh.x*rhs.x) + (lsh.y*rhs.y) + (lsh.z*rhs.z) + (lsh.w*rhs.w);\
}\
vec4 phongModel(LightInfo light, MaterialInfo mat, vec4 position, vec4 norm ) {\
	vec4 s = normalize( light.position - position );\
	vec4 v = normalize( -position );\
	vec4 r = reflect( -s, norm );\
	vec4 ambient = light.ambient * mat.ambient;\
	float sDotN = max( dot(s,norm), 0.0 );\
	vec4  diffuse = light.diffuse * mat.diffuse * sDotN;\
	vec4 specular = vec4(0.0);\
	if( sDotN > 0.0 ) {\
		specular = light.specular * mat.specular * pow( max( dot(r,v), 0.0 ), mat.shininess );\
	}\
	return ambient + clamp(diffuse,0.0, 1.0) +  clamp(specular, 0.0, 1.0);\
}\
const float GAMMA = 1.0 / 2.2;\
LightInfo light = defaultLight();\
MaterialInfo mat = defaultMaterial();\
void main(void) {\
	outFragColor =  pow(texture( textureSampler, outTexCoord ),vec4(GAMMA)) + phongModel(light, mat, outPosition, outNormal );\
}";

textured_static_mesh::textured_static_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& timg):
	model(),
	program_(),
	vbo_(),
	ibo_(),
	isize_(isize),
	texture_(),
	mvpUL_(-1),
	modelVeiwMatUL_(-1),
	normalMatUL_(-1),
	textureUL_(-1)
{
	vbo_ = gl::buffer::create( vertex, vsize,
							   gl::buffer_type::ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW
							 );
	ibo_ = gl::buffer::create( indexes, isize_*sizeof(uint32_t),
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
							   gl::data_type::UNSIGNED_INT,
							   gl::buffer_usage::STATIC_DRAW );

	texture_ = gl::texture::create_rgba_texture_2d(
													timg->width(), timg->height(),
													gl::texture_filter::NEAREST_MIPMAP_LINEAR,
													timg->data() );

	gl::shader vertex_sh(gl::shader_type::vertex, VERTEX_SHADER );
	gl::shader fragment_sh(gl::shader_type::fragment, FRAGMENT_SHADER );
	program_ = gl::program::create( std::move(vertex_sh), std::move(fragment_sh) );

	program_->bind_attrib_location(0, "vertexCoord");
	program_->bind_attrib_location(1, "vertexNormal");
	program_->bind_attrib_location(2, "vertexTexCoord");

	program_->pass_vertex_attrib_array(0, vbo_, false, 8, 3, 0);
	program_->pass_vertex_attrib_array(1, vbo_, false, 8, 3, 3);
	program_->pass_vertex_attrib_array(2, vbo_, false, 8, 2, 6);

	program_->link();

	mvpUL_ = program_->uniform_location("mvpMat");
	modelVeiwMatUL_ = program_->uniform_location("modelViewMat");
	normalMatUL_ = program_->uniform_location("normalMat");
	textureUL_ = program_->uniform_location("textureSampler");
}

void textured_static_mesh::draw(const scene& scn) const
{
	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;
    ::glm::mat4 normal_mat;
	scn.get_matrix(projection_mat,model_view_mat,normal_mat);


	program_->start();
	::glUniformMatrix4fv(mvpUL_, 1, GL_FALSE, glm::value_ptr( projection_mat * model_view_mat ) );
	::glUniformMatrix4fv(modelVeiwMatUL_, 1, GL_FALSE, glm::value_ptr( model_view_mat ) );
	::glUniformMatrix4fv(normalMatUL_, 1, GL_FALSE, glm::value_ptr( normal_mat ) );

	texture_->bind();
	glUniform1i(textureUL_, 0);

	ibo_->bind();
	::glDrawElements(GL_TRIANGLES, isize_, GL_UNSIGNED_INT, 0);

	ibo_->unbind();
	texture_->unbind();

	program_->stop();
}

} // namespace engine
