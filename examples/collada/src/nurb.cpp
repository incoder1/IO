#include "stdafx.hpp"
#include "nurb.hpp"

namespace engine {

const char* NURB::VERTEX = "gpu/tess/tess.vertex.glsl";
const char* NURB::FRAGMETN = "gpu/tess/nurb.frag.glsl";
const char* NURB::TCS = "gpu/tess/nurb.tcs.glsl";
const char* NURB::TES = "gpu/tess/nurb.tes.glsl";
const char* NURB::UNFM_TESSELATION_LEVEL = "tess_level";

s_surface NURB::create(const material_t& mat,const float* vbo,std::size_t vbo_size,unsigned int vcount,int tess_level)
{
	gl::shader vtx = gl::shader::load_glsl( gl::shader_type::vertex, gl::shader_file(VERTEX) );
	gl::shader frag = gl::shader::load_glsl( gl::shader_type::fragment, gl::shader_file(FRAGMETN) );
	gl::shader tcs = gl::shader::load_glsl(gl::shader_type::tess_control, gl::shader_file(TCS) );
	gl::shader tes = gl::shader::load_glsl(gl::shader_type::tess_evaluation, gl::shader_file(TES) );
	gl::s_program glpo = gl::program::create( std::move(vtx), std::move(frag) );
	glpo->attach_shader( std::move(tcs) );
	glpo->attach_shader( std::move(tes) );
	return s_surface( new NURB( std::move(glpo), mat, vbo, vbo_size, vcount, tess_level) );
}

NURB::NURB(gl::s_program&& po,const material_t& mat,const float* points,std::size_t points_size,unsigned int vcount,int tess_level):
	surface(),
	mat_helper_(mat),
	light_helper_(),
	program_( std::forward<gl::s_program>(po) ),
	mvp_ul_(-1),
	mv_ul_(-1),
	nrm_ul_(-1),
	tess_level_ul_(-1),
	vao_(0),
	vcount_(vcount),
	tess_level_(tess_level)
{
	::glGenVertexArrays(1, &vao_);
	::glBindVertexArray(vao_);

	gl::s_buffer vbo = gl::buffer::create( points, points_size,
										   gl::buffer_type::ARRAY_BUFFER, gl::buffer_usage::STATIC_DRAW);

	vbo->bind();
	gl::shader_program_attribute layout[1] = { {VATTR_CRD,3} };
	program_->pass_vertex_attrib_array(vbo, false, layout, 1);

	program_->link();

	// take matrix uniforms
	mvp_ul_ = program_->uniform_location(UNFM_MVP_MAT);
	mv_ul_ = program_->uniform_location(UNFM_MV_MAT);
	nrm_ul_ = program_->uniform_location(UNFM_NORMAL_MAT);
	// other uniforms
	tess_level_ul_ = program_->uniform_location(UNFM_TESSELATION_LEVEL);

	// material and light uniforms
	mat_helper_.bind_to_shader(program_);
	light_helper_.bind_to_shader(program_);

	::glPatchParameteri(GL_PATCH_VERTICES, vcount);

	// unbind vao
	::glBindVertexArray(0);
}

NURB::~NURB() noexcept
{
	::glDeleteVertexArrays(1, &vao_);
}

void NURB::draw(const scene& scn) const
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

	// transfer tesselation level
	::glUniform1i(tess_level_ul_, tess_level_);

	// transfer light
	light_helper_.transfer_to_shader( scn.light() );
	// transfer material
	mat_helper_.transfer_to_shader();

	::glBindVertexArray(vao_);

	::glDrawArrays(GL_PATCHES, 0, vcount_);

	// unbind VAO
	::glBindVertexArray(0);

}

} // namespace engine
