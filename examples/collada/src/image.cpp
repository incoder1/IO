#include "stdafx.hpp"
#include "image.hpp"

namespace engine {

// image
image::image(std::size_t w, std::size_t h,image_format fmt, pixel_format pfm,io::scoped_arr<uint8_t>&& data) noexcept:
	io::object(),
	width_(w),
	height_(h),
	format_(fmt),
	pfm_(pfm),
	data_( std::forward<io::scoped_arr<uint8_t> >( data ) )
{}


} // namespace engine
