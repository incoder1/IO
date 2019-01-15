#ifndef __NURB_HPP_INCLUDED__
#define __NURB_HPP_INCLUDED__

#include <channels.hpp>

#include "vbo.hpp"
#include "shader.hpp"
#include "surface.hpp"

namespace engine {

class NURB: public surface
{
	public:
		NURB(const io::s_read_channel vtx,const io::s_read_channel& frag,const io::s_read_channel& tess,const io::s_read_channel& tessctrl);
		virtual ~NURB();
	private:

	gl::s_program program_;
	gl::s_buffer vbo_;
	gl::s_buffer ibo_;
	std::size_t isize_;
	::GLint mvp_UL_;
	::GLint modelVeiwMatUL_;
	::GLint normalMatUL_;
};

} // namespace engine

#endif // __NURB_HPP_INCLUDED__
