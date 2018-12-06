#include "stdafx.hpp"
#include "image.hpp"

#include <png.h>
#include <memory>

static  io::scoped_arr<uint8_t> read_png(const io::s_read_channel& src,uint16_t &w,uint16_t& h)
{
	::png_struct* png_rs =  ::png_create_read_struct(
											PNG_LIBPNG_VER_STRING,
											nullptr, nullptr, nullptr
									 	);
	::png_infop info = png_create_info_struct(png_rs);

	//::png_set_read_fn(png_ptr, static_cast<void*>(nullptr),  );

	::png_destroy_read_struct(&png_rs, &info, nullptr);
	return io::scoped_arr<uint8_t>();
}
