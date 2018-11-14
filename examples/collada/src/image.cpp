#include "stdafx.hpp"
#include "image.hpp"

namespace engine {

// image
image::image(std::size_t w, std::size_t h,pixel_format pfm,io::byte_buffer&& data) noexcept:
	io::object(),
	width_(w),
	height_(h),
	pfm_(pfm),
	data_( std::forward<io::byte_buffer>(data) )
{}


} // namespace engine
