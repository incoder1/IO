#ifndef __VBO_HPP_INCLUDED__
#define __VBO_HPP_INCLUDED__

#include <object.hpp>
#include "openglload.hpp"

namespace gl {

enum class data_type: ::GLenum {
	BYTE = GL_BYTE,
	UNSIGNED_BYTE= GL_UNSIGNED_BYTE,
	SHORT = GL_SHORT,
	UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
	INT = GL_INT,
	UNSIGNED_INT = GL_UNSIGNED_INT,
	FLOAT = GL_FLOAT,
	DOUBLE = GL_DOUBLE
};

enum class buffer_type: ::GLenum {
	/// Vertex attributes. Requires: OpenGL 1.5+
	ARRAY_BUFFER = GL_ARRAY_BUFFER,
	/// Atomic counter storage. Requires: OpenGL 4.2+
	ATOMIC_COUNTER = GL_ATOMIC_COUNTER_BUFFER,
	/// Buffer copy source. Requires: OpenGL 3.1+
	COPY_READ_BUFFER = GL_COPY_READ_BUFFER,
	/// Buffer copy destination. Requires: OpenGL 3.1+
	COPY_WRITE_BUFFER = GL_COPY_WRITE_BUFFER,
	/// Indirect compute dispatch commands. Requires: OpenGL 4.0+
	DISPATCH_INDIRECT_BUFFER = GL_DISPATCH_INDIRECT_BUFFER,
	/// Indirect command arguments. Requires: OpenGL 4.0+
	DRAW_INDIRECT_BUFFER = GL_DRAW_INDIRECT_BUFFER,
	/// Vertex array indexes. Requires: OpenGL 1.5+
	ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
	/// Query result buffer. Requires: OpenGL 3.1+
	QUERY_BUFFER = GL_QUERY_BUFFER,
	/// Pixel read target. Requires: OpenGL 2.1+
	PIXEL_PACK_BUFFER = GL_PIXEL_PACK_BUFFER,
	/// Texture data source. Requires: OpenGL 2.1+
	PIXEL_UNPACK_BUFFER = GL_PIXEL_UNPACK_BUFFER,
	/// Read-write storage for shaders. Requires: OpenGL 4.3+
	SHADER_STORAGE_BUFFER = GL_SHADER_STORAGE_BUFFER,
	/// Texture data buffer.Requires: OpenGL 3.1+
	TEXTURE_BUFFER = GL_TEXTURE_BUFFER,
	/// Transform feedback buffer. Requires: OpenGL 4.0+
	TRANSFORM_FEEDBACK_BUFFER = 35982,
	/// Uniform block storage. Requires: OpenGL 3.1+
	UNIFORM_BUFFER = GL_UNIFORM_BUFFER
};

enum class stride: ::GLenum {
	ONE   = 1,
	TWO   = 2,
	THREE = 3,
	FOUR  = 4
};

enum class buffer_usage: ::GLenum {

	DYNAMIC_COPY = GL_DYNAMIC_COPY,
	DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
	DYNAMIC_READ = GL_DYNAMIC_READ,

	STATIC_COPY = GL_STATIC_COPY,
	STATIC_DRAW = GL_STATIC_DRAW,
	STATIC_READ = GL_DYNAMIC_READ,

	STREAM_COPY = GL_STREAM_COPY,
	STREAM_DRAW = GL_STREAM_DRAW,
	STREAM_READ = GL_STREAM_READ
};

class buffer;
DECLARE_IPTR(buffer);

class buffer: public io::object
{
	buffer(const buffer&) = delete;
	buffer& operator=(const buffer&) = delete;
private:
	buffer(::GLuint id,buffer_type bt,data_type dt,std::size_t size, stride st, buffer_usage u) noexcept;
public:

	static s_buffer create(const void* data, std::size_t size, buffer_type bt, data_type dt, stride st, buffer_usage u);


	virtual ~buffer() noexcept override;

	bool bind() noexcept;
	void unbind() noexcept;

	void swap(buffer& other) noexcept;

	inline buffer_type type() const noexcept
	{
		return type_;
	}

	inline data_type element_type() const noexcept
	{
		return data_type_;
	}

	inline std::size_t size() const noexcept
	{
		return size_;
	}

	inline stride element_stride() const noexcept
	{
        return stride_;
	}

	inline buffer_usage usage() const noexcept
	{
		return usage_;
	}

private:
	::GLuint id_;
	buffer_type type_;
	data_type data_type_;
	std::size_t size_;
	stride stride_;
	buffer_usage usage_;
	std::atomic_flag bound_;
};



} // namespace gl

#endif // __VBO_HPP_INCLUDED__
