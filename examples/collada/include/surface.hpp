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
#ifndef __SURFACE_HPP_INCLUDED__
#define __SURFACE_HPP_INCLUDED__

#include <object.hpp>

#include "array_view.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "shader.hpp"

namespace engine {

struct sufrace_data {
public:
	constexpr sufrace_data(const material_t& mat,
							std::size_t vtx_count,
							util::array_view<float>&& vtx,
							util::array_view<unsigned int>&& idx) noexcept:
		material_(mat),
		vertex_count_(vtx_count),
		vertex_(std::forward<util::array_view<float> >(vtx)),
		index_(std::forward<util::array_view<unsigned int> >(idx))
	{}
	material_t material() const noexcept {
		return material_;
	}
	std::size_t vertex_count() const noexcept {
		return vertex_count_;
	}
	util::array_view<float> vertex() const noexcept {
		return vertex_;
	}
	util::array_view<unsigned int> index() const noexcept {
		return index_;
	}
private:
	material_t material_;
	std::size_t vertex_count_;
	util::array_view<float> vertex_;
	util::array_view<unsigned int> index_;
};

/// Basic 3D primitive type, i.e. base for mesh, polymesh, NURB etc
class surface: public io::object
{
	surface(const surface&) = delete;
	surface& operator=(const surface&) = delete;
protected:
	// Unified engine shader uniform and vertex attribute names
	// used by all type of engine shaders as code convention

	/// Vertex coordinate shader attribute name
	static constexpr const char* VATTR_CRD = "vertex_coord";
	/// Vertex normal vector shader attribute name
	static constexpr const char* VATTR_NRM = "vertex_normal";
	/// Vertex texture coordinates shader attribute name
	static constexpr const char* VATTR_UV = "vertex_uv";

	/// model-view-projection matrix uniform name
	static constexpr const char* UNFM_MVP_MAT = "mvp";
	/// model-view matrix uniform name
	static constexpr const char* UNFM_MV_MAT = "mv";
	/// normal matrix uniform name
	static constexpr const char* UNFM_NORMAL_MAT = "nm";
	/// diffuse texture uniform name
	static constexpr const char* UNFM_DIFFUSE_TEXTURE = "diffuse_texture";

	constexpr surface() noexcept:
		io::object()
	{}
public:
	virtual ~surface() noexcept = 0;
	virtual void draw(const scene& scn) const = 0;
};

DECLARE_IPTR(surface);

/// Helper class to transfer a material data into engines GLSL program
class material_helper
{
	material_helper(const material_helper&) = delete;
	material_helper& operator=(const material_helper&) = delete;
private:
	static constexpr const char* UNFM_MAT_ADSE_UL = "material_adse";
	static constexpr const char* UNFM_MAT_SHININESS = "material_shines";
public:
	material_helper(const material_t& mat) noexcept;
	/// Takes uniform identifiers from linked program objects
	void bind_to_shader(const gl::s_program& prg);
	/// Pass material uniforms into GPU program
	void transfer_to_shader() const noexcept;
private:
	::GLint adse_ul_;
	::GLint shininess_ul_;
	material_t mat_;
};

/// Helper class to transfer a light data into engines GLSL program
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
