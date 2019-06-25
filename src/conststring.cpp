/*
 *
 * Copyright (c) 2016-2019
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include "stdafx.hpp"
#include "conststring.hpp"

namespace io {


const_string::const_string(const char* str, std::size_t length) noexcept:
	data_( {false,0,nullptr} )
{
	assert(nullptr != str &&  length > 0  && length < SIZE_MAX );
	std::size_t size = length;
	if( '\0' != str[length-1] )
		++size;
	if(size < detail::SSO_MAX) {
		// Optimize short string, i.e. str size is less then sizeof(char*)+sizeof(std::size_t)
		data_.short_buf.sso = true;
		data_.short_buf.size = length;
		traits_type::assign(data_.short_buf.char_buf, '\0', detail::SSO_MAX);
		traits_type::move(data_.short_buf.char_buf, str, length);
	}
	else {
		// allocate string + reference counter
		uint8_t* px = memory_traits::malloc_array<uint8_t>( size + sizeof(std::size_t) );
		if( nullptr != px ) {
			// set initial intrusive reference count
			std::size_t *rc = reinterpret_cast<std::size_t*>(px);
			// can be regular non atomic store operation, since new block of memory
			// not shared between any threads yet
			*rc = 1;
			// copy string data
			char *b = reinterpret_cast<char*>( px + sizeof(std::size_t) );
			traits_type::move(b, str, length);
			data_.long_buf.size = length;
			data_.long_buf.char_buf = px;
		}
	}
}

void const_string::long_buf_release(detail::sso_variant_t& var) noexcept {
	std::size_t volatile *p = reinterpret_cast<std::size_t volatile*>(var.long_buf.char_buf);
	// decrement atomic intrusive reference counter
	// release long buffer if needed
	if( 0 == detail::atomic_traits::dec(p) ) {
		// infer read memory barrier
		std::atomic_thread_fence( std::memory_order_acquire  );
		memory_traits::free(var.long_buf.char_buf);
	}
}



} // namespace io
