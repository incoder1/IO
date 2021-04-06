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
#ifndef __mesh_HPP_INCLUDED__
#define __mesh_HPP_INCLUDED__

#include "array_view.hpp"
#include "image.hpp"
#include "vbo.hpp"
#include "shader.hpp"
#include "surface.hpp"
#include "texture.hpp"

namespace engine {

class mesh : public surface
{
protected:
	constexpr mesh() noexcept:
		surface()
	{}
public:
	virtual ~mesh() noexcept = 0;
	virtual void draw(const scene& scn) const override = 0;
};

/// A triangle based mesh which specify only a geometry
/// e.g. {position:[x,y,z],normal[x,y,z]} vertex attributes
/// and a material
class geometry_mesh final: public mesh
{
private:
	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;
public:
	geometry_mesh(sufrace_data&& data);
	virtual void draw(const scene& scn) const override;
	virtual ~geometry_mesh() noexcept;
private:
	gl::s_program program_;
	::GLuint vao_;
	std::size_t vertex_count_;

	material_helper mat_helper_;
	light_helper light_helper_;

	::GLint mvp_ul_;
	::GLint mv_ul_;
	::GLint nrm_ul_;
	::GLint camera_pos_ul_;
	bool elemens_draw_;
};

/// A triangle based mesh which specify geometry an base colors
/// e.g. {position:[x,y,z],color:[r,g,b],normal[x,y,z]} vertex attributes
/// and a material
class colored_geometry_mesh final: public mesh
{
public:
	colored_geometry_mesh(sufrace_data&& data);
	virtual void draw(const scene& scn) const override;
	virtual ~colored_geometry_mesh() noexcept override;
private:
	static constexpr const char* VATTR_CRL = "vertex_color";

	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;

	gl::s_program program_;
	::GLuint vao_;
	std::size_t vertex_count_;

	material_helper mat_helper_;
	light_helper light_helper_;

	::GLint mvp_ul_;
	::GLint mv_ul_;
	::GLint nrm_ul_;
};

/// A triangle based mesh which specify geometry and takes color from applied diffuse texture
/// e.g. {position:[x,y,z],normal[x,y,z],uv:[u,v]} vertex attributes
class textured_mesh final: public mesh
{
public:
	textured_mesh(sufrace_data&& data, const gl::s_texture& texture);
	virtual void draw(const scene& scn) const override;
	virtual ~textured_mesh() noexcept override;
private:
	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;

	gl::s_program program_;
	::GLuint vao_;
	std::size_t vertex_count_;

	gl::s_texture texture_;
	material_helper mat_helper_;
	light_helper light_helper_;

	::GLint mvp_ul_;
	::GLint mv_ul_;
	::GLint nrm_ul_;

	::GLint diffise_tex_ul_;
	bool elemens_draw_;
};

/// A triangle based mesh which specify geometry and takes color from applied diffuse texture
/// as well as contains bump-mapping (normal mapping style) additional depth
/// e.g. {position:[x,y,z],normal[x,y,z],uv:[u,v],tangent:[x,y,z]} vertex attributes
/// Note, bi-tangent is calculated by the GPU with vertex shader and should not be passed
class normal_mapped_mesh final: public mesh
{
public:

	normal_mapped_mesh(sufrace_data&& data,
					const gl::s_texture& difftex,
					const gl::s_texture& nm_text);

	virtual void draw(const scene& scn) const override;
	virtual ~normal_mapped_mesh() noexcept override;
private:
	static constexpr const char* VATTR_TAN = "tangent";
	static constexpr const char* UNFM_BUMPMAP_TEXTURE = "bumpmap_texture";

	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;

	gl::s_program program_;
	::GLuint vao_;
	std::size_t vertex_count_;

	gl::s_texture diffuse_tex_;
	gl::s_texture bumpmap_tex_;

	material_helper mat_helper_;
	light_helper light_helper_;

	::GLint mvp_ul_;
	::GLint mv_ul_;

	::GLint diffise_tex_ul_;
	::GLint bump_tex_ul_;
	bool elemens_draw_;
};

} // namespace engine


#endif // __mesh_HPP_INCLUDED__
