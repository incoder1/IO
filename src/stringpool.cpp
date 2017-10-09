/*
 *
 * Copyright (c) 2016
 * Viktor Gubin
 *
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "stdafx.hpp"
#include "stringpool.hpp"

namespace io {

namespace detail {

char_holder* char_holder::alloc(const char* s,const std::size_t len) noexcept {
	std::size_t bytes = ('\0'!=*(s+len)) ? len+1 : len;
	uint8_t *raw = memory_traits::malloc_array<uint8_t>( sizeof(char_holder) + bytes );
	if(nullptr == raw)
		return nullptr;
	char* dst = reinterpret_cast<char*>( raw + sizeof(char_holder) );
	io_memmove( dst, s, len);
	return new ( static_cast<void*>(raw) ) char_holder( dst );
}

} // namespace detail

// cached_string
cached_string::cached_string(const char* s, std::size_t count) noexcept:
	holder_(nullptr)
{
	if(nullptr != s && count >= 0)
		holder_ = detail::char_holder::alloc(s,count);
}


// string_pool

string_pool::string_pool():
	object(),
	pool_()
{}

const cached_string string_pool::get(const char* s, std::size_t count) noexcept
{
	if(nullptr == s || '\0' == *s || count == 0)
		return cached_string();
	const std::size_t str_hash = io::hash_bytes(s,count);
	pool_type::const_iterator i = pool_.find( str_hash );
	if( i == pool_.end() ) {
        cached_string new_str(s, count);
        if( new_str.empty() )
            return cached_string(); // an error
        auto ret = pool_.emplace( str_hash, std::move( new_str ) );
        return ret.second ? ret.first->second : new_str;
	} else {
	    // hash miss (collision) check
		return i->second.eq(s, count) ? i->second : cached_string(s, count);
	}
	return cached_string();
}

} // namespace io
