/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef __SHADER_HPP_INCLUDED__
#define __SHADER_HPP_INCLUDED__

#include <algorithm>
#include <vector>
#include <memory>

#include <core/object.hpp>
#include <core/buffer.hpp>
#include <core/channels.hpp>
#include <core/files.hpp>

#include "openglload.hpp"
#include "vbo.hpp"

namespace gl {

enum class shader_type: ::GLuint {
 compute = GL_COMPUTE_SHADER,
 vertex = GL_VERTEX_SHADER,
 tess_control = GL_TESS_CONTROL_SHADER,
 tess_evaluation = GL_TESS_EVALUATION_SHADER,
 geometry = GL_GEOMETRY_SHADER,
 fragment = GL_FRAGMENT_SHADER
};


/// Gets shader file relative to the program executable directory
io::file shader_file(const char* name);
// Shader object.
class shader
{
	shader(const shader&) = delete;
	shader& operator=(const shader&) = delete;
public:

	/// Loads and compiles a GLSL shader from any readable source
	/// \param type GLSL shading stage type
	/// \param src shader source channel
	/// \return compiled shader
	static shader load_glsl(shader_type type,const io::s_read_channel& src);
	/// Loads and compiles a GLSL shader from file
	/// \param type GLSL shading stage type
	/// \param src shader source file
	/// \return compiled shader
	static shader load_glsl(shader_type type,const io::file& src);

	/// Compiles GLSL shader from string source
	/// \throw runtime_error in case of compilation failure
	shader(shader_type type, const char* source);
	shader(shader&& other) noexcept;
	shader& operator=(shader&& rhs) noexcept;
	~shader() noexcept;
	/// Gets OpenGL shader identifier
	/// \return shader identifier
	inline ::GLuint handle() const noexcept {
		return hsdr_;
	}
	/// Gets this shader pipeline state type
	/// \return pipeline state type
	inline shader_type type() const noexcept {
		return type_;
	}
	inline void swap(shader& other) noexcept;
private:
	::GLuint hsdr_;
	shader_type type_;
};

struct shader_program_attribute
{
	const char* name;
	uint8_t stride;
};

class program;
DECLARE_IPTR(program);

/// OpenGL program object
class program final:public io::object {
private:
	program(shader&& vertex, shader&& fragment);
	void validate(::GLenum pname,const char* emsg);
public:
	/// Creates a new OpenGL program object
	/// \param vertex - vertex stage shader
	/// \param fragment - fragment stage shader
	/// \throw std::invalid_argument when vertex or fragment are not correct shader type
	static s_program create(shader&& vertex, shader&& fragment);
	/// Attaches additional shading pipeline shader ( geometry, tesselation, compute etc )
	/// \param sh additional shader
	/// \throw logic_error when attempting to attach additional vertex or fragment shader
	void attach_shader(shader&& sh);
	/// Links OpengGL program object
	/// \throw runtime_error in case of linking or validation error
	void link();
	/// Starts OpenGL rendering pipeline
	inline void start() noexcept {
		::glUseProgram(hprg_);
	}
	/// Stopes OpenGL rendering pipeline
	inline void stop() noexcept {
		::glUseProgram(0);
	}
	/// Passing vertex attribute to program
	/// \param attr_no vertex attribute number
	/// \param vbo vertex buffer object to bind
	/// \param normalized whether attribute array normalized
	/// \param stride vertex attribute stride
	/// \param size vertex attribute size in bytes
	/// \param offset vertex attribute offset inside VBO
	void pass_vertex_attrib_array(::GLsizei attr_no, const s_buffer& vbo, bool normalized,uint8_t stride, uint8_t size, uint8_t offset);
	/// Passing vertex attributes in a VBO to program
	/// \param vbo vertex buffer object to bind
	/// \param normalized whether attribute array normalized
	/// \param layout array of vertex attributes layout in format {name,stride}, layout index follows the attribute number
	/// \param lsize size of layout array in elements
	void pass_vertex_attrib_array(const s_buffer& vbo, bool normalized,const shader_program_attribute* layout,std::size_t lsize);
	/// Binds attribute location to the shader attribute name
	void bind_attrib_location(::GLsizei attr_no, const char* name);
	/// Gets GLSL program uniform location (identifier) by its name
	/// \param uniform a uniform name
	/// \return uniform location
	::GLuint uniform_location(const char* uniform);
	/// Gets GLSL program vertex attribute number (identifier) by its name
	/// \param attribute_name a vertex attribute name
	/// \return attribute number
	::GLuint attribute_number(const char* attribute_name);
	/// Destroys underlying OpenGL program object
	virtual ~program() noexcept override;
private:
	::GLuint hprg_;
	std::vector<shader> shaders_;
};

} // namespace gl

#endif // __SHADER_HPP_INCLUDED__
