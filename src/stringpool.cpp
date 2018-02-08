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


// string_pool

string_pool::string_pool() noexcept:
	object(),
	pool_()
{}

const cached_string string_pool::get(const char* s, std::size_t count) noexcept
{
#ifdef __GNUG__
	if( __builtin_expect( (nullptr == s || '\0' == *s || count == 0 ), false) )
		return cached_string();
#else
	if(nullptr == s || '\0' == *s || count == 0) {
#	ifdef _MSC_VER
		__assume(0);
#	endif // _MSC_VER
		return cached_string();
	}
#endif // __GNUG__

	const std::size_t str_hash = io::hash_bytes(s,count);
	pool_type::const_iterator i = pool_.find( str_hash );
	if( i == pool_.end() ) {
        auto ret = pool_.emplace( str_hash, cached_string(s, count) );
#ifdef __GNUG__
		return __builtin_expect(ret.second,true)
			? ret.first->second : cached_string(s, count);
#else
		return ret.second ? ret.first->second : cached_string(s, count);
#endif // defined
	} else {
#ifdef __GNUG__
        return __builtin_expect( i->second.equal(s, count), true)
        	? i->second : cached_string(s, count);
#else
		return i->second.equal(s, count)
			? i->second : cached_string(s, count);
#endif // __GNUG__
	}
	return cached_string();
}

} // namespace io
