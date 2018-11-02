#include "stdafx.hpp"
#include "texture.hpp"

#include <cmath>

namespace gl {

// texture

static void tex2d_generate_mipmaps(::GLsizei width, ::GLint minFiler, ::GLint magFileter)
{
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFiler);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFileter);
	static const double ln_2 = std::log(2.0);
	double maxLevel = ( std::log( static_cast<double>(width) ) / ln_2 ) - 1.0;
	::GLuint mipmapsMax = static_cast<::GLuint>(  std::lround(std::abs(maxLevel) ) );
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmapsMax);
	::glGenerateMipmap(GL_TEXTURE_2D);
}


s_texture texture::texture2d(::GLsizei width, ::GLsizei height, ::GLint format, texture_filter filtering, const void* pixels)
{
	::GLuint id;
	::glGenTextures(1,&id);

	::glBindTexture( GL_TEXTURE_2D, id );

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	::glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	switch (filtering) {
	case texture_filter::NEAREST_MIPMAP_NEAREST:
		tex2d_generate_mipmaps(width, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
		break;
	case texture_filter::NEAREST_MIPMAP_LINEAR:
		tex2d_generate_mipmaps(width, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR);
		break;
	case texture_filter::LINEAR_MIPMAP_NEAREST:
		tex2d_generate_mipmaps(width, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
		break;
	case texture_filter::LINEAR_MIPMAP_LINEAR:
		tex2d_generate_mipmaps(width, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		break;
	default:
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<::GLenum>(filtering) );
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<::GLenum>(filtering) );
	}

	::glBindTexture(GL_TEXTURE_2D, 0 );

	return s_texture( new texture(id, texture_type::TEXTURE_2D) );
}

texture::texture(::GLuint id,texture_type type) noexcept:
	io::object(),
	id_(id),
	type_(type),
	bound_(ATOMIC_FLAG_INIT)
{}

bool texture::bind() noexcept
{
	if( !bound_.test_and_set() ) {
		::glBindTexture( static_cast<GLenum>(type_), id_ );
		return true;
	}
	return false;
}

void texture::unbind() noexcept
{
	::glBindTexture(static_cast<GLenum>(type_), 0 );
	bound_.clear();
}

texture::~texture() noexcept
{
	::glDeleteTextures(1, &id_);
}

// USE GDI+ as PNG decoder
#ifdef _WIN32

#include <Gdiplus.h>

#endif // _WIN32

s_texture load_2d_texture_png(const io::s_read_channel& src, texture_filter filter)
{
	return s_texture();
}

} // namespace gl
