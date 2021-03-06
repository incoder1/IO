/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
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
