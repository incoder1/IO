#include "stdafx.hpp"
#include "image.hpp"

namespace engine {

// image
image::image(std::size_t w, std::size_t h, unsigned short stride,io::byte_buffer&& data) noexcept:
	io::object(),
	width_(w),
	height_(h),
	stride_(stride),
	data_( std::forward<io::byte_buffer>(data) )
{}


} // namespace engine
