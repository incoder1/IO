/*
 *
 * Copyright (c) 2016-2023
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "io/textapi/detail/prober.hpp"

namespace io {

namespace detail {

// prober
const char* prober::ENGLISH_LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

prober::prober() noexcept:
	io::object()
{}

//This filter applies to all scripts which do not use English characters
io::byte_buffer prober::filter_without_english_letters(std::error_code& ec,const uint8_t* buff, std::size_t size) noexcept
{
	byte_buffer ret = byte_buffer::allocate(ec, size);
	if(!ec) {
		const char* b = reinterpret_cast<const char*>(buff);
		std::size_t offset = 0;
		do {
			b += offset;
			offset = io_strcspn(b, ENGLISH_LETTERS);
			ret.put(b,offset);
			b += offset;
			offset = io_strspn(b, ENGLISH_LETTERS);
		}
		while(offset > 0);
		ret.flip();
	}
	return ret;
}

//This filter applies to all scripts which contain both English characters and upper ASCII characters
io::byte_buffer prober::filter_with_english_letters(std::error_code& ec,const uint8_t* buff, std::size_t size) noexcept
{
	byte_buffer ret = byte_buffer::allocate(ec, size);
	if(!ec) {
		const char* b = reinterpret_cast<const char*>(buff);
		const char* e = b + size;
		while(b < e) {
			if( is_alpha(*b) ) {
				std::size_t chars = io_strspn(b, ENGLISH_LETTERS);
				ret.put(b,chars);
				b += chars;
			}
			else if( utf8::is_multibyte(*b) ) {
				std::size_t bytes = utf8::mblen(b);
				ret.put(b, bytes);
				b += bytes;
			}
			else {
				++b;
			}
		}
		ret.flip();
	}
	return ret;
}

} // namespace detail

} // namespace io
