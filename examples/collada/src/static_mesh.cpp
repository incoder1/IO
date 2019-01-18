#include "stdafx.hpp"
#include "static_mesh.hpp"
#include <glm/gtc/type_ptr.hpp>

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
varying vec4 frontColor;\n\
varying vec4 backColor;\n\
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
 	result.shininess = 0;\n\
 	return result;\n\
}\n\
float dot(vec4 lsh, vec4 rhs) {\n\
  return (lsh.x*rhs.x) + (lsh.y*rhs.y) + (lsh.z*rhs.z) + (lsh.w*rhs.w);\n\
}\n\
void getEyeSpace( out vec4 norm, out vec4 position ) {\n\
	norm = normalize( normalMat * vec4(vertexNormal,0) );\n\
	position = modelViewMat * vec4(vertexCoord.xyz,0);\n\
}\n\
vec4 phongShading(LightInfo light, MaterialInfo mat, vec4 position, vec4 norm ) {\n\
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
	return ambient + clamp(diffuse,0.0, 1.0) +  clamp(specular, 0.0, 1.0);\n\
}\n\
LightInfo light = defaultLight();\n\
MaterialInfo mat = defaultMaterial();\n\
void main(void) {\
	vec4 eyeNorm;\
	vec4 eyePosition;\
	getEyeSpace(eyeNorm, eyePosition);\
	frontColor = vec4(vertexColor,1) + phongShading(light, mat, eyePosition, eyeNorm );\
	backColor = vec4(vertexColor,1) + phongShading(light, mat, eyePosition, -eyeNorm );\
	gl_Position = mvpMat * vec4(vertexCoord,1.0);\
}";

const char* untextured_static_mesh::FRAGMENT_SHADER = "\
#version 140 \n\
#pragma optimize(on)\n\
precision highp float;\n\
varying vec4 frontColor;\n\
varying vec4 backColor;\n\
out vec4 outFragColor;\n\
void main(void) {\
	if( gl_FrontFacing ) {\
		outFragColor = frontColor;\
	} else {\
		outFragColor = backColor;\
	}\
}";

untextured_static_mesh::untextured_static_mesh(const float *vertex, std::size_t vsize,const uint32_t* index,std::size_t isize):
	surface(),
	program_(),
	vbo_(),
	ibo_(),
	isize_(isize),
	mvp_UL_(-1),
	modelVeiwMatUL_(-1),
	normalMatUL_(-1)
{
	vbo_ = gl::buffer::create( vertex, vsize,
							   gl::buffer_type::ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW
							 );
	ibo_ = gl::buffer::create( index, isize_,
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
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

	mvp_UL_ = program_->uniform_location("mvpMat");
	modelVeiwMatUL_ = program_->uniform_location("modelViewMat");
	normalMatUL_ = program_->uniform_location("normalMat");
}

void untextured_static_mesh::draw(const scene& scn) const
{
	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;
	scn.get_matrix(projection_mat,model_view_mat);
	::glm::mat4 normal_mat( glm::transpose( glm::inverse(  glm::mat3(model_view_mat) ) ) );

	program_->start();
	::glUniformMatrix4fv(mvp_UL_, 1, GL_FALSE, glm::value_ptr( projection_mat * model_view_mat ) );
	::glUniformMatrix4fv(modelVeiwMatUL_, 1, GL_FALSE, glm::value_ptr( model_view_mat ) );
	::glUniformMatrix4fv(normalMatUL_, 1, GL_FALSE, glm::value_ptr( normal_mat ) );

	ibo_->bind();
	::glDrawElements(GL_TRIANGLES, isize_, GL_UNSIGNED_INT, 0);
	ibo_->unbind();

	program_->stop();
}

//textured_static_mesh

const char* textured_static_mesh::VERTEX_SHADER = "gpu/textured_static_mesh.vertx.glsl";

const char* textured_static_mesh::FRAGMENT_SHADER = "gpu/textured_static_mesh.frag.glsl";

textured_static_mesh::textured_static_mesh(const material_t& mat,const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& timg):
	surface(),
	program_(),
	vbo_(),
	ibo_(),
	isize_(isize),
	texture_(),
	mat_helper_(mat),
	light_helper_(),
	mvp_ul_(-1),
	mv_ul_(-1),
	nrm_ul_(-1),
	diffise_tex_ul_(-1)
{
	gl::shader vertex_sh = gl::shader::load_glsl(gl::shader_type::vertex,io::file(VERTEX_SHADER));
	gl::shader fragment_sh =  gl::shader::load_glsl(gl::shader_type::fragment, io::file(FRAGMENT_SHADER) );
	program_ = gl::program::create( std::move(vertex_sh), std::move(fragment_sh) );

	vbo_ = gl::buffer::create( vertex, vsize,
							   gl::buffer_type::ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW
							 );
	ibo_ = gl::buffer::create( indexes, isize_,
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW );

	program_->bind_attrib_location(0, VATTR_CRD);
	program_->bind_attrib_location(1, VATTR_NRM);
	program_->bind_attrib_location(2, VATTR_UV);

	program_->pass_vertex_attrib_array(0, vbo_, false, 8, 3 , 0);
	program_->pass_vertex_attrib_array(1, vbo_, false, 8, 3 , 3);
	program_->pass_vertex_attrib_array(2, vbo_, false, 8, 2 , 6);

	texture_ = gl::texture::create_texture2d_from_image(
				   timg,
				   gl::texture_filter::LINEAR_MIPMAP_LINEAR);

	program_->link();

	mvp_ul_ = program_->uniform_location(UNFM_MVP_MAT);
	mv_ul_ = program_->uniform_location(UNFM_MV_MAT);
	nrm_ul_ = program_->uniform_location(UNFM_NORMAL_MAT);

	mat_helper_.bind_to_shader(program_);
	light_helper_.bind_to_shader(program_);

	diffise_tex_ul_ = program_->uniform_location(UNFM_DIFFUSE_TEXTURE);
}

textured_static_mesh::textured_static_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& timg):
	textured_static_mesh(DEFAULT_MATERIAL, vertex, vsize,indexes, isize, timg)
{}

void textured_static_mesh::draw(const scene& scn) const
{
	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;
	scn.get_matrix(projection_mat,model_view_mat);
	::glm::mat4 normal_mat( glm::transpose( glm::inverse(  glm::mat3(model_view_mat) ) ) );

	program_->start();

	// transfer world
	::glUniformMatrix4fv(mvp_ul_, 1, GL_FALSE, glm::value_ptr( projection_mat * model_view_mat ) );
	::glUniformMatrix4fv(mv_ul_, 1, GL_FALSE, glm::value_ptr( model_view_mat ) );
	::glUniformMatrix4fv(nrm_ul_, 1, GL_FALSE, glm::value_ptr( normal_mat ) );

	// transfer light
	light_helper_.transfer_to_shader( scn.light() );
	// transfer material
	mat_helper_.transfer_to_shader();

	// transfer texture
	::glActiveTexture(GL_TEXTURE0);
	::glUniform1i(diffise_tex_ul_, 0);
	 texture_->bind();

	ibo_->bind();
	::glDrawElements(GL_TRIANGLES, isize_, GL_UNSIGNED_INT, 0);

	ibo_->unbind();
	texture_->unbind();

	program_->stop();
}

//normal_mapped_static_mesh
const char* normal_mapped_static_mesh::VERTEX_SHADER = "gpu/normal_mapped_static_mesh.vertx.glsl";

const char* normal_mapped_static_mesh::FRAGMENT_SHADER = "gpu/normal_mapped_static_mesh.frag.glsl";

normal_mapped_static_mesh::normal_mapped_static_mesh(const material_t& mat,const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& difftex,const s_image& nm_text):
	surface(),
	program_(),
	vbo_(),
	ibo_(),
	isize_(isize),
	diffuse_tex_(),
	normal_map_tex_(),
	mat_helper_(mat),
	light_helper_(),
	mvp_ul_(-1),
	mv_ul_(-1),
	diffise_tex_ul_(-1),
	nm_tex_ul_(-1)
{
	gl::shader vertex_sh = gl::shader::load_glsl(gl::shader_type::vertex,io::file(VERTEX_SHADER));
	gl::shader fragment_sh =  gl::shader::load_glsl(gl::shader_type::fragment, io::file(FRAGMENT_SHADER) );
	program_ = gl::program::create( std::move(vertex_sh), std::move(fragment_sh) );

	vbo_ = gl::buffer::create( vertex, vsize,
							   gl::buffer_type::ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW
							 );
	ibo_ = gl::buffer::create( indexes, isize_,
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW );

	program_->bind_attrib_location(0, VATTR_CRD);
	program_->bind_attrib_location(1, VATTR_NRM);
	program_->bind_attrib_location(2, VATTR_UV);
	program_->bind_attrib_location(3, VATTR_TAN);

	program_->pass_vertex_attrib_array(0, vbo_, false, 11, 3 , 0);
	program_->pass_vertex_attrib_array(1, vbo_, false, 11, 3 , 3);
	program_->pass_vertex_attrib_array(2, vbo_, false, 11, 2 , 6);
	program_->pass_vertex_attrib_array(3, vbo_, false, 11, 3 , 8);


	program_->link();

	mvp_ul_ = program_->uniform_location(UNFM_MVP_MAT);
	mv_ul_ = program_->uniform_location(UNFM_MV_MAT);

	mat_helper_.bind_to_shader(program_);
	light_helper_.bind_to_shader(program_);

	diffise_tex_ul_ = program_->uniform_location(UNFM_DIFFUSE_TEXTURE);
	nm_tex_ul_ = program_->uniform_location(UNFM_NORMALMAP_TEXTURE);

	diffuse_tex_ = gl::texture::create_texture2d_from_image(difftex, gl::texture_filter::LINEAR_MIPMAP_LINEAR);
	normal_map_tex_ = gl::texture::create_texture2d_from_image(nm_text,gl::texture_filter::NEAREST);
}

normal_mapped_static_mesh::normal_mapped_static_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& difftex,const s_image& nm_text):
	normal_mapped_static_mesh(DEFAULT_MATERIAL, vertex, vsize, indexes, isize, difftex, nm_text)
{}

void normal_mapped_static_mesh::draw(const scene& scn) const
{

	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;
	scn.get_matrix(projection_mat,model_view_mat);

	program_->start();

	::glUniformMatrix4fv(mvp_ul_, 1, GL_FALSE, glm::value_ptr( projection_mat * model_view_mat ) );
	::glUniformMatrix4fv(mv_ul_, 1, GL_FALSE, glm::value_ptr( model_view_mat ) );

	// transfer light
	light_helper_.transfer_to_shader( scn.light() );

	// transfer material
	mat_helper_.transfer_to_shader();

	// transfer textures
	::glActiveTexture(GL_TEXTURE0);
	::glUniform1i(diffise_tex_ul_, 0);
	diffuse_tex_->bind();

	::glActiveTexture(GL_TEXTURE1);
	::glUniform1i(nm_tex_ul_, 1);
	normal_map_tex_->bind();

	ibo_->bind();
	::glDrawElements(GL_TRIANGLES, isize_, GL_UNSIGNED_INT, 0);
	ibo_->unbind();

	diffuse_tex_->unbind();
	normal_map_tex_->unbind();

	program_->stop();
}

} // namespace engine
