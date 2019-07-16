#include "stdafx.hpp"
#include "nurb.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <atomic>

namespace engine {

namespace detail {


void tess_builder::vertex(const io::s_read_channel& src)
{
	vertex_ = gl::shader::load_glsl(gl::shader_type::vertex, src);}

void tess_builder::fragment(const io::s_read_channel& src)
{
	fragment_ = gl::shader::load_glsl(gl::shader_type::fragment, src);
}

void tess_builder::tcs(const io::s_read_channel& src)
{
	tcs_ = gl::shader::load_glsl(gl::shader_type::tess_control, src);
}

void tess_builder::tes(const io::s_read_channel& src)
{
	tes_ = gl::shader::load_glsl(gl::shader_type::tess_evaluation,src);
}

gl::s_program tess_builder::build() {
	gl::s_program ret = gl::program::create( std::move(vertex_), std::move(fragment_) );
	ret->attach_shader( std::move(tcs_) );
	ret->attach_shader( std::move(tes_) );
	return ret;
}

} // namespace detail

NURB::NURB(gl::s_program&& po):
	surface(),
	program_( std::forward<gl::s_program>(po) )
{}

NURB::~NURB() noexcept
{}

void NURB::draw(const scene& scn) const
{

}

} // namespace engine
