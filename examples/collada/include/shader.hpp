#ifndef __SHADER_HPP_INCLUDED__
#define __SHADER_HPP_INCLUDED__

#include <algorithm>
#include <vector>
#include <memory>

#include <object.hpp>
#include <buffer.hpp>
#include <channels.hpp>
#include <files.hpp>

#include "openglload.hpp"
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


// Shader object.
class shader
{
	shader(const shader&) = delete;
	shader& operator=(const shader&) = delete;
public:

	static shader load_glsl(shader_type type,const io::s_read_channel& src);
	static shader load_glsl(shader_type type,const io::file& file);

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


class program;
DECLARE_IPTR(program);
class program final:public io::object {
private:
	program(shader&& vertex, shader&& fragment);
public:
	static s_program create(shader&& vertex, shader&& fragment);
	void attach_shader(shader&& sh);
	void link();
	inline void start() {
		::glUseProgram(hprg_);
	}
	inline void stop() {
		::glUseProgram(0);
	}
	void pass_vertex_attrib_array(::GLsizei attr_no, const s_buffer& vbo, bool normalized,uint8_t stride, uint8_t size, uint8_t offset);
	void bind_attrib_location(::GLsizei attr_no, const char* name);
	::GLuint uniform_location(const char* uniform);
	::GLuint attribute_number(const char* attribute_name);
	virtual ~program() noexcept override;
private:
	::GLuint hprg_;
	std::vector<shader> shaders_;
};

} // namespace gl

#endif // __SHADER_HPP_INCLUDED__
