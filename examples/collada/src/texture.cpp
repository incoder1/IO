#include "stdafx.hpp"
#include "texture.hpp"

#include <cmath>

namespace gl {

// texture

static void tex2d_generate_mipmaps(const ::GLsizei width,const ::GLsizei height,::GLenum ifmt,::GLenum pxmft, const ::GLvoid* pixels,const ::GLint min_filter,const ::GLint mag_filter)
{
	static const double ln_2 = std::log(2.0);
	const double max_level = ( std::log( static_cast<double>(width) ) / ln_2 ) - 1.0;
	::GLuint num_mipmaps = static_cast<::GLuint>( std::lround(std::abs(max_level) ) );
	::glTexStorage2D(GL_TEXTURE_2D, num_mipmaps, ifmt, width, height);
	::glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, pxmft, GL_UNSIGNED_BYTE, pixels);
	::glGenerateMipmap(GL_TEXTURE_2D);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
}


s_texture texture::texture2d(::GLsizei width, ::GLsizei height, ::GLint i_format, ::GLenum px_format, texture_filter filtering, const void* pixels)
{
	::GLuint id;
	::glGenTextures(1,&id);

	::glBindTexture( GL_TEXTURE_2D, id );

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	::glTexImage2D(GL_TEXTURE_2D, 0, i_format, width, height, 0, px_format, GL_UNSIGNED_BYTE, pixels);

	switch (filtering) {
	case texture_filter::NEAREST_MIPMAP_NEAREST:
		tex2d_generate_mipmaps(width, height, i_format, px_format, pixels, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
		break;
	case texture_filter::NEAREST_MIPMAP_LINEAR:
		tex2d_generate_mipmaps(width, height, i_format, px_format, pixels, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR);
		break;
	case texture_filter::LINEAR_MIPMAP_NEAREST:
		tex2d_generate_mipmaps(width, height, i_format,  px_format, pixels, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
		break;
	case texture_filter::LINEAR_MIPMAP_LINEAR:
		tex2d_generate_mipmaps(width, height, i_format, px_format, pixels, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		break;
	default:
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<::GLenum>(filtering) );
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<::GLenum>(filtering) );
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	::glBindTexture(GL_TEXTURE_2D, 0 );
	validate_opengl("Invalid texture");

	return s_texture( new texture(id, texture_type::TEXTURE_2D) );
}

s_texture texture::create_texture2d_from_image(const engine::s_image& img, gl::texture_filter filter)
{

	::GLint internal_format;
	::GLenum px_format;
	switch(img->pix_format()) {
		case engine::pixel_format::rgb:
			internal_format = GL_SRGB8;
			px_format = GL_RGB;
			break;
		case engine::pixel_format::rgba:
			internal_format = GL_SRGB8_ALPHA8;
			px_format = GL_RGBA;
			break;
		case engine::pixel_format::bgra:
			internal_format = GL_RGBA8;
			px_format = GL_BGRA;
			break;
	}
	return texture2d(img->width(), img->height(), internal_format, px_format, filter, img->data() );
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


} // namespace gl
