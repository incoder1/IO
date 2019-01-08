#include "stdafx.hpp"
#include "shader.hpp"

namespace gl {

// shader
shader shader::load(shader_type type, const io::s_read_channel& src)
{

	std::error_code ec;
	std::size_t chunk = 8;
	io::byte_buffer buff = io::byte_buffer::allocate(ec, chunk );
	if(ec)
		throw std::system_error(ec);
	io::scoped_arr<uint8_t> tmp( chunk );
	for(std::size_t read = src->read(ec, tmp.get(), chunk); 0 != read && !ec ; ) {
		if( buff.available() < read && !buff.ln_grow() ) {
			ec = std::make_error_code( std::errc::not_enough_memory );
			break;
		}
		buff.put( tmp.get(), read);
		read = src->read(ec, tmp.get(), chunk);
	}
	if(ec)
		throw std::system_error(ec);
	buff.flip();
	return shader(type, buff.position().cdata());
}

shader shader::load(shader_type type,const io::file& file)
{
	std::error_code ec;
	io::s_read_channel src = file.open_for_read(ec);
	if(ec)
		throw std::system_error(ec);
	return load(type, src);
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
	::GLint errc;
	::glGetShaderiv(hsdr_, GL_COMPILE_STATUS, &errc);
	if(GL_FALSE == errc ) {
		std::string msg("Error compiling shader: ");
		char err[512];
		::GLsizei len;
		::glGetShaderInfoLog(hsdr_, 512, &len, err);
		msg.append(err);
		throw std::runtime_error( msg.data() );
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
		throw std::invalid_argument("Can not attach GL_VERTEX_SHADER");
	if(shader_type::fragment == sh.type())
		throw std::invalid_argument("Can not attach GL_FRAGMENT_SHADER");
	shaders_.emplace_back( std::move(sh) );
}

void program::link()
{
	std::for_each(shaders_.begin(), shaders_.end(), [this] (const shader& sh) {
		::glAttachShader(hprg_, sh.handle() );
	} );
	// TODO: make error handling generic
	::glLinkProgram(hprg_);
	std::for_each(shaders_.begin(), shaders_.end(), [this] (const shader& sh) {
		::glDetachShader(hprg_, sh.handle() );
	} );
	::GLint errc = GL_FALSE;
	::glGetProgramiv(hprg_, GL_LINK_STATUS, &errc );
	if( GL_TRUE != errc) {
		std::string msg("Link error: ");
		char log[512];
		::GLsizei len;
		::glGetProgramInfoLog(hprg_, 512, &len, log );
		msg.append(log, len);
		throw std::runtime_error( msg );
	}
	::glValidateProgram(hprg_);
	::glGetProgramiv(hprg_, GL_VALIDATE_STATUS, &errc );
	if( GL_TRUE != errc) {
		std::string msg("Validate error: ");
		char log[512];
		::GLsizei len;
		::glGetProgramInfoLog(hprg_, 512, &len, log );
		msg.append(log, len);
		throw std::runtime_error( msg );
	}
}


void program::pass_vertex_attrib_array(::GLsizei attr_no, const s_buffer& vbo, bool normalized,uint8_t stride, uint8_t size, uint8_t offset)
{
	if (buffer_type::ARRAY_BUFFER != vbo->type() )
		throw std::runtime_error("Array buffer expected");
	const std::size_t dtp_size = sizeof_data_type(vbo->element_type());
	vbo->bind();
	::GLsizei vertex_size = stride *  dtp_size;
	::GLsizei voffset = offset * dtp_size;
	::glVertexAttribPointer(
		attr_no,
		size,
		static_cast<::GLenum>(vbo->element_type()),
		normalized,
		vertex_size,
		reinterpret_cast<::GLvoid*>(voffset)
	);
	::glEnableVertexAttribArray(attr_no);
	vbo->unbind();
	validate_opengl("Can not pass vertex attributes array");
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
