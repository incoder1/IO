/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "mesh.hpp"

#ifdef  __IO_WINDOWS_BACKEND__
#	include <windows.h>
#else
#	include <cstdio>
#	include <climits>
#	include <sys/types.h>
#	include <unistd.h>
#endif // defined

namespace engine {

// mesh
mesh::~mesh() noexcept
{}

// geometry_mesh
const char* geometry_mesh::VERTEX_SHADER = "gpu/mesh/geometry_mesh.vertex.glsl";

const char* geometry_mesh::FRAGMENT_SHADER = "gpu/mesh/geometry_mesh.frag.glsl";


geometry_mesh::geometry_mesh(sufrace_data&& data):
	mesh(),
	program_(),
	vao_(0),
	vertex_count_( data.vertex_count() ),
	mat_helper_( data.material() ),
	light_helper_(),
	mvp_ul_(-1),
	mv_ul_(-1),
	nrm_ul_(-1),
	elemens_draw_( data.index() )
{
	gl::shader vertex_sh = gl::shader::load_glsl(gl::shader_type::vertex,gl::shader_file(VERTEX_SHADER));
	gl::shader fragment_sh =  gl::shader::load_glsl(gl::shader_type::fragment,gl::shader_file(FRAGMENT_SHADER) );

	program_ = gl::program::create( std::move(vertex_sh), std::move(fragment_sh) );

	::glGenVertexArrays(1, &vao_);
	::glBindVertexArray(vao_);

	// bind VBO to VAO
	 gl::s_buffer vbo = gl::buffer::create(
							   data.vertex(),
							   gl::buffer_type::ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW
							);
	vbo->bind();
	// bind VIO to VAO if we have index
	// object should exist until vao is unbind
	gl::s_buffer vio;
	if( elemens_draw_) {
		vio = gl::buffer::create( data.index(),
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW );
		vio->bind();
	}

	gl::shader_program_attribute layout[2] = {
		{VATTR_CRD,3},
		{VATTR_NRM,3}
	};

	program_->pass_vertex_attrib_array(vbo, false, layout, 2);

	program_->link();

	mvp_ul_ = program_->uniform_location(UNFM_MVP_MAT);
	mv_ul_ = program_->uniform_location(UNFM_MV_MAT);
	nrm_ul_ = program_->uniform_location(UNFM_NORMAL_MAT);

	mat_helper_.bind_to_shader(program_);
	light_helper_.bind_to_shader(program_);

	::glBindVertexArray(0);
}

void geometry_mesh::draw(const scene& scn) const
{
	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;
	scn.world(projection_mat,model_view_mat);
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

	::glBindVertexArray(vao_);
	// draw with index, if any
	if( elemens_draw_ )
		::glDrawElements(GL_TRIANGLES, vertex_count_, GL_UNSIGNED_INT, nullptr );
	else
		::glDrawArrays(GL_TRIANGLES, 0, vertex_count_);

	::glBindVertexArray(0);

	program_->stop();
}

geometry_mesh::~geometry_mesh() noexcept
{
	::glDeleteVertexArrays(1, &vao_);
}

// colored_geometry_mesh
const char* colored_geometry_mesh::VERTEX_SHADER = "gpu/mesh/colored_geometry_mesh.vertex.glsl";

const char* colored_geometry_mesh::FRAGMENT_SHADER = "gpu/mesh/geometry_mesh.frag.glsl";

colored_geometry_mesh::colored_geometry_mesh(sufrace_data&& data):
	mesh(),
	program_(),
	vao_(0),
	vertex_count_( data.vertex_count() ),
	mat_helper_( data.material() ),
	light_helper_(),
	mvp_ul_(-1),
	mv_ul_(-1),
	nrm_ul_(-1)
{

	gl::shader vertex_sh = gl::shader::load_glsl(gl::shader_type::vertex,gl::shader_file(VERTEX_SHADER));
	gl::shader fragment_sh =  gl::shader::load_glsl(gl::shader_type::fragment,gl::shader_file(FRAGMENT_SHADER));

	program_ = gl::program::create( std::move(vertex_sh), std::move(fragment_sh) );

	::glGenVertexArrays(1, &vao_);
	::glBindVertexArray(vao_);

	gl::s_buffer vbo = gl::buffer::create( data.vertex(),
							   gl::buffer_type::ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW
							 );
	gl::s_buffer vio = gl::buffer::create( data.index(),
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW );

	gl::shader_program_attribute layout[3] = {
			{VATTR_CRD,3},
			{VATTR_CRL,3},
			{VATTR_NRM,3}
		};

	program_->pass_vertex_attrib_array(vbo, false, layout, 3);

	program_->link();

	mvp_ul_ = program_->uniform_location(UNFM_MVP_MAT);
	mv_ul_ = program_->uniform_location(UNFM_MV_MAT);
	nrm_ul_ = program_->uniform_location(UNFM_NORMAL_MAT);

	mat_helper_.bind_to_shader(program_);
	light_helper_.bind_to_shader(program_);

	::glBindVertexArray(0);

}

void colored_geometry_mesh::draw(const scene& scn) const
{
	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;
	scn.world(projection_mat,model_view_mat);
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

	::glBindVertexArray(vao_);
	::glDrawElements(GL_TRIANGLES, vertex_count_ , GL_UNSIGNED_INT, nullptr);
	::glBindVertexArray(0);

	program_->stop();
}

colored_geometry_mesh::~colored_geometry_mesh() noexcept
{
	::glDeleteVertexArrays(1, &vao_);
}

//textured_mesh

const char* textured_mesh::VERTEX_SHADER = "gpu/mesh/textured_mesh.vertex.glsl";

const char* textured_mesh::FRAGMENT_SHADER = "gpu/mesh/textured_mesh.frag.glsl";

textured_mesh::textured_mesh(sufrace_data&& data,const gl::s_texture& texture):
	mesh(),
	program_(),
	vao_(0),
	vertex_count_( data.vertex_count() ),
	texture_( texture ),
	mat_helper_( data.material() ),
	light_helper_(),
	mvp_ul_(-1),
	mv_ul_(-1),
	nrm_ul_(-1),
	diffise_tex_ul_(-1),
	elemens_draw_( data.index() )
{
	gl::shader vertex_sh = gl::shader::load_glsl(gl::shader_type::vertex,gl::shader_file(VERTEX_SHADER));
	gl::shader fragment_sh =  gl::shader::load_glsl(gl::shader_type::fragment,gl::shader_file(FRAGMENT_SHADER) );
	program_ = gl::program::create( std::move(vertex_sh), std::move(fragment_sh) );

	::glGenVertexArrays(1, &vao_);
	::glBindVertexArray(vao_);

	gl::s_buffer vbo = gl::buffer::create( data.vertex(),
							   gl::buffer_type::ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW
							 );

	vbo->bind();
	// bind VIO to VAO if we have index
	// object should exist until vao is unbind
	gl::s_buffer vio;
	if( elemens_draw_) {
		vio = gl::buffer::create( data.index(),
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW );
		vio->bind();
	}


	gl::shader_program_attribute layout[3] = {
		{VATTR_CRD,3},
		{VATTR_NRM,3},
		{VATTR_UV,2} };

	program_->pass_vertex_attrib_array(vbo, false, layout, 3);

	::glBindVertexArray(0);

	program_->link();

	mvp_ul_ = program_->uniform_location(UNFM_MVP_MAT);
	mv_ul_ = program_->uniform_location(UNFM_MV_MAT);
	nrm_ul_ = program_->uniform_location(UNFM_NORMAL_MAT);

	mat_helper_.bind_to_shader(program_);
	light_helper_.bind_to_shader(program_);

	diffise_tex_ul_ = program_->uniform_location(UNFM_DIFFUSE_TEXTURE);
}

void textured_mesh::draw(const scene& scn) const
{
	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;
	scn.world(projection_mat,model_view_mat);
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

	::glBindVertexArray(vao_);
	// draw with index, if any
	if( elemens_draw_ )
		::glDrawElements(GL_TRIANGLES, vertex_count_, GL_UNSIGNED_INT, nullptr );
	else
		::glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
	::glBindVertexArray(0);

	texture_->unbind();

	program_->stop();
}

textured_mesh::~textured_mesh() noexcept
{
	::glDeleteVertexArrays(1, &vao_);
}

//normal_mapped_mesh
const char* normal_mapped_mesh::VERTEX_SHADER = "gpu/mesh/normal_mapped_mesh.vertex.glsl";

const char* normal_mapped_mesh::FRAGMENT_SHADER = "gpu/mesh/normal_mapped_mesh.frag.glsl";

normal_mapped_mesh::normal_mapped_mesh(sufrace_data&& data,
				const gl::s_texture& difftex,
				const gl::s_texture& bump_text):
	mesh(),
	program_(),
	vao_(0),
	vertex_count_( data.vertex_count() ),
	diffuse_tex_( difftex ),
	bumpmap_tex_( bump_text ),
	mat_helper_( data.material() ),
	light_helper_(),
	mvp_ul_(-1),
	mv_ul_(-1),
	diffise_tex_ul_(-1),
	bump_tex_ul_(-1),
	elemens_draw_( data.index() )
{
	gl::shader vertex_sh = gl::shader::load_glsl(gl::shader_type::vertex,gl::shader_file(VERTEX_SHADER));
	gl::shader fragment_sh =  gl::shader::load_glsl(gl::shader_type::fragment,gl::shader_file(FRAGMENT_SHADER) );
	program_ = gl::program::create( std::move(vertex_sh), std::move(fragment_sh) );

	::glGenVertexArrays(1, &vao_);
	::glBindVertexArray(vao_);

	gl::s_buffer vbo = gl::buffer::create( data.vertex(),
							   gl::buffer_type::ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW
							 );
	vbo->bind();
	// bind VIO to VAO if we have index
	// object should exist until vao is unbind
	gl::s_buffer vio;
	if( elemens_draw_) {
		vio = gl::buffer::create( data.index(),
							   gl::buffer_type::ELEMENT_ARRAY_BUFFER,
							   gl::buffer_usage::STATIC_DRAW );
		vio->bind();
	}


	gl::shader_program_attribute layout[4] = {
		{VATTR_CRD,3},
		{VATTR_NRM,3},
		{VATTR_UV,2},
		{VATTR_TAN,3} };

	program_->pass_vertex_attrib_array(vbo, false, layout, 4);

	::glBindVertexArray(0);

	program_->link();

	mvp_ul_ = program_->uniform_location(UNFM_MVP_MAT);
	mv_ul_ = program_->uniform_location(UNFM_MV_MAT);

	mat_helper_.bind_to_shader(program_);
	light_helper_.bind_to_shader(program_);

	diffise_tex_ul_ = program_->uniform_location(UNFM_DIFFUSE_TEXTURE);
	bump_tex_ul_ = program_->uniform_location(UNFM_BUMPMAP_TEXTURE);

}

void normal_mapped_mesh::draw(const scene& scn) const
{

	glm::mat4 projection_mat;
	glm::mat4 model_view_mat;
	scn.world(projection_mat,model_view_mat);

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
	::glUniform1i(bump_tex_ul_, 1);

	bumpmap_tex_->bind();

	::glBindVertexArray(vao_);
	// draw with index, if any
	if( elemens_draw_ )
		::glDrawElements(GL_TRIANGLES, vertex_count_, GL_UNSIGNED_INT, nullptr );
	else
		::glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
	::glBindVertexArray(0);

	diffuse_tex_->unbind();
	bumpmap_tex_->unbind();

	program_->stop();
}

normal_mapped_mesh::~normal_mapped_mesh() noexcept
{
	::glDeleteVertexArrays(1, &vao_);
}

} // namespace engine
