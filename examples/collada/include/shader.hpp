#ifndef __SHADER_HPP_INCLUDED__
#define __SHADER_HPP_INCLUDED__

#include "openglload.hpp"

#include <object.hpp>
#include <string>
#include <memory>

#include "vbo.hpp"

namespace gl {

enum class shader_type: ::GLuint {
 comute = GL_COMPUTE_SHADER,
 vertex = GL_VERTEX_SHADER,
 tess_control = GL_TESS_CONTROL_SHADER,
 tess_evaluation = GL_TESS_EVALUATION_SHADER,
 geometry = GL_GEOMETRY_SHADER,
 fragment = GL_FRAGMENT_SHADER
};

class shader
{
	shader(const shader&) = delete;
	shader& operator=(const shader&) = delete;
public:
	shader(shader_type type, const char* source);
	shader(shader&& other) noexcept;
	shader& operator=(shader&& rhs) noexcept;
	~shader() noexcept;
	inline ::GLuint handle() const noexcept {
		return hsdr_;
	}
	inline shader_type type() const noexcept {
		return type_;
	}
	inline void swap(shader& other) noexcept;
private:
	::GLuint hsdr_;
	shader_type type_;
};

class vao {
	vao(const vao&) = delete;
	vao& operator=(const vao&) = delete;
private:

	constexpr vao(::GLuint* const arr, ::GLsizei size) noexcept:
		arr_(arr),
		size_(size)
	{}

public:
	static vao create(::GLsizei size);

	constexpr vao() noexcept:
		vao(nullptr, 0)
	{}

	vao(vao&& c) noexcept:
		arr_( c.arr_ ),
		size_( c.size_ )
	{
		c.arr_ = nullptr;
		c.size_ = 0;
	}

	vao& operator=(vao&& rhs) noexcept
	{
		vao( std::forward<vao>(rhs) ).swap( *this );
		return *this;
	}

	~vao() noexcept;

	::GLuint operator[](std::size_t idx) const noexcept
	{
		return arr_[idx];
	}

	inline void swap(vao& other) noexcept
	{
        std::swap(arr_, other.arr_);
        std::swap(size_, other.size_);
	}

private:
	::GLuint *arr_;
	::GLsizei size_;
};


class program;
DECLARE_IPTR(program);
class program final:public io::object {
private:
	program(shader&& vertex, shader&& fragment);
public:
	static s_program create(shader&& vertex, shader&& fragment);
	void attach_shader(shader&& sh);
	void link();
	void start();
	void stop();
	//void gen_vertex_attrib_arrays(std::size_t count);
	void pass_vertex_attrib_array(::GLsizei attr_no, const s_buffer& vbo, bool normalized,uint8_t stride, uint8_t pack, uint8_t offset);
	void bind_attrib_location(::GLsizei attr_no, const char* name);
	::GLuint uniform_location(const char* uniform);
	::GLuint attribute_number(const char* attribute_name);
	virtual ~program() noexcept override;
private:
	::GLuint hprg_;
	std::vector<shader> shaders_;
	vao vao_;
};

} // namespace gl

#endif // __SHADER_HPP_INCLUDED__
