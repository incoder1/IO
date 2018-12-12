#include "stdafx.hpp"
#include "image.hpp"

#include <png.h>
#include <memory>

namespace engine {

constexpr std::size_t align_up(const std::size_t alignment,const std::size_t size) noexcept
{
	return ( size + (alignment - 1) ) & ~(alignment - 1);
}

class png_reader {
	png_reader(const png_reader&) = delete;
	png_reader& operator=(const png_reader&) = delete;
private:

	static constexpr int TRANSFORMS_RGBA = PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND;
	static constexpr int TRANSFORMS_RGB = TRANSFORMS_RGBA | PNG_TRANSFORM_STRIP_ALPHA;

	static void read_callback(::png_structp png,::png_bytep out,::png_size_t count) {
		const png_reader* self = static_cast<const png_reader*>( ::png_get_io_ptr(png) );
		std::error_code ec;
		std::size_t read = 0, requared = count;
		do {
		 	read = self->src_->read(ec, out, requared);
		 	requared -= read;
		} while( requared > 0 || ec );
		if(ec)
			throw std::system_error( ec );
	}

public:
	png_reader(io::s_read_channel&& src):
		png_(nullptr),
		info_(nullptr),
		src_( std::forward<io::s_read_channel>(src) )
	{

		png_ = ::png_create_read_struct(PNG_LIBPNG_VER_STRING,
										nullptr, nullptr, nullptr
									   );
		if(nullptr == png_)
			throw std::bad_alloc();

		if (setjmp(png_jmpbuf(png_)))
			throw std::runtime_error("Corrupt PNG source");

		::png_set_sig_bytes(png_, 0);

		::png_set_read_fn(png_, const_cast<void*>( static_cast<const void*>(this) ), png_reader::read_callback );

		info_ = ::png_create_info_struct(png_);
		if(nullptr == png_) {
			::png_destroy_read_struct(&png_, nullptr, nullptr);
			throw std::bad_alloc();
		}

	}


	s_image read(pixel_format pixfmt) {
		switch(pixfmt) {
		case pixel_format::rgb:
			::png_read_png(png_, info_, TRANSFORMS_RGB, nullptr);
			break;
		case pixel_format::rgba:
			::png_read_png(png_, info_, TRANSFORMS_RGBA, nullptr);
			break;
		}

		::png_uint_32 width = 0, height = 0;
		int bit_depth, color_type, interlace_type;
		::png_get_IHDR(png_, info_, &width, &height, &bit_depth, &color_type,
					   &interlace_type, nullptr, nullptr);

		unsigned int rowbytes =  align_up(4, ::png_get_rowbytes(png_, info_) );
		io::scoped_arr<uint8_t> data( rowbytes * height * sizeof(png_byte)+15 );

		png_bytepp row_pointers = ::png_get_rows(png_, info_);

		uint8_t* px = data.get();

    	for (::png_uint_32 i = 0; i < height; i++) {
        	std::memcpy( px, row_pointers[i], rowbytes  );
			px += rowbytes;
		}

		return s_image( new image(width, height, pixfmt , std::move(data) ) );
	}


	~png_reader() noexcept {
		::png_destroy_read_struct(&png_, &info_, nullptr);
	}
private:
	::png_structp png_;
	::png_infop info_;
	io::s_read_channel src_;
};


s_image load_rgb(io::s_read_channel&& src, image_format format)
{
	// FIXME: make correct API
	if( image_format::PNG != format )
		throw std::runtime_error( "Unsupported texture format" );

	png_reader reader( std::forward<io::s_read_channel>(src) );
	return reader.read( pixel_format::rgb   );

}

s_image load_rgb(const io::file& file, image_format format)
{
	std::error_code ec;
	io::s_read_channel src = file.open_for_read(ec);
	if(ec)
		throw std::system_error(ec);
	return load_rgb( std::move(src) , format);
}

s_image load_rgba(io::s_read_channel&& src, image_format format)
{
	// FIXME: make correct API
	if( image_format::PNG != format )
		throw std::runtime_error( "Unsupported texture format" );
	png_reader reader( std::forward<io::s_read_channel>(src) );
	return reader.read( pixel_format::rgba   );
}

s_image load_rgba(const io::file& file, image_format format)
{
	std::error_code ec;
	io::s_read_channel src =  file.open_for_read(ec);
	if(ec)
		throw std::system_error(ec);
	return load_rgba( std::move(src) , format);
}

} // namesapace engine
