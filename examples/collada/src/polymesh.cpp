#include "stdafx.hpp"
#include "polymesh.hpp"

namespace engine {

const char* poly_mesh::VERTEX = "gpu/tess/polygon.vertex.glsl";
const char* poly_mesh::FRAGMETN = "gpu/tess/polygon.frag.glsl";
const char* poly_mesh::TCS = "gpu/tess/polygon.tcs.glsl";
const char* poly_mesh::TES = "gpu/tess/polygon.tes.glsl";
const char* poly_mesh::UNFM_TESSELATION_LEVEL = "tess_level";

s_surface poly_mesh::create(const material_t& mat,float_array_view&& knots, byte_array&& vertices)
{
	gl::shader vtx = gl::shader::load_glsl(gl::shader_type::vertex, gl::shader_file(VERTEX));
	gl::shader frag = gl::shader::load_glsl(gl::shader_type::fragment, gl::shader_file(FRAGMETN));
	gl::shader tcs = gl::shader::load_glsl(gl::shader_type::tess_control, gl::shader_file(TCS));
	gl::shader tes = gl::shader::load_glsl(gl::shader_type::tess_evaluation, gl::shader_file(TES));
	gl::s_program glpo = gl::program::create(std::move(vtx),std::move(frag));
	glpo->attach_shader( std::move(tcs) );
	glpo->attach_shader( std::move(tes) );
	return s_surface( new poly_mesh(
					std::move(glpo),
					mat,
					std::forward<float_array_view>(knots),
					std::forward<byte_array>(vertices) ) );
}

poly_mesh::poly_mesh(gl::s_program&& po,const material_t& mat,float_array_view&& knots, byte_array&& vertices):
	surface(),
	mat_helper_(mat),
	light_helper_(),
	program_( std::forward<gl::s_program>(po) ),
	mvp_ul_(-1),
	nrm_ul_(-1),
	tess_level_ul_(-1),
	vao_(0),
	vertices_( std::forward<byte_array>(vertices) )
{
	::glGenVertexArrays(1, &vao_);
	::glBindVertexArray(vao_);

	gl::s_buffer vbo = gl::buffer::create( knots, gl::buffer_type::ARRAY_BUFFER, gl::buffer_usage::STATIC_DRAW);

	vbo->bind();
	gl::shader_program_attribute layout[2] = { {VATTR_CRD,3},{VATTR_NRM,3} };
	program_->pass_vertex_attrib_array(vbo, false, layout, 2);

	program_->link();

	// take matrix uniforms
	mvp_ul_ = program_->uniform_location(UNFM_MVP_MAT);
	nrm_ul_ = program_->uniform_location(UNFM_NORMAL_MAT);
	// other uniforms
	tess_level_ul_ = program_->uniform_location(UNFM_TESSELATION_LEVEL);

	// material and light uniforms
	mat_helper_.bind_to_shader(program_);
	light_helper_.bind_to_shader(program_);


	// unbind vao
	::glBindVertexArray(0);
}

poly_mesh::~poly_mesh() noexcept
{
	::glDeleteVertexArrays(1, &vao_);
}

void poly_mesh::draw(const scene& scn) const
{
	::glm::mat4 projection_mat;
	::glm::mat4 model_view_mat;
	scn.world(projection_mat,model_view_mat);
	::glm::mat4 normal_mat( glm::transpose( glm::inverse(  glm::mat3(model_view_mat) ) ) );

	program_->start();

	// transfer world
	::glUniformMatrix4fv(mvp_ul_, 1, GL_FALSE, glm::value_ptr( projection_mat * model_view_mat ) );
	::glUniformMatrix4fv(nrm_ul_, 1, GL_FALSE, glm::value_ptr( normal_mat ) );

	// transfer tesselation level
	::glUniform1i(tess_level_ul_, 4);

	// transfer light
	light_helper_.transfer_to_shader( scn.light() );
	// transfer material
	mat_helper_.transfer_to_shader();

	::glPatchParameteri(GL_PATCH_VERTICES, 4);

	::glBindVertexArray(vao_);

	::glDrawArrays(GL_PATCHES, 0, 144 );

	// unbind VAO
	::glBindVertexArray(0);
}

} // namespace engine
