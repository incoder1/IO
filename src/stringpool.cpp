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
#include "stringpool.hpp"

namespace io {


// string_pool
s_string_pool string_pool::create(std::error_code& ec) noexcept
{
	string_pool* result = new (std::nothrow) string_pool();
	return nullptr != result ? s_string_pool(result) : s_string_pool();
}

string_pool::string_pool() noexcept:
	object(),
	pool_()
{
}

string_pool::~string_pool() noexcept
{
}

const cached_string string_pool::get(const char* s, std::size_t count) noexcept
{
	typedef pool_type::value_type pair_type;

	if( io_unlikely( (nullptr == s || '\0' == *s || count == 0 ) ) )
		return cached_string();
	if( count > detail::SSO_MAX ) {
		const std::size_t str_hash = io::hash_bytes(s,count);
		pool_type::iterator it = pool_.find( str_hash );
		if( it != pool_.end() ) {
			// handle hash-miss collision
			// more likely never happens, since City Hash
			if( io_unlikely( pool_.count( str_hash ) > 1 ) ) {
				auto its = pool_.equal_range( str_hash );
				it = std::find_if(its.first, its.second, [s,count] (const pair_type& entry) {
					return entry.second.equal(s, count);
				} );
			}
			return it->second;
		}
#ifndef IO_NO_EXCEPTIONS
		try {
#endif // IO_NO_EXCEPTIONS
			std::pair<pool_type::iterator,bool> ret = pool_.emplace( str_hash, cached_string(s, count) );
			if( io_likely( ret.second ) )
				return ret.first->second;
#ifndef IO_NO_EXCEPTIONS
		}
		catch(std::exception&) {
			// skip out of memory, and return string as it is
			// i.e. empty string
		}
#endif // IO_NO_EXCEPTIONS
	}
	// no problem on SSO string, it should not be pulled since
	// all data stored inside string object it self
	return cached_string(s, count);
}


} // namespace io
