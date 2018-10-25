#include "shader.hpp"

namespace gl {

// shader
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

// vao
vao vao::create(::GLsizei size)
{
	::GLuint *arr = new ::GLuint [size];
	::glGenVertexArrays(size, arr );
	return vao( arr, size );
}

vao::~vao() noexcept
{
	if(nullptr != arr_) {
		for(::GLsizei i =0; i < size_; i++)
			::glDisableVertexAttribArray(arr_[i]);
		::glDeleteVertexArrays(size_, arr_);
		delete [] arr_;
	}
}


// program
s_program program::create(shader&& vertex, shader&& fragment)
{
	if(shader_type::vertex != vertex.type() )
		throw std::runtime_error("vertex is not GL_VERTEX_SHADER");
	if(shader_type::fragment != fragment.type())
		throw std::runtime_error("fragment is not GL_FRAGMENT_SHADER");
	return s_program( new program( std::forward<shader>(vertex), std::forward<shader>(fragment) ) );
}

program::program(shader&& vertex, shader&& fragment):
	io::object(),
	shaders_(),
	vao_()
{
	shaders_.emplace_back( std::move(vertex) );
	shaders_.emplace_back( std::move(fragment) );
	hprg_ = ::glCreateProgram();
}

program::~program() noexcept
{
	std::for_each(shaders_.begin(), shaders_.end(), [this] (const shader& sh) {
          ::glDetachShader(hprg_, sh.handle() );
	} );
	::glDeleteProgram(hprg_);
}

void program::attach_shader(shader&& sh)
{
    shaders_.emplace_back( std::move(sh) );
}

void program::link()
{
	std::for_each(shaders_.begin(), shaders_.end(), [this] (const shader& sh) {
          ::glAttachShader(hprg_, sh.handle() );
	} );
	// TODO: make error handling generic
	::glLinkProgram(hprg_);
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

void program::start()
{
	::glUseProgram(hprg_);
}

void program::stop()
{
	::glUseProgram(0);
}

void program::gen_vertex_attrib_arrays(std::size_t count)
{
	vao_ = vao::create( count );
 	if (GL_NO_ERROR != ::glGetError() )
		throw std::runtime_error("Can not initialize GLSL program vertex attributes");
}

void program::pass_vertex_attrib_array(::GLsizei attr_no, const s_buffer& vbo, bool normalized)
{
	if (buffer_type::ARRAY_BUFFER != vbo->type() )
		throw std::runtime_error("Array buffer expected");
	::glEnableVertexAttribArray(attr_no);
	vbo->bind();
	::glVertexAttribPointer(
			attr_no,
			vbo->size(),
			static_cast<::GLenum>( vbo->element_type() ),
			normalized,
			static_cast<::GLenum>(vbo->element_stride()),
			nullptr);
	vbo->unbind();
	if (GL_NO_ERROR != ::glGetError())
		throw std::runtime_error("Can not pass vertex attributes array");
}

void program::bind_attrib_location(::GLsizei attr_no, const char* name)
{
	::glBindAttribLocation(hprg_, attr_no, name);
}

::GLuint program::uniform_location(const char* uniform)
{
	return ::glGetUniformLocation(hprg_, uniform);
}

::GLuint program::attribute_number(const char* attribute_name)
{
	return ::glGetAttribLocation(hprg_, attribute_name);
}

} // namespace gl
