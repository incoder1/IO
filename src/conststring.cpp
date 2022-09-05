/*
 *
 * Copyright (c) 2016-2022
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "conststring.hpp"

namespace io {

void const_string::init_short(detail::sso_variant_t& dst, const char* str, std::size_t length) noexcept
{
	dst.short_buf.sso = true;
	dst.short_buf.size = length;
	io_zerro_mem(dst.short_buf.char_buf, detail::SSO_MAX);
	io_memmove(dst.short_buf.char_buf, str, length);
}

void const_string::init_long(detail::sso_variant_t& dst,std::size_t size,const char* str, std::size_t length) noexcept
{
	// allocate string + reference counter, by calloc
	detail::utf8char* px = memory_traits::malloc_array<detail::utf8char>( size + sizeof(std::size_t) );
	if( nullptr != px ) {
		// set initial intrusive reference count
		std::size_t *rc = reinterpret_cast<std::size_t*>(px);
		// can be regular non atomic store operation, since new block of memory
		// not shared between any threads yet
		*rc = 1;
		// copy string data
		io_memmove( px + sizeof(std::size_t), str, length);
		dst.long_buf.size = length;
		dst.long_buf.char_buf = px;
	}
}

void const_string::long_buf_release(detail::sso_variant_t& var) noexcept
{
	// decrement atomic intrusive reference counter
	// release long buffer if needed
	if( 0 == detail::atomic_traits::dec(reinterpret_cast<std::size_t volatile*>(var.long_buf.char_buf)) ) {
		// infer read memory barrier
		std::atomic_thread_fence( std::memory_order_acquire  );
		// release string memory
		memory_traits::free(var.long_buf.char_buf);
	}
}

const_string::const_string(const char* str, std::size_t length) noexcept:
	data_(
{
	false,0,nullptr
} )
{
	assert(nullptr != str &&  length > 0  && length < SIZE_MAX );
	std::size_t new_size = length;
	if( '\0' != str[length-1] )
		++new_size;
	if(new_size < detail::SSO_MAX)
		init_short(data_, str, length);
	else
		init_long(data_, new_size, str, length);
}

const_string::~const_string() noexcept
{
	if( !empty() && !sso() )
		long_buf_release(data_);
}

bool const_string::blank() const noexcept
{
	static const char* WS = "\t\n\v\f\r ";
	return empty() ? true : size() == io_strspn(data(), WS);
}

bool const_string::ptr_equal(const const_string& rhs) const noexcept
{
	return (this == std::addressof(rhs)) ||
			( !sso() &&
			 (data_.long_buf.char_buf == rhs.data_.long_buf.char_buf)
			);
}

int const_string::compare(const const_string& rhs) const noexcept
{
	int ret = 1;
	if( ( empty() && rhs.empty() ) || ptr_equal(rhs) ) {
		ret = 0;
	}
	else {
		const std::size_t byte_size = size();
		ret = byte_size < rhs.size() ? -1 : traits_type::compare( data(), rhs.data(), byte_size );
	}
	return ret;
}

bool const_string::equal(const char* rhs, const std::size_t len) const noexcept
{
	bool ret = false;
	if(carr_empty(rhs, len) && empty() )
		ret = true;
	else
		if( !empty() && !carr_empty(rhs, len) && len <= size() )
			ret = ( 0 == traits_type::compare( data(), rhs, len ) );
	return ret;
}

} // namespace io
