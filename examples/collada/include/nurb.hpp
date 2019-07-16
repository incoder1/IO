#ifndef __NURB_HPP_INCLUDED__
#define __NURB_HPP_INCLUDED__

#include <channels.hpp>

#include "vbo.hpp"
#include "shader.hpp"
#include "surface.hpp"
#include "texture.hpp"

namespace engine {

namespace detail {

class tess_builder {
public:

	/// Adds vertex shader
	void vertex(const io::s_read_channel& src);
	/// Adds fragment shader
	void fragment(const io::s_read_channel& src);
	/// Adds Tessellation Control Shader
	void tcs(const io::s_read_channel& src);
	/// Adds Tessellation Evaluation Shader
	void tes(const io::s_read_channel& src);

	gl::s_program build();

private:
	gl::shader vertex_;
	gl::shader fragment_;
	gl::shader tcs_;
	gl::shader tes_;
};

} // namespace detail


/// Nonuniform Rational B-Spline Surface
class NURB: public surface
{
	public:
		NURB(gl::s_program&& po);
		virtual ~NURB() noexcept override;
		virtual void draw(const scene& scn) const override;
	private:

	gl::s_program program_;

	::GLint mvp_ul_;
	::GLint mv_ul_;
};

} // namespace engine

#endif // __NURB_HPP_INCLUDED__
