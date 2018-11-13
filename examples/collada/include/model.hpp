#ifndef __MODEL_HPP_INCLUDED__
#define __MODEL_HPP_INCLUDED__

#include <object.hpp>

#include "vbo.hpp"
#include "image.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "texture.hpp"

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

class textured_static_mesh final: public model
{
public:
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
	::GLint mvpUL_;
	::GLint modelVeiwMatUL_;
	::GLint normalMatUL_;
	::GLint textureUL_;
};

class normal_mapped_static_mesh final: public model
{
public:
	normal_mapped_static_mesh(const float *vertex, std::size_t vsize,const uint32_t* indexes,std::size_t isize,const s_image& difftex,const s_image& nm_text);
	virtual void draw(const scene& scn) const override;
private:
	static const char* VERTEX_SHADER;
	static const char* FRAGMENT_SHADER;
	gl::s_program program_;
	gl::s_buffer vbo_;
	gl::s_buffer ibo_;
	std::size_t isize_;
	gl::s_texture diffuse_tex_;
	gl::s_texture normal_map_tex_;
	::GLint mvpUL_;
	::GLint modelVeiwMatUL_;
	::GLint diffiseTxtrUL_;
	::GLint nmTxtrUL_;
};


} // namespace engine

#endif // __MODEL_HPP_INCLUDED__
