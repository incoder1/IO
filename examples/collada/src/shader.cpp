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
#include "stdafx.hpp"
#include "shader.hpp"

namespace gl {

#ifdef  __IO_WINDOWS_BACKEND__
static std::string get_process_start_dir() {
	char  exe_name[ MAX_PATH+1 ] = {'\0'};
	::GetModuleFileNameA(nullptr,exe_name,MAX_PATH);
	std::string exe(exe_name);
	return exe.substr(0, exe.find_last_of('\\'));
}
#else
static std::string get_process_start_dir() {
	char  exe_name[ PATH_MAX+1 ] = {'\0'};
	char query[64] = {'\0'};
	std::snprintf(query, 64, "/proc/%u/exe", ::getpid() );
	::readlink(query, exe_name, PATH_MAX);
	std::string exe(exe_name);
	return exe.substr(0, exe.find_last_of('\\'));
}
#endif // __IO_WINDOWS_BACKEND__

io::file shader_file(const char* name) {
	std::string full_name = get_process_start_dir();
	full_name.push_back( io::file::separator() );
	full_name.append( name );
	return io::file(full_name);
}

// shader
shader shader::load_glsl(shader_type type, const io::s_read_channel& src)
{
	std::error_code ec;
	io::byte_buffer buff = io::byte_buffer::allocate(ec, io::memory_traits::page_size() / 4 );
	io::check_error_code(ec);
	std::size_t read = 0;
	do {
		uint8_t* pos = const_cast<uint8_t*>( buff.position().get() );
		read = src->read(ec, pos, buff.available() );
		if(0 != read) {
			buff.move(read);
			if( !buff.ln_grow() )
				ec = std::make_error_code( std::errc::not_enough_memory );
		}
	} while(0 != read && !ec);
	io::check_error_code(ec);
	buff.flip();
	return shader(type, buff.position().cdata());
}

shader shader::load_glsl(shader_type type,const io::file& file)
{
	std::error_code ec;
	io::s_read_channel src = file.open_for_read(ec);
	if(ec)
		throw std::system_error(ec);
	return load_glsl(type, src);
}


shader::shader(shader_type type, const char* source):
	hsdr_(0),
	type_(type)
{
	hsdr_ = ::glCreateShader( static_cast<::GLenum>(type_) );
	if(0 == hsdr_)
		throw std::runtime_error("Can not create shader");
	::glShaderSource(hsdr_, 1, std::addressof( source ), nullptr);
	::glCompileShader( hsdr_ );
	::GLint val;
	::glGetShaderiv(hsdr_, GL_COMPILE_STATUS, &val);
	if(GL_FALSE == val ) {
		::glGetShaderiv(hsdr_, GL_INFO_LOG_LENGTH, &val);
		::GLchar *log = static_cast<GLchar*>( io_alloca(val) );
		::glGetShaderInfoLog(hsdr_, val, &val, log);
		std::string msg("Error compiling ");
		switch(type_) {
		case shader_type::compute:
			msg.append("compute");
			break;
		case shader_type::vertex:
			msg.append("vertex");
			break;
		case shader_type::tess_control:
			msg.append("tesselation control");
			break;
		case shader_type::tess_evaluation:
			msg.append("tesselation evaluation");
			break;
		case shader_type::geometry:
			msg.append("geometry");
			break;
		case shader_type::fragment:
			msg.append("fragment");
			break;
		}
		msg.append(" shader:\n\t");
		msg.append(log);
		throw std::runtime_error( msg );
	}
}

shader::shader(shader&& other) noexcept:
	hsdr_(other.hsdr_),
	type_(other.type_)
{
	other.hsdr_ = 0;
}

shader& shader::operator=(shader&& rhs) noexcept
{
	shader( std::forward<shader>(rhs) ).swap( *this );
	return *this;
}

inline void shader::swap(shader& other) noexcept
{
	std::swap( hsdr_, other.hsdr_ );
	std::swap( type_, other.type_ );
}

shader::~shader() noexcept
{
	if( ::glIsShader(hsdr_) )
		::glDeleteShader(hsdr_);
}

// program
s_program program::create(shader&& vertex, shader&& fragment)
{
	if(shader_type::vertex != vertex.type() )
		throw std::invalid_argument("vertex is not GL_VERTEX_SHADER");
	if(shader_type::fragment != fragment.type())
		throw std::invalid_argument("fragment is not GL_FRAGMENT_SHADER");
	return s_program( new program( std::forward<shader>(vertex), std::forward<shader>(fragment) ) );
}

program::program(shader&& vertex, shader&& fragment):
	io::object(),
	shaders_()
{
	shaders_.emplace_back( std::move(vertex) );
	shaders_.emplace_back( std::move(fragment) );
	hprg_ = ::glCreateProgram();
}

program::~program() noexcept
{
	::glDeleteProgram(hprg_);
}

void program::attach_shader(shader&& sh)
{
	if( shader_type::vertex == sh.type() )
		throw std::invalid_argument("Can not attach additional GL_VERTEX_SHADER");
	if(shader_type::fragment == sh.type())
		throw std::invalid_argument("Can not attach additional GL_FRAGMENT_SHADER");
	shaders_.emplace_back( std::move(sh) );
}

void program::validate(::GLenum pname,const char* emsg)
{
	::GLint val = GL_FALSE;
	::glGetProgramiv(hprg_, pname, &val);
	if(GL_TRUE != val) {
		::glGetProgramiv(hprg_, GL_INFO_LOG_LENGTH, &val);
		::GLchar *log = static_cast<GLchar*>( io_alloca(val) );
		::glGetProgramInfoLog( hprg_, val, &val, log );
		std::string msg(emsg);
		msg.push_back('\t');
		msg.append(log);
		throw std::runtime_error( msg );
	}
}

void program::link()
{
	// attach all shader objects before linking program
	std::for_each(shaders_.begin(), shaders_.end(), [this] (const shader& sh) {
		::glAttachShader(hprg_, sh.handle() );
	} );
	::glLinkProgram(hprg_);
	// detach all shader objects after linking program
	// since no longer needed
	std::for_each(shaders_.begin(), shaders_.end(), [this] (const shader& sh) {
		::glDetachShader(hprg_, sh.handle() );
	} );
	// now we can clean shaders since they are no longer needed
	shaders_.clear();
	validate(GL_LINK_STATUS, "Shader program link error:\n");
	::glValidateProgram(hprg_);
	validate(GL_VALIDATE_STATUS, "Shader program validate error:\n");
}


void program::pass_vertex_attrib_array(::GLsizei attr_no, const s_buffer& vbo, bool normalized,uint8_t stride, uint8_t size, uint8_t offset)
{
	if (buffer_type::ARRAY_BUFFER != vbo->type() )
		throw std::runtime_error("Array buffer expected");
	const std::size_t dtp_size = sizeof_data_type(vbo->element_type());

	vbo->bind();
	const ::GLsizei vertex_size = stride *  dtp_size;
	::GLvoid* voffset = reinterpret_cast<::GLvoid*>(offset * dtp_size);
	::glVertexAttribPointer(
		attr_no,
		size,
		static_cast<::GLenum>(vbo->element_type()),
		normalized,
		vertex_size,
		voffset
	);
	::glEnableVertexAttribArray(attr_no);
	vbo->unbind();

	validate_opengl("Can not pass vertex attributes array");
}


void program::pass_vertex_attrib_array(const s_buffer& vbo, bool normalized,const shader_program_attribute* layout,std::size_t lsize)
{

	uint8_t stride = 0;
	for(uint8_t i = 0; i < lsize; i++) {
        bind_attrib_location( i, layout[i].name );
		stride += layout[i].stride;
	}
	uint8_t offset = 0;
	for(uint8_t i = 0; i < lsize; i++) {
		pass_vertex_attrib_array(i, vbo, normalized, stride, layout[i].stride, offset);
		offset += layout[i].stride;
	}
}

void program::bind_attrib_location(::GLsizei attr_no, const char* name)
{
	::glBindAttribLocation(hprg_, attr_no, name);
}

::GLuint program::uniform_location(const char* uniform)
{
	::GLuint ret = ::glGetUniformLocation(hprg_, uniform);
	if (GL_NO_ERROR != ::glGetError() )
		throw std::runtime_error( std::string("Uniform: ").append(uniform).append(" not found") );
	return ret;
}

::GLuint program::attribute_number(const char* attribute_name)
{
	::GLuint ret =  ::glGetAttribLocation(hprg_, attribute_name);
	if (GL_NO_ERROR != ::glGetError() )
		throw std::runtime_error( std::string("Attribute: ").append(attribute_name).append(" not found") );
	return ret;
}

} // namespace gl
