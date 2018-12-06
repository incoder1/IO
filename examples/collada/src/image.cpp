#include "stdafx.hpp"
#include "image.hpp"

namespace engine {

// image
image::image(std::size_t w, std::size_t h,pixel_format pfm,io::scoped_arr<uint8_t>&& data) noexcept:
	io::object(),
	width_(w),
	height_(h),
	pfm_(pfm),
	data_( std::forward<io::scoped_arr<uint8_t> >( data ) )
{}


} // namespace engine
