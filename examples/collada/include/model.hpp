#ifndef __MODEL_HPP_INCLUDED__
#define __MODEL_HPP_INCLUDED__

#include <object.hpp>

#include "vbo.hpp"
#include "shader.hpp"
#include "scene.hpp"

namespace engine {

class model: public io::object
{
protected:
	constexpr model() noexcept:
		io::object()
	{}
public:
	virtual void draw(const scene& scn) const = 0;
};

DECLARE_IPTR(model);


class untextured_static_mesh final: public model
{
public:
	untextured_static_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize);
	virtual void draw(const scene& scn) const override;
	virtual ~untextured_static_mesh() noexcept = default;
private:
	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;
	gl::s_program program_;
	gl::s_buffer vbo_;
	gl::s_buffer ibo_;
	std::size_t isize_;
	::GLint mvpUL_;
	::GLint modelVeiwMatUL_;
	::GLint normalMatUL_;
};


} // namespace engine

#endif // __MODEL_HPP_INCLUDED__
