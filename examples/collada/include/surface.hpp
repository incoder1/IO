#ifndef __SURFACE_HPP_INCLUDED__
#define __SURFACE_HPP_INCLUDED__

#include <object.hpp>

#include "renderer.hpp"
#include "scene.hpp"
#include "shader.hpp"

namespace engine {

class surface: public io::object
{
	surface(const surface&) = delete;
	surface& operator=(const surface&) = delete;
protected:
	constexpr surface() noexcept:
		io::object()
	{}
public:
	virtual ~surface() noexcept = 0;
	virtual void draw(const scene& scn) const = 0;
};

DECLARE_IPTR(surface);

class material_helper
{
	material_helper(const material_helper&) = delete;
	material_helper& operator=(const material_helper&) = delete;
private:
	static constexpr const char* UNFM_MAT_ADSE_UL = "material_adse";
	static constexpr const char* UNFM_MAT_SHININESS = "material_shines";
public:
	material_helper(const material_t& mat) noexcept;
	void bind_to_shader(const gl::s_program& prg);
	void transfer_to_shader() const noexcept;
private:
	::GLint adse_ul_;
	::GLint shininess_ul_;
	material_t mat_;
};

class light_helper {
	light_helper(const light_helper&) = delete;
	light_helper& operator=(const light_helper&) = delete;
public:
	light_helper() noexcept;
	void bind_to_shader(const gl::s_program& prg);
	void transfer_to_shader(const light_t& light) const noexcept;
private:
	static constexpr const char* UNFM_LIGH_PADS = "light_pads";
	::GLint pads_ul_;
};


} // namespace engine

#endif // __SURFACE_HPP_INCLUDED__
