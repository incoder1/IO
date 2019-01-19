#ifndef __STATIC_MESH_HPP_INCLUDED__
#define __STATIC_MESH_HPP_INCLUDED__

#include "image.hpp"
#include "vbo.hpp"
#include "shader.hpp"
#include "surface.hpp"
#include "texture.hpp"

namespace engine {

class static_mesh : public surface
{
protected:
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

	constexpr static_mesh() noexcept:
		surface()
	{}

};

class untextured_static_mesh final: public static_mesh
{
public:
	untextured_static_mesh(const material_t& mat, const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize);
	untextured_static_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize);
	virtual void draw(const scene& scn) const override;
	virtual ~untextured_static_mesh() noexcept = default;
private:
	static constexpr const char* VATTR_CRL = "vertex_color";

	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;

	gl::s_program program_;
	gl::s_buffer vbo_;
	gl::s_buffer ibo_;
	std::size_t isize_;

	material_helper mat_helper_;
	light_helper light_helper_;

	::GLint mvp_ul_;
	::GLint mv_ul_;
	::GLint nrm_ul_;
};

class textured_static_mesh final: public static_mesh
{
public:
	textured_static_mesh(const material_t& mat,const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& texture);
	textured_static_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& texture);
	virtual void draw(const scene& scn) const override;
	virtual ~textured_static_mesh() noexcept = default;
private:
	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;

	gl::s_program program_;
	gl::s_buffer vbo_;
	gl::s_buffer ibo_;
	std::size_t isize_;

	gl::s_texture texture_;
	material_helper mat_helper_;
	light_helper light_helper_;

	::GLint mvp_ul_;
	::GLint mv_ul_;
	::GLint nrm_ul_;

	::GLint diffise_tex_ul_;
};


class normal_mapped_static_mesh final: public static_mesh
{
public:
	normal_mapped_static_mesh(const material_t& mat,const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& difftex,const s_image& nm_text);
	normal_mapped_static_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& difftex,const s_image& nm_text);
	virtual void draw(const scene& scn) const override;
private:
	static constexpr const char* VATTR_TAN = "tangent";
	static constexpr const char* UNFM_NORMALMAP_TEXTURE = "normal_map_texture";

	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;

	gl::s_program program_;
	gl::s_buffer vbo_;
	gl::s_buffer ibo_;
	std::size_t isize_;

	gl::s_texture diffuse_tex_;
	gl::s_texture normal_map_tex_;

	material_helper mat_helper_;
	light_helper light_helper_;

	::GLint mvp_ul_;
	::GLint mv_ul_;

	::GLint diffise_tex_ul_;
	::GLint nm_tex_ul_;
};

} // namespace engine


#endif // __STATIC_MESH_HPP_INCLUDED__
