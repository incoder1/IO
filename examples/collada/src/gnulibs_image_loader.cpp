#include "stdafx.hpp"
#include "image.hpp"

#ifdef _OPENMP
#	include <omp.h>
#endif

#include <cstring>
#include <memory>
#include <png.h>

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
		std::size_t left = count;
		std::size_t read;
		do {
			read = self->src_.read(out, left);
		} while( (read > 0) && ( (left -= read) > 0 ) );
	}

public:
	png_reader(io::s_read_channel&& src, bool flip):
		png_(nullptr),
		info_(nullptr),
		src_( std::forward<io::s_read_channel>(src) ),
		flip_(flip)
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
		case pixel_format::bgra:
            break;
		}

		::png_uint_32 width = 0, height = 0;
		int bit_depth, color_type, interlace_type;
		::png_get_IHDR(png_, info_, &width, &height, &bit_depth, &color_type,
					   &interlace_type, nullptr, nullptr);

		unsigned int rowbytes =  align_up(4, ::png_get_rowbytes(png_, info_) );
		io::scoped_arr<uint8_t> data( rowbytes * height  );

		png_bytepp row_pointers = ::png_get_rows(png_, info_);

		uint8_t* px = data.get();
		if(flip_) {
#ifdef _OPENMP
#           pragma omp parallel for
#endif // _OPENMP
            for (::png_uint_32 i = height-1; i > 0 ; i--) {
                std::memcpy( px + (rowbytes*(height-(i+1))) , row_pointers[i], rowbytes  );
            }
		} else {
#ifdef _OPENMP
#           pragma omp parallel for
#endif // _OPENMP
            for (::png_uint_32 i = 0; i < height; i++) {
                std::memcpy( px + (rowbytes*i) , row_pointers[i], rowbytes  );
            }
		}

		return s_image( new image(width, height, image_format::bitmap, pixfmt , std::move(data) ) );
	}


	~png_reader() noexcept {
		::png_destroy_read_struct(&png_, &info_, nullptr);
	}
private:
	::png_structp png_;
	::png_infop info_;
	io::unsafe<io::read_channel> src_;
	bool flip_;
};


s_image load_png_rgb(io::s_read_channel&& src, bool vertical_flip)
{
	png_reader reader( std::forward<io::s_read_channel>(src), vertical_flip);
	return reader.read( pixel_format::rgb   );
}

s_image load_png_rgb(const io::file& file, bool vertical_flip)
{
	std::error_code ec;
	io::s_read_channel src = file.open_for_read(ec);
	if(ec)
		throw std::system_error(ec);
	return load_png_rgb( std::move(src), vertical_flip);
}

s_image load_png_rgba(io::s_read_channel&& src, bool vertical_flip)
{
	png_reader reader( std::forward<io::s_read_channel>(src), vertical_flip);
	return reader.read( pixel_format::rgba   );
}

s_image load_png_rgba(const io::file& file, bool vertical_flip)
{
	std::error_code ec;
	io::s_read_channel src =  file.open_for_read(ec);
	if(ec)
		throw std::system_error(ec);
	return load_png_rgba( std::move(src), vertical_flip);
}

} // namesapace engine
