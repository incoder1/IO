#ifndef __NURB_HPP_INCLUDED__
#define __NURB_HPP_INCLUDED__

#include <channels.hpp>

#include "vbo.hpp"
#include "shader.hpp"
#include "surface.hpp"

namespace engine {

/// Nonuniform Rational B-Spline Surface
class NURB: public surface
{
	public:
		NURB(const io::s_read_channel vtx,const io::s_read_channel& frag,const io::s_read_channel& tess,const io::s_read_channel& tessctrl);
		virtual ~NURB() noexcept;
	private:

	gl::s_program program_;

	::GLint mvp_ul_;
	::GLint mv_ul_;
};

} // namespace engine

#endif // __NURB_HPP_INCLUDED__
