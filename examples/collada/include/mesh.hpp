#ifndef __mesh_HPP_INCLUDED__
#define __mesh_HPP_INCLUDED__

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

};

class geometry_mesh final: public mesh
{
private:
	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;
public:
	geometry_mesh(const material_t& mat, const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize);
	virtual void draw(const scene& scn) const override;
	virtual ~geometry_mesh() noexcept;
private:
	gl::s_program program_;
	::GLuint vao_;
	std::size_t isize_;

	material_helper mat_helper_;
	light_helper light_helper_;

	::GLint mvp_ul_;
	::GLint mv_ul_;
	::GLint nrm_ul_;
	bool elemens_draw_;
};

class colored_geometry_mesh final: public mesh
{
public:
	colored_geometry_mesh(const material_t& mat, const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize);
	colored_geometry_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize);
	virtual void draw(const scene& scn) const override;
	virtual ~colored_geometry_mesh() noexcept override;
private:
	static constexpr const char* VATTR_CRL = "vertex_color";

	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;

	gl::s_program program_;
	::GLuint vao_;
	std::size_t isize_;

	material_helper mat_helper_;
	light_helper light_helper_;

	::GLint mvp_ul_;
	::GLint mv_ul_;
	::GLint nrm_ul_;
};

class textured_mesh final: public mesh
{
public:
	textured_mesh(const material_t& mat,const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const gl::s_texture& texture);
	textured_mesh(const material_t& mat,const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& texture);
	textured_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& texture):
		textured_mesh(DEFAULT_MATERIAL, vertex, vsize,indexes, isize, texture)
	{}
	virtual void draw(const scene& scn) const override;
	virtual ~textured_mesh() noexcept override;
private:
	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;

	gl::s_program program_;
	::GLuint vao_;
	std::size_t isize_;

	gl::s_texture texture_;
	material_helper mat_helper_;
	light_helper light_helper_;

	::GLint mvp_ul_;
	::GLint mv_ul_;
	::GLint nrm_ul_;

	::GLint diffise_tex_ul_;
	bool elemens_draw_;
};


class normal_mapped_mesh final: public mesh
{
public:

	normal_mapped_mesh(const material_t& mat,
				const float *vertex,
				std::size_t vsize,
				const uint32_t* indexes,
				std::size_t isize,
				const gl::s_texture& difftex,const gl::s_texture& nm_text);

	normal_mapped_mesh(const material_t& mat,
				const float *vertex,
				std::size_t vsize,
				const uint32_t* indexes,
				std::size_t isize,
				const s_image& difftex,const s_image& nm_text);



	virtual void draw(const scene& scn) const override;
	virtual ~normal_mapped_mesh() noexcept override;
private:
	static constexpr const char* VATTR_TAN = "tangent";
	static constexpr const char* UNFM_NORMALMAP_TEXTURE = "normal_map_texture";

	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;

	gl::s_program program_;
	::GLuint vao_;
	std::size_t isize_;

	gl::s_texture diffuse_tex_;
	gl::s_texture normal_map_tex_;

	material_helper mat_helper_;
	light_helper light_helper_;

	::GLint mvp_ul_;
	::GLint mv_ul_;

	::GLint diffise_tex_ul_;
	::GLint nm_tex_ul_;
	bool elemens_draw_;
};

} // namespace engine


#endif // __mesh_HPP_INCLUDED__
