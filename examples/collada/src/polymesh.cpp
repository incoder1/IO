#include "stdafx.hpp"
#include "polymesh.hpp"

namespace engine {

const char* poly_mesh::VERTEX = "gpu/tess/polygon.vertex.glsl";
const char* poly_mesh::FRAGMETN = "gpu/tess/polygon.frag.glsl";
const char* poly_mesh::TCS = "gpu/tess/polygon.tcs.glsl";
const char* poly_mesh::TES = "gpu/tess/polygon.tes.glsl";

s_surface poly_mesh::create(const material_t& mat,const float* points,std::size_t points_size,unsigned int vcount)
{
	gl::shader vtx = gl::shader::load_glsl(gl::shader_type::vertex, gl::shader_file(VERTEX));
	gl::shader frag = gl::shader::load_glsl(gl::shader_type::fragment, gl::shader_file(FRAGMETN));
	gl::shader tcs = gl::shader::load_glsl(gl::shader_type::tess_control, gl::shader_file(TCS));
	gl::shader tes = gl::shader::load_glsl(gl::shader_type::tess_evaluation, gl::shader_file(TES));
	gl::s_program glpo = gl::program::create(std::move(vtx),std::move(frag));
	glpo->attach_shader( std::move(tcs) );
	glpo->attach_shader( std::move(tes) );
	return s_surface( new poly_mesh( std::move(glpo), mat, points, points_size, vcount ) );
}

poly_mesh::poly_mesh(gl::s_program&& po,const material_t& mat,const float* points,std::size_t points_size,unsigned int vcount):
	surface(),
	mat_helper_(mat),
	light_helper_(),
	program_( std::forward<gl::s_program>(po) ),
	mvp_ul_(-1),
	mv_ul_(-1),
	nrm_ul_(-1),
	tess_level_ul_(-1),
	vao_(0),
	vcount_(vcount)
{}

poly_mesh::~poly_mesh() noexcept
{
}

void poly_mesh::draw(const scene& scn) const
{
	::glPatchParameteri(GL_PATCH_VERTICES, vcount_);
}

} // namespace engine
