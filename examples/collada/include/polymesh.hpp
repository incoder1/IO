#ifndef __POLYMESH_HPP_INCLUDED__
#define __POLYMESH_HPP_INCLUDED__

#include "vbo.hpp"
#include "shader.hpp"
#include "surface.hpp"
#include "texture.hpp"

namespace engine {

/// Polygonal based mesh surface (patch). e.g. mesh combined from polygons instead of triangles
/// WARN! OpenGL 4.2+ required, since tesselation and geometry stages needed
class poly_mesh final:public surface
{
	private:
		/// Vertex shader name
		static const char* VERTEX;
		/// Fragment shader name
		static const char* FRAGMETN;
		/// tesselation control shader name
		static const char* TCS;
		/// tesselation evaluation shader name
		static const char* TES;
		static const char* UNFM_TESSELATION_LEVEL;
	public:
		static s_surface create(const material_t& mat,const float* points,std::size_t points_size);
		virtual void draw(const scene& scn) const override;
		virtual ~poly_mesh() noexcept override;
	private:
		poly_mesh(gl::s_program&& po,const material_t& mat,const float* points,std::size_t points_size);
	private:
		material_helper mat_helper_;
		light_helper light_helper_;

		gl::s_program program_;

		// model view projection matrix uniform identifier
		::GLint mvp_ul_;
		// normal matrix uniform identifier
		::GLint nrm_ul_;
		// tesselation level uniform identifier
		::GLint tess_level_ul_;
		// vertex array object
		::GLuint vao_;

		unsigned int vcount_;
};

} // namespace engine

#endif // __POLYMESH_HPP_INCLUDED__
