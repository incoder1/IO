#include "stdafx.hpp"
#include "mesh.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace engine {

// geometry_mesh
const char* geometry_mesh::VERTEX_SHADER = "gpu/geometry_mesh.vertex.glsl";

const char* geometry_mesh::FRAGMENT_SHADER = "gpu/geometry_mesh.frag.glsl";

geometry_mesh::geometry_mesh(const material_t& mat,const float *vertex, std::size_t vsize,const uint32_t* index,std::size_t isize):
	mesh(),
	program_(),
	vbo_(),
	ibo_(),
	mat_helper_(mat),
	light_helper_(),
	mvp_ul_(-1),
	mv_ul_(-1),
	nrm_ul_(-1)
{

	gl::shader vertex_sh = gl::shader::load_glsl(gl::shader_type::vertex,io::file(VERTEX_SHADER));
	gl::shader fragment_sh =  gl::shader::load_glsl(gl::shader_type::fragment, io::file(FRAGMENT_SHADER) );

	program_ = gl::program::create( std::move(vertex_sh), std::move(fragment_sh) );

	vbo_ = gl::buffer::create( vertex, vsize,
							   gl::buffer_type::ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW
							 );
	ibo_ = gl::buffer::create( index, isize,
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW );

	gl::shader_program_attribute layout[3] = {
		{VATTR_CRD,3},
		{VATTR_CRL,3},
		{VATTR_NRM,3} };

	program_->pass_vertex_attrib_array(vbo_, false, layout, 3);

	program_->link();

	mvp_ul_ = program_->uniform_location(UNFM_MVP_MAT);
	mv_ul_ = program_->uniform_location(UNFM_MV_MAT);
	nrm_ul_ = program_->uniform_location(UNFM_NORMAL_MAT);

	mat_helper_.bind_to_shader(program_);
	light_helper_.bind_to_shader(program_);

}

geometry_mesh::geometry_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize):
	geometry_mesh(DEFAULT_MATERIAL, vertex, vsize,indexes, isize)
{}

void geometry_mesh::draw(const scene& scn) const
{
	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;
	scn.get_frustum(projection_mat,model_view_mat);
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

	ibo_->bind();
	::glDrawElements(GL_TRIANGLES, ibo_->size() , GL_UNSIGNED_INT, 0);
	ibo_->unbind();

	program_->stop();
}

//textured_mesh

const char* textured_mesh::VERTEX_SHADER = "gpu/textured_mesh.vertex.glsl";

const char* textured_mesh::FRAGMENT_SHADER = "gpu/textured_mesh.frag.glsl";

textured_mesh::textured_mesh(const material_t& mat,const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& timg):
	mesh(),
	program_(),
	vbo_(),
	ibo_(),
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
	ibo_ = gl::buffer::create( indexes, isize,
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW );


	gl::shader_program_attribute layout[3] = {
		{VATTR_CRD,3},
		{VATTR_NRM,3},
		{VATTR_UV,2} };

	program_->pass_vertex_attrib_array(vbo_, false, layout, 3);

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

textured_mesh::textured_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& timg):
	textured_mesh(DEFAULT_MATERIAL, vertex, vsize,indexes, isize, timg)
{}

void textured_mesh::draw(const scene& scn) const
{
	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;
	scn.get_frustum(projection_mat,model_view_mat);
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
	::glDrawElements(GL_TRIANGLES, ibo_->size(), GL_UNSIGNED_INT, 0);

	ibo_->unbind();
	texture_->unbind();

	program_->stop();
}

//normal_mapped_mesh
const char* normal_mapped_mesh::VERTEX_SHADER = "gpu/normal_mapped_mesh.vertex.glsl";

const char* normal_mapped_mesh::FRAGMENT_SHADER = "gpu/normal_mapped_mesh.frag.glsl";

normal_mapped_mesh::normal_mapped_mesh(const material_t& mat,const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& difftex,const s_image& nm_text):
	mesh(),
	program_(),
	vbo_(),
	ibo_(),
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
	ibo_ = gl::buffer::create( indexes, isize,
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW );

	gl::shader_program_attribute layout[4] = {
		{VATTR_CRD,3},
		{VATTR_NRM,3},
		{VATTR_UV,2},
		{VATTR_TAN,3} };

	program_->pass_vertex_attrib_array(vbo_, false, layout, 4);

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

normal_mapped_mesh::normal_mapped_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& difftex,const s_image& nm_text):
	normal_mapped_mesh(DEFAULT_MATERIAL, vertex, vsize, indexes, isize, difftex, nm_text)
{}

void normal_mapped_mesh::draw(const scene& scn) const
{

	glm::mat4 projection_mat;
	glm::mat4 model_view_mat;
	scn.get_frustum(projection_mat,model_view_mat);

	glm::mat4 mvp = projection_mat * model_view_mat;

	program_->start();

	::glUniformMatrix4fv(mvp_ul_, 1, GL_FALSE, glm::value_ptr( mvp ) );
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
	::glDrawElements(GL_TRIANGLES, ibo_->size(), GL_UNSIGNED_INT, 0);
	ibo_->unbind();

	diffuse_tex_->unbind();
	normal_map_tex_->unbind();

	program_->stop();
}

} // namespace engine
