#include "stdafx.hpp"
#include "surface.hpp"

namespace engine {


//surface
surface::~surface() noexcept
{}

// material_helper
material_helper::material_helper(const material_t& mat) noexcept:
	adse_ul_(-1),
	shininess_ul_(-1),
	mat_(mat)
{}

void material_helper::bind_to_shader(const gl::s_program& prg)
{
	adse_ul_ = prg->uniform_location(UNFM_MAT_ADSE_UL);
	shininess_ul_ = prg->uniform_location(UNFM_MAT_SHININESS);
}

void material_helper::transfer_to_shader() const noexcept
{
	::glUniformMatrix4fv(adse_ul_, 1, GL_FALSE, mat_.adse);
	::glUniform1f(shininess_ul_, mat_.shininess);
}

// light_helper
light_helper::light_helper() noexcept:
	pads_ul_(-1)
{}

void light_helper::bind_to_shader(const gl::s_program& prg)
{
	pads_ul_ = prg->uniform_location(UNFM_LIGH_PADS);
}

void light_helper::transfer_to_shader(const light_t& light) const noexcept
{
	::glUniformMatrix4fv(pads_ul_, 1, GL_FALSE, light.pads);
}

} // namespace engine
