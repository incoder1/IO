#ifndef __GL_TEXTURE_HPP_INCLUDED__
#define __GL_TEXTURE_HPP_INCLUDED__

#include <atomic>

#include <object.hpp>

#include "openglload.hpp"
#include "image.hpp"

namespace gl {

enum class texture_filter: ::GLenum {
	NEAREST = GL_NEAREST,
	LINEAR = GL_LINEAR,
	NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
	LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST
};

enum class texture_type: ::GLenum {
	TEXTURE_1D = GL_TEXTURE_1D,
	TEXTURE_2D = GL_TEXTURE_2D,
	TEXTURE_3D = GL_TEXTURE_3D,
	TEXTURE_1D_ARRAY = GL_TEXTURE_1D_ARRAY,
	TEXTURE_2D_ARRAY = GL_TEXTURE_2D_ARRAY,
	TEXTURE_RECTANGLE = GL_TEXTURE_RECTANGLE,
	TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP,
	TEXTURE_CUBE_MAP_ARRAY = GL_TEXTURE_CUBE_MAP_ARRAY,
	TEXTURE_BUFFER = GL_TEXTURE_BUFFER,
	TEXTURE_2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE,
	TEXTURE_2D_MULTISAMPLE_ARRAY = GL_TEXTURE_2D_MULTISAMPLE_ARRAY
};

class texture;
DECLARE_IPTR(texture);

class texture final:public io::object {
private:
	texture(::GLuint id,texture_type type) noexcept;
	static s_texture texture2d(::GLsizei width, ::GLsizei height, ::GLint i_format, ::GLenum format, texture_filter filtering, const void* pixels);
public:

	static inline s_texture create_rgb_texture_2d(std::size_t width, std::size_t height, texture_filter filtering,const void* pixels) {
		return texture2d(width, height, GL_RGB, GL_RGB, filtering, pixels);
	}

	static inline s_texture create_rgba_texture_2d(std::size_t width, std::size_t height, texture_filter filtering, const void* pixels) {
		return texture2d(width, height, GL_SRGB8_ALPHA8, GL_RGBA, filtering, pixels);
	}

	static s_texture create_texture2d_from_image(const engine::s_image& image, gl::texture_filter filter);


	bool bind()  noexcept;
	void unbind()  noexcept;
	inline texture_type type() noexcept {
		return type_;
	}
	virtual ~texture() noexcept override;
private:
	::GLuint id_;
	texture_type type_;
	std::atomic_flag bound_;
};




} // namespace gl

#endif // __GL_TEXTURE_HPP_INCLUDED__
