#ifndef __NURB_HPP_INCLUDED__
#define __NURB_HPP_INCLUDED__

#include "vbo.hpp"
#include "shader.hpp"
#include "surface.hpp"
#include "texture.hpp"

namespace engine {


/// Nonuniform Rational B-Spline Surface
/// WARN! OpenGL 4.2+ required, since tesselation stage needed
class NURB: public surface {
private:
	/// Vertex shader name
	static const char* VERTEX;
	/// Fragment shader name
	static const char* FRAGMETN;
	/// tesselation control shader name
	static const char* TCS;
	/// tesselation evaluation shader name
	static const char* TES;
	/// tesselation level uniform name
	static const char* UNFM_TESSELATION_LEVEL;
public:
	/// Recreates new NURB surface
	/// \param mat surface material, for Phong shading
	/// \param vbo NURB control points vertex buffer object
	/// \param vbo_size size of VBO in array elements
	/// \param vcount control points count
	/// \param tess_level tesselation level (Bezier interpolation level)
	static s_surface create(const material_t& mat,const float* vbo,std::size_t vbo_size,unsigned int vcount,int tess_level);
	virtual ~NURB() noexcept override;
	virtual void draw(const scene& scn) const override;
private:

	NURB(gl::s_program&& po,const material_t& mat,const float* points,std::size_t points_size,unsigned int vcount,int tess_level);

	material_helper mat_helper_;
	light_helper light_helper_;

	gl::s_program program_;

	::GLint mvp_ul_;
	::GLint mv_ul_;
	::GLint nrm_ul_;
	::GLint tess_level_ul_;

	::GLuint vao_;

	unsigned int vcount_;
	int tess_level_;
};

} // namespace engine

#endif // __NURB_HPP_INCLUDED__
